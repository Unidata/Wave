#ifndef DATACANVAS_H
#define DATACANVAS_H

#include <QWidget>

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QSharedPointer>

class QQuickView;
class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QOpenGLFunctions;
class QOpenGLShaderProgram;
class QOpenGLTimeMonitor;

typedef QSharedPointer<QOpenGLShaderProgram> QOpenGLShaderProgramPtr;

class DataCanvas : public QWidget
{
    Q_OBJECT

    QOpenGLBuffer verts, colors;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgramPtr prog;
    QQuickView *qmlView;
    QOpenGLFunctions *funcs;
    QOpenGLDebugLogger *logger;
    QOpenGLTimeMonitor *monitor;

    void glMessage(const QString &msg);

public:
    explicit DataCanvas(QWidget *parent = 0);

signals:

public slots:
    void renderGL();
    void initGL();
    void cleanUpGL();
    void logMessage(const QOpenGLDebugMessage &msg);
};

#endif // DATACANVAS_H
