#include "datacanvas.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTimeMonitor>
#include <QQuickView>
#include <QWindow>

#include <vector>

DataCanvas::DataCanvas(QWidget *parent)
    : QWidget(parent),
      qmlView(new QQuickView),
      funcs(nullptr),
      logger(new QOpenGLDebugLogger(this)),
      monitor(new QOpenGLTimeMonitor(this))
{
    // Set up OpenGL Context
    QSurfaceFormat fmt(QSurfaceFormat::DebugContext);
    fmt.setVersion(3, 2);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setStencilBufferSize(8);

    // Set-up the view
    qmlView->setSource({"qrc:/qml/main.qml"});
    qmlView->setResizeMode(QQuickView::SizeRootObjectToView);
    qmlView->setClearBeforeRendering(false);
    qmlView->setFormat(fmt);

    // Create a widget container for the view and add to a layout. The container
    // takes ownership of the view.
    auto container = createWindowContainer(qmlView, this);
    auto layout = new QHBoxLayout(this);
    layout->setMargin(20);
    layout->addWidget(container);

    // Hook up this widget to draw within the QQuickView
    connect(qmlView, &QQuickView::beforeRendering,
            this, &DataCanvas::renderGL, Qt::DirectConnection);
    connect(qmlView, &QQuickView::sceneGraphInitialized,
            this, &DataCanvas::initGL, Qt::DirectConnection);
    connect(qmlView, &QQuickView::sceneGraphInvalidated,
            this, &DataCanvas::cleanUpGL, Qt::DirectConnection);

    // Hook up debug messages
    connect(logger, &QOpenGLDebugLogger::messageLogged,
            this, &DataCanvas::logMessage, Qt::DirectConnection);
}

void DataCanvas::renderGL()
{
    // Start query timer
//    monitor->recordSample();

    auto size = qmlView->size();
    funcs->glViewport(0, 0, size.width(), size.height());

    funcs->glClearColor(1.f, 1.f, 1.f, 1.f);
    funcs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog->bind();

    QMatrix4x4 mvpMatrix;
    mvpMatrix.ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 10.f);
    prog->setUniformValue("mvp", mvpMatrix);

    // Separate block for handling binding/release of VAO
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);
        funcs->glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    prog->release();

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
    funcs = qmlView->openglContext()->functions();
    if (!funcs)
    {
        qFatal("Could not obtain OpenGL function pointers");
    }

    // Set up time monitor
//    if (!monitor->create())
//    {
//        qWarning() << "Error creating timer query object";
//    }

    // Create shader program -- must be done with correct context active
    prog = QOpenGLShaderProgramPtr(new QOpenGLShaderProgram);

    // Load vertex shader
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                       ":/shaders/simple.vert"))
    {
        qWarning() << "Error compiling vertex shader: " << prog->log();
    }

    // Load fragment shader
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                       ":/shaders/simple.frag"))
    {
        qWarning() << "Error compiling fragment shader: " << prog->log();
    }

    // Link
    if (!prog->link())
    {
        qWarning() << "Error linking shader: " << prog->log();
    }
    else
    {
        prog->bind();
    }

    // Create Vertex Array Object for storing state. Create block for
    // binding VAO.
    vao.create();
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);

        // Load vertex data
        verts.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (!verts.create())
        {
            qWarning() << "Error creating vertex buffer";
        }
        else
        {
            std::vector<QVector2D> vertData{{-1., 0.}, {1., 0.}, {0., 1.}};
            verts.bind();
            verts.allocate(vertData.data(), vertData.size() * sizeof(QVector2D));
            prog->enableAttributeArray("vertex");
            prog->setAttributeBuffer("vertex", GL_FLOAT, 0, 2);
            verts.release();
        }

        // Load color data
        colors.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (!colors.create())
        {
            qWarning() << "Error creating colors buffer";
        }
        else
        {
            std::vector<QVector3D> colorData{{1., 0., 0.}, {0., 1., 0.},
                                             {0., 0., 1.}};
            colors.bind();
            colors.allocate(colorData.data(), colorData.size() * sizeof(QVector3D));
            prog->enableAttributeArray("color");
            prog->setAttributeBuffer("color", GL_FLOAT, 0, 3);
            colors.release();
        }
    }

    prog->release();
}

void DataCanvas::cleanUpGL()
{
    qDebug() << "cleanUpGL";
//    vao.destroy();
//    verts.destroy();
//    colors.destroy();
//    prog.reset();
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
