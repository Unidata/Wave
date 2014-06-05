#include "datacanvas.h"

#include <QDebug>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLTimeMonitor>

#include "drawlayer.h"
#include "maplayer.h"

#include <vector>

DataCanvas::DataCanvas()
    : QQuickView(),
      funcs(nullptr),
      logger(new QOpenGLDebugLogger(this)),
      monitor(new QOpenGLTimeMonitor(this)),
      glInitialized(false),
      dragging(false)
{
    // Set up OpenGL Context
    QSurfaceFormat fmt(QSurfaceFormat::DebugContext);
    fmt.setVersion(4, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setStencilBufferSize(8);

    // Set-up the view
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

    addLayer(new DrawLayer(this));
    addLayer(new MapLayer(this));

    proj.setScreenSize(size());
}

void DataCanvas::addLayer(DrawLayer* layer)
{
    layers.push_back(std::move(DrawLayerPtr(layer)));
    connect(this, &QQuickView::sceneGraphInitialized,
            layer, &DrawLayer::init, Qt::DirectConnection);
    connect(this, &QQuickView::sceneGraphInvalidated,
            layer, &DrawLayer::cleanUp, Qt::DirectConnection);
}

void DataCanvas::mousePressEvent(QMouseEvent *ev)
{
    QQuickView::mousePressEvent(ev);
    if (!ev->isAccepted() && ev->button() == Qt::LeftButton)
    {
        dragging = true;
        dragPoint = ev->pos();
        ev->accept();
    }
}

void DataCanvas::mouseReleaseEvent(QMouseEvent *ev)
{
    QQuickView::mouseReleaseEvent(ev);
    if (!ev->isAccepted() && ev->button() == Qt::LeftButton)
    {
        dragging = false;
        ev->accept();
    }
}

void DataCanvas::mouseMoveEvent(QMouseEvent *ev)
{
    QQuickView::mouseMoveEvent(ev);
    if (!ev->isAccepted() && dragging)
    {
        proj.shift(dragPoint - ev->pos());
        dragPoint = ev->pos();
        ev->accept();
        update();
    }
}

void DataCanvas::wheelEvent(QWheelEvent *ev)
{
    QQuickView::wheelEvent(ev);
    if (!ev->isAccepted())
    {
        if (ev->angleDelta().y() > 0.f)
            proj.zoomIn();
        else
            proj.zoomOut();

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
    // Start query timer
//    monitor->recordSample();

    auto size = this->size();
    funcs->glViewport(0, 0, size.width(), size.height());

    funcs->glClearColor(1.f, 1.f, 1.f, 1.f);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& layer: layers)
        layer->draw();

    // Query after draw
//    monitor->recordSample();

    // Get intervals (in nanoseconds)
//    auto intervals = monitor->waitForIntervals();
//    for (const auto val: intervals)
//        qDebug() << "Draw took" << val / 1e6 << "ms";

    // Reset timer
//    monitor->reset();

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
//    if (!monitor->create())
//    {
//        qWarning() << "Error creating timer query object";
//    }
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
