#ifndef DATACANVAS_H
#define DATACANVAS_H

#include <QQuickView>

#include <memory>
#include <vector>

#include "drawlayer.h"

class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class QOpenGLFunctions;
class QOpenGLTimeMonitor;

typedef std::unique_ptr<DrawLayer> DrawLayerPtr;
typedef std::vector<DrawLayerPtr> LayerStore;

class DataCanvas : public QQuickView
{
    Q_OBJECT

    QOpenGLFunctions *funcs;
    QOpenGLDebugLogger *logger;
    QOpenGLTimeMonitor *monitor;
    bool glInitialized;

    LayerStore layers;

public:
    explicit DataCanvas();
    void glMessage(const QString &msg);
    QOpenGLFunctions* glFuncs() const { return funcs; }
    void addLayer(DrawLayer *layer);

signals:

public slots:
    void renderGL();
    void initGL();
    void cleanUpGL();
    void logMessage(const QOpenGLDebugMessage &msg);
};

#endif // DATACANVAS_H
