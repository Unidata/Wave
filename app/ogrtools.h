#ifndef OGRTOOLS_H
#define OGRTOOLS_H

#include <memory>
#include <vector>

#include <QDebug>
#include <QVector2D>

#include "gdal/ogr_feature.h"
#include "gdal/ogrsf_frmts.h"

class OGRCoordinateTransformation;

struct FeatureDeleter
{
    void operator()(OGRFeature *feat) const
    {
        OGRFeature::DestroyFeature(feat);
    }
};

struct DataSourceDeleter
{
    void operator()(OGRDataSource *ds) const
    {
        OGRDataSource::DestroyDataSource(ds);
    }
};

typedef std::unique_ptr<OGRFeature, FeatureDeleter> FeaturePtr;
typedef std::unique_ptr<OGRDataSource, DataSourceDeleter> DataSourcePtr;

struct FeatureInfo
{
    std::vector<QVector2D> verts;
    std::vector<uint> starts, counts;
};

QDebug operator<<(QDebug dbg, OGRLayer* layer);

FeatureInfo extractAllPoints(const QString& fname, OGRCoordinateTransformation *trans);

#endif // OGRTOOLS_H
