#include "projectionview.h"

#include <QRectF>

#include "ogr_spatialref.h"

const float zoomIncrement = 2.f;

ProjectionView::ProjectionView(QObject *parent) :
    QObject(parent),
    geogCoords(new OGRSpatialReference),
    projCoords(new OGRSpatialReference)
{
    geogCoords->SetWellKnownGeogCS("WGS84");
    projCoords->SetEquirectangular(35.f, -97.f, 0.f, 0.f);
    projCoords->SetLinearUnits("Kilometers", 1000.f);
    setupTransforms();

    m_scale = 1.f;
    updateMatrix();
}

void ProjectionView::setupTransforms()
{
    projTrans = std::unique_ptr<OGRCoordinateTransformation>(
                OGRCreateCoordinateTransformation(geogCoords.get(), projCoords.get()));
    geogTrans = std::unique_ptr<OGRCoordinateTransformation>(
                OGRCreateCoordinateTransformation(projCoords.get(), geogCoords.get()));
}

void ProjectionView::setGeogCS(OGRSpatialReference *coords)
{
    geogCoords = std::unique_ptr<OGRSpatialReference>(coords);
    setupTransforms();
}

void ProjectionView::setProjCS(OGRSpatialReference *coords)
{
    projCoords = std::unique_ptr<OGRSpatialReference>(coords);
    setupTransforms();
}

const QMatrix4x4& ProjectionView::viewMatrix() const
{
    return mvpMatrix;
}

void ProjectionView::updateMatrix()
{
    mvpMatrix = projMatrix;
    mvpMatrix.scale(m_scale, m_scale);
    mvpMatrix.translate(m_center.x(), m_center.y());
}

void ProjectionView::setScale(float scale)
{
    m_scale = qBound(1/1024.f, scale, 1024.f);
    updateMatrix();
}

void ProjectionView::zoomIn()
{
    setScale(m_scale * zoomIncrement);
}

void ProjectionView::zoomOut()
{
    setScale(m_scale / zoomIncrement);
}

void ProjectionView::shift(QPoint s)
{
    m_center -= QPointF(s.x(), -s.y()) / m_scale;
    updateMatrix();
}

void ProjectionView::setScreenSize(QSize size)
{
    projMatrix.setToIdentity();
    projMatrix.ortho(-size.width() / 2., size.width() / 2.,
                     -size.height() / 2., size.height() / 2.,
                     -10., 10.);
    updateMatrix();
}
