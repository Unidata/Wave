#include "ogrtools.h"

#include <tuple>

QDebug operator<<(QDebug dbg, OGRLayer* layer)
{
    layer->ResetReading();
    OGRFeatureDefn* def = layer->GetLayerDefn();
    FeaturePtr feat;
    while ((feat = FeaturePtr(layer->GetNextFeature())) != nullptr)
    {
        for (int i = 0; i < def->GetFieldCount(); ++i)
        {
            OGRFieldDefn* fdef = def->GetFieldDefn(i);
            switch (fdef->GetType())
            {
            case OFTInteger:
                dbg << fdef->GetNameRef() << feat->GetFieldAsInteger(i);
                break;
            case OFTReal:
                dbg << fdef->GetNameRef() << feat->GetFieldAsDouble(i);
                break;
            case OFTString:
                dbg << fdef->GetNameRef() << feat->GetFieldAsString(i);
                break;
            default:
                dbg << "Unknown type:" << fdef->GetType();
            }
        }
        dbg << "-----";
    }

    return dbg;
}

void countGeometry(OGRPolygon* poly, size_t &points, size_t &geom)
{
    auto extRing = poly->getExteriorRing();
    points += extRing->getNumPoints();
    geom += poly->getNumInteriorRings() + 1;
    for (int i = 0; i < poly->getNumInteriorRings(); ++i)
    {
        points += poly->getInteriorRing(i)->getNumPoints();
    }
}

std::tuple<size_t, size_t> countGeometry(OGRLayer* layer)
{
    FeaturePtr feat;
    size_t numVerts = 0, numObj = 0;
    while ((feat = FeaturePtr(layer->GetNextFeature())) != nullptr)
    {
        auto geom = feat->GetGeometryRef();
        switch (geom->getGeometryType())
        {
        case wkbPolygon:
        {
            countGeometry(static_cast<OGRPolygon*>(geom), numVerts, numObj);
            break;
        }
        case wkbMultiPolygon:
        {
            auto mpoly = static_cast<OGRMultiPolygon*>(geom);
            for (int i = 0; i < mpoly->getNumGeometries(); ++i)
            {
                countGeometry(static_cast<OGRPolygon*>(mpoly->getGeometryRef(i)),
                              numVerts, numObj);
            }
            break;
        }
        default:
            qWarning() << "Unhandled geometry type:" << geom->getGeometryName();
        }
    }
    return std::tuple<size_t, size_t>{numVerts, numObj};
}

void extractGeometry(OGRLinearRing *ring, FeatureInfo& info)
{
    info.starts.push_back(info.verts.size());
    info.counts.push_back(ring->getNumPoints());
    for (int p = 0; p < ring->getNumPoints(); ++p)
    {
        info.verts.emplace_back(ring->getX(p), ring->getY(p));
    }
}

void extractGeometry(OGRPolygon *poly, FeatureInfo& info)
{
    extractGeometry(poly->getExteriorRing(), info);
    for (int i = 0; i < poly->getNumInteriorRings(); ++i)
    {
        extractGeometry(poly->getInteriorRing(i), info);
    }
}

void extractGeometry(OGRLayer *layer, OGRCoordinateTransformation* trans,
                     FeatureInfo& info)
{
    FeaturePtr feat;
    while ((feat = FeaturePtr(layer->GetNextFeature())) != nullptr)
    {
        auto geom = feat->GetGeometryRef();
        geom->transform(trans);
        switch (geom->getGeometryType())
        {
        case wkbPolygon:
        {
            extractGeometry(static_cast<OGRPolygon*>(geom), info);
            break;
        }
        case wkbMultiPolygon:
        {
            auto mpoly = static_cast<OGRMultiPolygon*>(geom);
            for (int i = 0; i < mpoly->getNumGeometries(); ++i)
            {
                extractGeometry(static_cast<OGRPolygon*>(mpoly->getGeometryRef(i)),
                                info);
            }
            break;
        }
        default:
            qWarning() << "Unhandled geometry type:" << geom->getGeometryName();
        }
    }
}

FeatureInfo extractAllPoints(const QString& fname, OGRCoordinateTransformation *trans)
{
    FeatureInfo ret = FeatureInfo();

    DataSourcePtr ds(OGRSFDriverRegistrar::Open(fname.toLocal8Bit(), false));
    if (ds == nullptr)
    {
        qWarning() << "Error opening file:" << fname;
    }
    else
    {
        size_t verts, objs;
        auto layer = ds->GetLayer(0);
        layer->ResetReading();
        std::tie(verts, objs) = countGeometry(layer);
        qDebug() << "Points:" << verts << "Objects:" << objs;

        // Initialize the info
        ret.verts.reserve(verts);
        ret.starts.reserve(objs);
        ret.counts.reserve(objs);

        layer->ResetReading();
        extractGeometry(layer, trans, ret);
    }

    return ret;
}
