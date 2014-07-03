#include "datacanvas.h"

#include <QDebug>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLTimeMonitor>
#include <QtQml>

#include "drawlayer.h"
#include "hurricanelayer.h"
#include "maplayer.h"
#include "pointslayer.h"
#include "radarlayer.h"
#include "rasterimagelayer.h"

#include <vector>

DataCanvas::DataCanvas()
    : QQuickView(),
      funcs(nullptr),
      logger(new QOpenGLDebugLogger(this)),
      monitor(new QOpenGLTimeMonitor(this)),
      glInitialized(false),
      dragging(false),
      wheelDelta(0)
{
    // Set up OpenGL Context
    QSurfaceFormat fmt(QSurfaceFormat::DebugContext);
    fmt.setVersion(4, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setStencilBufferSize(8);
    fmt.setSwapBehavior(QSurfaceFormat::TripleBuffer);
    fmt.setSamples(4);

    // Set-up the view
    rootContext()->setContextProperty("viewProjection", &proj);
    setSource({"qrc:/qml/main.qml"});
    setResizeMode(QQuickView::SizeRootObjectToView);
    setClearBeforeRendering(false);
    setFormat(fmt);

    // Hook up this widget to draw within the QQuickView
    connect(this, &QQuickView::beforeRendering,
            this, &DataCanvas::renderGL, Qt::DirectConnection);
    connect(this, &QQuickView::sceneGraphInitialized,
            this, &DataCanvas::initGL, Qt::DirectConnection);
    connect(this, &QQuickView::sceneGraphInvalidated,
            this, &DataCanvas::cleanUpGL, Qt::DirectConnection);

    // Hook up debug messages
    connect(logger, &QOpenGLDebugLogger::messageLogged,
            this, &DataCanvas::logMessage, Qt::DirectConnection);

    QVariantMap config;
    addLayer(new RasterImageLayer(this));
//    config["filename"] = "/home/rmay/maps/dnb_land_ocean_ice.2012.3600x1800_geo.tif";
//    config["filename"] = "/home/rmay/maps/dnb_land_ocean_ice.2012.13500x6750_geo.tif";
//    config["filename"] = "/home/rmay/maps/world.topo.bathy.200406.3x21600x10800.jpg";
//    config["filename"] = "/home/rmay/maps/world.200406.3x5400x2700.jpg";
    config["filename"] = "/home/rmay/maps/world.topo.bathy.200406.3x5400x2700.png";
    layers.back()->configure(config);
    layers.back()->setObjectName("ImageLayer");

    addLayer(new DrawLayer(this));
    layers.back()->setObjectName("TestLayer");

    addLayer(new MapLayer(this));
    config["filename"] = "/home/rmay/maps/GSHHS_shp/f/GSHHS_f_L1.shp";
    config["color"] = "red";
    layers.back()->configure(config);
    layers.back()->setObjectName("CoastL1");

    addLayer(new MapLayer(this));
    config["filename"] = "/home/rmay/maps/GSHHS_shp/f/GSHHS_f_L2.shp";
    config["color"] = "orange";
    layers.back()->configure(config);
    layers.back()->setObjectName("CoastL2");

    addLayer(new MapLayer(this));
    config["filename"] = "/home/rmay/maps/WDBII_shp/f/WDBII_border_f_L1.shp";
    config["color"] = "cyan";
    layers.back()->configure(config);
    layers.back()->setObjectName("Borders");

    addLayer(new RadarLayer(this));
    layers.back()->configure(config);
    layers.back()->setObjectName("Radar");

    addLayer(new HurricaneLayer(this));
    layers.back()->setObjectName("Hurricane");
//    addLayer(new MapLayer(this));
//    config["filename"] = "/home/rmay/maps/tl_2013_us_county.shp";
//    config["color"] = "grey";
//    layers.back()->configure(config);
//    layers.back()->setObjectName("Counties");

//    addLayer(new MapLayer(this));
//    config["filename"] = "/home/rmay/maps/tl_2013_us_state.shp";
//    config["color"] = "black";
//    layers.back()->configure(config);
//    layers.back()->setObjectName("States");

//    addLayer(new MapLayer(this));
//    config["filename"] = "/home/rmay/maps/tl_2013_us_primaryroads.shp";
//    config["color"] = "blue";
//    layers.back()->configure(config);
//    layers.back()->setObjectName("Roads");

    addLayer(new PointsLayer(this));
    layers.back()->setObjectName("Points");

    proj.setScreenSize(size());
}

void DataCanvas::addLayer(DrawLayer* layer)
{
    layers.push_back(std::move(DrawLayerPtr(layer)));
    connect(this, &DataCanvas::sceneGraphInitialized,
            layer, &DrawLayer::init, Qt::DirectConnection);
    connect(this, &DataCanvas::sceneGraphInvalidated,
            layer, &DrawLayer::cleanUp, Qt::DirectConnection);
    connect(this, &DataCanvas::afterSynchronizing,
            layer, &DrawLayer::flushState, Qt::DirectConnection);
    connect(layer, &DrawLayer::needUpdate, this, &DataCanvas::update);
}

void DataCanvas::mousePressEvent(QMouseEvent *ev)
{
    QQuickView::mousePressEvent(ev);
    if (ev->button() == Qt::LeftButton)
    {
        dragging = true;
        dragPoint = ev->pos();
        ev->accept();
    }
}

void DataCanvas::mouseReleaseEvent(QMouseEvent *ev)
{
    QQuickView::mouseReleaseEvent(ev);
    if (ev->button() == Qt::LeftButton)
    {
        dragging = false;
        ev->accept();
    }
}

void DataCanvas::mouseMoveEvent(QMouseEvent *ev)
{
    QQuickView::mouseMoveEvent(ev);
    if (dragging)
    {
        proj.shift(dragPoint - ev->pos());
        dragPoint = ev->pos();
        ev->accept();
        update();
    }
}

void DataCanvas::wheelEvent(QWheelEvent *ev)
{
    static const int wheelZoomStep = 60;
    QQuickView::wheelEvent(ev);
    if (!ev->isAccepted())
    {
        wheelDelta += ev->angleDelta().y();
        if (wheelDelta > wheelZoomStep)
        {
            proj.zoomInTo(ev->pos());
            wheelDelta %= wheelZoomStep;
        }
        else if (wheelDelta < -wheelZoomStep)
        {
            proj.zoomOutFrom(ev->pos());
            wheelDelta = -(-wheelDelta % wheelZoomStep);
        }

        ev->accept();
        update();
    }
}

void DataCanvas::resizeEvent(QResizeEvent *ev)
{
    QQuickView::resizeEvent(ev);
    proj.setScreenSize(ev->size());
}

void DataCanvas::renderGL()
{
    funcs->glViewport(0, 0, size().width() * devicePixelRatio(),
                      size().height() * devicePixelRatio());
    funcs->glDepthRangef(-10.f, 10.f);
    funcs->glClearColor(0.19921875f, 0.44140625f, 0.45703125, 1.f);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projection().lock();

    // Start query timer
    monitor->recordSample();

    for (auto& layer: layers)
    {
        layer->draw();

        // Query after draw
        monitor->recordSample();
    }
    projection().unlock();

    // Get intervals (in nanoseconds)
    auto intervals = monitor->waitForIntervals();
    for (int i=0; i < intervals.size(); ++i)
//        qDebug() << layers[i]->objectName() << "took" << intervals[i] / 1e6 << "ms";

    // Reset timer
    monitor->reset();

    resetOpenGLState();
    glMessage("end render");
}

void DataCanvas::initGL()
{
    glInitialized = true;
    qDebug() << "initGL";
    // Set up debug logging
    if (!logger->initialize())
    {
        qDebug() << "Unable to initialize OpenGL debug logger";
    }
    else
    {
//        logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
//        logger->enableMessages();
    }

    // Get proper pointers to API functions
    funcs = openglContext()->functions();
    if (!funcs)
    {
        qFatal("Could not obtain OpenGL function pointers");
    }

    // Set up time monitor
    monitor->setSampleCount(layers.size() + 1);
    if (!monitor->create())
    {
        qWarning() << "Error creating timer query object";
    }
}

void DataCanvas::cleanUpGL()
{
    glInitialized = false;
}

void DataCanvas::logMessage(const QOpenGLDebugMessage &msg)
{
    qDebug() << msg;
}

void DataCanvas::glMessage(const QString& msg)
{
    auto message = QOpenGLDebugMessage::createApplicationMessage(msg);
    logger->logMessage(message);
}
