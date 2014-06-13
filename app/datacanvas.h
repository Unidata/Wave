#ifndef DATACANVAS_H
#define DATACANVAS_H

#include <QQuickView>

#include <memory>
#include <vector>

#include "drawlayer.h"
#include "projectionview.h"

class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QOpenGLFunctions;
class QOpenGLTimeMonitor;

struct DeleteLater
{
    void operator()(QObject *p) const
    {
        p->deleteLater();
    }
};

typedef std::unique_ptr<DrawLayer, DeleteLater> DrawLayerPtr;
typedef std::vector<DrawLayerPtr> LayerStore;

class DataCanvas : public QQuickView
{
    Q_OBJECT

    QOpenGLFunctions *funcs;
    QOpenGLDebugLogger *logger;
    QOpenGLTimeMonitor *monitor;
    bool glInitialized, dragging;
    QPoint dragPoint;
    int wheelDelta;

    LayerStore layers;
    ProjectionView proj;

public:
    explicit DataCanvas();
    void glMessage(const QString &msg);
    void addLayer(DrawLayer *layer);

    QOpenGLFunctions* glFuncs() const { return funcs; }
    ProjectionView& projection() { return proj; }

protected:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void resizeEvent(QResizeEvent *ev) override;

signals:

public slots:
    void renderGL();
    void initGL();
    void cleanUpGL();
    void logMessage(const QOpenGLDebugMessage &msg);
};

#endif // DATACANVAS_H
