#include "projectionview.h"

#include <QRectF>

#include "gdal/ogr_spatialref.h"

const float zoomIncrement = 2.f;

ProjectionView::ProjectionView(QObject *parent) :
    QObject(parent),
    geogCoords(new OGRSpatialReference),
    projCoords(new OGRSpatialReference)
{
    geogCoords->SetWellKnownGeogCS("WGS84");
    projCoords->SetEquirectangular(35.f, -97.f, 0.f, 0.f);
    projCoords->SetLinearUnits("Kilometers", 1000.f);
    projTrans = std::unique_ptr<OGRCoordinateTransformation>(
                OGRCreateCoordinateTransformation(geogCoords.get(), projCoords.get()));
    geogTrans = std::unique_ptr<OGRCoordinateTransformation>(
                OGRCreateCoordinateTransformation(projCoords.get(), geogCoords.get()));

    m_scale = 1.f;
    updateMatrix();
}

const QMatrix4x4& ProjectionView::viewMatrix() const
{
    return mvpMatrix;
}

void ProjectionView::updateMatrix()
{
    mvpMatrix.setToIdentity();
    mvpMatrix.translate(m_center.x(), m_center.y());
    mvpMatrix.scale(m_scale, -m_scale);
    mvpMatrix *= projMatrix;
    qDebug() << mvpMatrix;
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
    m_center += s / m_scale;
    updateMatrix();
}

void ProjectionView::setScreenSize(QSize size)
{
    projMatrix.setToIdentity();
    projMatrix.ortho(QRectF{QPointF{-size.width() / 2.f, -size.height() / 2.f},
                            QSizeF{size}});
    updateMatrix();
}
