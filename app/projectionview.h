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
    QMatrix4x4 projMatrix, mvpMatrix;
    QPointF m_center;
    float m_scale;
    bool matrixLocked;

    void updateMatrix();
    void setupTransforms();

public:
    explicit ProjectionView(QObject *parent = 0);

    OGRCoordinateTransformation* transGeogToProj() const { return projTrans.get(); }

    const QMatrix4x4 &viewMatrix() const;
    void setScreenSize(QSize size);

    void setGeogCS(OGRSpatialReference *coords);
    void setProjCS(OGRSpatialReference *coords);

    void lock();
    void unlock();

signals:

public slots:
    void setScale(float scale);
    void zoomIn();
    void zoomOut();
    void shift(QPoint s);
};

#endif // PROJECTIONVIEW_H
