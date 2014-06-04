#ifndef DRAWLAYER_H
#define DRAWLAYER_H

#include <QObject>

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QSharedPointer>

class QOpenGLShaderProgram;
class QOpenGLFunctions;

class DataCanvas;
class ProjectionView;

typedef QSharedPointer<QOpenGLShaderProgram> QOpenGLShaderProgramPtr;

class DrawLayer : public QObject
{
    Q_OBJECT

    QOpenGLBuffer verts, colors;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgramPtr prog;

public:
    virtual void draw();
    virtual void init();
    virtual void cleanUp();

    explicit DrawLayer(DataCanvas *parent = 0);

protected:
    QOpenGLFunctions* glFuncs() const;
    const ProjectionView &projection() const;

signals:

public slots:

};

#endif // DRAWLAYER_H
