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
    bool dirty;

protected:
    QOpenGLFunctions* glFuncs() const;
    ProjectionView &projection() const;

signals:
    void needUpdate();

public:
    explicit DrawLayer(DataCanvas *parent = 0);

    bool isDirty() const { return dirty; }
    void setDirty(bool d);

public slots:
    virtual void configure(const QVariantMap &config);
    virtual void draw();
    virtual void init();
    virtual void cleanUp();
    virtual void flushState();
};

#endif // DRAWLAYER_H
