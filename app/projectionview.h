#ifndef PROJECTIONVIEW_H
#define PROJECTIONVIEW_H

#include <QObject>

#include <memory>

#include <QMatrix4x4>

#include "ogr_spatialref.h"

class ProjectionView : public QObject
{
    Q_OBJECT

    std::unique_ptr<OGRSpatialReference> geogCoords, projCoords;
    std::unique_ptr<OGRCoordinateTransformation> projTrans, geogTrans;
    QMatrix4x4 projMatrix, mvpMatrix, normMatrix, screenToProj;
    QPointF m_center;
    QRectF domain;
    QSize screenSize;
    float zoom, worldScale;
    bool matrixLocked;

    void updateMatrix();
    void updateWorldScale();
    void setupTransforms();
    void limitCenter();
    void zoomFixedPoint(QPointF pt, float factor);

public:
    explicit ProjectionView(QObject *parent = 0);

    OGRCoordinateTransformation* transGeogToProj() const { return projTrans.get(); }

    const QMatrix4x4 &viewMatrix() const;
    void setScreenSize(QSize size);

    void setGeogCS(OGRSpatialReference *coords);
    void setProjCS(OGRSpatialReference *coords);
    void setDomain(QRectF dom);

    void lock();
    void unlock();

    inline float scale() const { return zoom * worldScale; }

signals:

public slots:
    float setZoom(float scale);
    void zoomIn();
    void zoomInTo(QPoint pt);
    void zoomOut();
    void zoomOutFrom(QPoint pt);
    void shift(QPoint s);
};

#endif // PROJECTIONVIEW_H
