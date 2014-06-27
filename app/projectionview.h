#ifndef PROJECTIONVIEW_H
#define PROJECTIONVIEW_H

#include <QObject>

#include <memory>

#include <QMatrix4x4>
#include <QPointF>

#include "ogr_spatialref.h"

class ProjectionView : public QObject
{
    Q_OBJECT

    std::unique_ptr<OGRSpatialReference> geogCoords, projCoords;
    std::unique_ptr<OGRCoordinateTransformation> projTrans, geogTrans;
    QMatrix4x4 projMatrix, mvpMatrix, normMatrix, screenToProj, projToScreen;
    QRectF domain;
    QSize screenSize;
    QVector3D cameraLoc, lookAt, up;
    float zoom, worldScale, aspect;
    bool matrixLocked, matrixChanged;

    void updateMatrix();
    void updateWorldScale();
    void setupTransforms();
    void limitCenter();
    void zoomFixedPoint(QPointF pt, float factor);
    void shift(QVector3D delta);

public:
    explicit ProjectionView(QObject *parent = 0);

    OGRCoordinateTransformation* transGeogToProj() const { return projTrans.get(); }

    Q_PROPERTY(QMatrix4x4 viewMatrix READ viewMatrix NOTIFY viewMatrixChanged)
    Q_PROPERTY(QMatrix4x4 screenMatrix READ screenMatrix NOTIFY screenMatrixChanged)

    void setScreenSize(QSize size);

    void setGeogCS(OGRSpatialReference *coords);
    void setProjCS(OGRSpatialReference *coords);
    void setDomain(QRectF dom);

    void lock();
    void unlock();

    inline float scale() const { return zoom * worldScale; }

    const QMatrix4x4& viewMatrix() const
    {
        return mvpMatrix;
    }

    const QMatrix4x4& screenMatrix() const
    {
        return projToScreen;
    }

    Q_INVOKABLE QPointF transScreenToProj(float x, float y)
    {
        return screenToProj.map(QPointF{x, y});
    }

signals:
    void viewMatrixChanged(QMatrix4x4 arg);
    void screenMatrixChanged(QMatrix4x4 arg);

public slots:
    float setZoom(float scale);
    void zoomIn();
    void zoomInTo(QPoint pt);
    void zoomOut();
    void zoomOutFrom(QPoint pt);
    void shift(QPoint s);
};

#endif // PROJECTIONVIEW_H
