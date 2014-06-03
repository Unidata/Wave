#ifndef PROJECTIONVIEW_H
#define PROJECTIONVIEW_H

#include <QObject>

#include <memory>

#include "gdal/ogr_spatialref.h"


class ProjectionView : public QObject
{
    Q_OBJECT

    std::unique_ptr<OGRSpatialReference> geogCoords, projCoords;
    std::unique_ptr<OGRCoordinateTransformation> projTrans, geogTrans;

public:
    explicit ProjectionView(QObject *parent = 0);

signals:

public slots:

};

#endif // PROJECTIONVIEW_H
