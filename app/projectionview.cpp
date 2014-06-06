#include "projectionview.h"

#include <QRectF>

#include "ogr_spatialref.h"

static const float zoomIncrement = 1.5f;

ProjectionView::ProjectionView(QObject *parent) :
    QObject(parent),
    geogCoords(new OGRSpatialReference),
    projCoords(new OGRSpatialReference),
    matrixLocked(false)
{
    geogCoords->SetWellKnownGeogCS("WGS84");
    projCoords->SetEquirectangular(35.f, -97.f, 0.f, 0.f);
    projCoords->SetLinearUnits("Kilometers", 1000.f);
    domain = {-1e3, 1e3, 2e3, 2e3};
    setupTransforms();

    zoom = 1.f;
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

void ProjectionView::setDomain(QRectF dom)
{
    domain = std::move(dom);
    updateWorldScale();
}

void ProjectionView::updateWorldScale()
{
    // Change to qMax to set limits to fit smallest dimension of domain instead
    worldScale = qMin(screenSize.width() / domain.width(),
                      screenSize.height() / domain.height());
    updateMatrix();
}

const QMatrix4x4& ProjectionView::viewMatrix() const
{
    return mvpMatrix;
}

void ProjectionView::updateMatrix()
{
    if (!matrixLocked)
    {
        mvpMatrix = projMatrix;
        mvpMatrix.scale(scale(), scale());
        mvpMatrix.translate(m_center.x(), m_center.y());

        screenToProj = mvpMatrix.inverted() * normMatrix;
    }
}

void ProjectionView::lock()
{
    matrixLocked = true;
}

void ProjectionView::unlock()
{
    matrixLocked = false;
    updateMatrix();
}

float ProjectionView::setZoom(float scale)
{
    zoom = qBound(1.f, scale, 100.f);
    return zoom;
}

void ProjectionView::zoomInTo(QPoint pt)
{
    float oldZoom = zoom;
    zoomFixedPoint(pt, oldZoom / setZoom(zoom * zoomIncrement));
}

void ProjectionView::zoomOutFrom(QPoint pt)
{
    float oldZoom = zoom;
    zoomFixedPoint(pt, oldZoom / setZoom(zoom / zoomIncrement));
}

void ProjectionView::zoomFixedPoint(QPointF pt, float factor)
{
    auto mapPoint = screenToProj.map(pt);
    m_center = (mapPoint + m_center) * factor - mapPoint;
    limitCenter();
}

void ProjectionView::zoomIn()
{
    setZoom(zoom * zoomIncrement);
    limitCenter();
}

void ProjectionView::zoomOut()
{
    setZoom(zoom / zoomIncrement);
    limitCenter();
}

void ProjectionView::limitCenter()
{
    // Limit X position within the box
    float totalScale = scale();
    float scaledWidth = screenSize.width() / totalScale;
    if (scaledWidth <= domain.width())
    {
        m_center.setX(qBound(domain.left() + 0.5f * scaledWidth,
                             m_center.x(),
                             domain.right() - 0.5f * scaledWidth));
    }
    else
    {
        m_center.setX(qBound(domain.right() - 0.5f * scaledWidth,
                             m_center.x(),
                             domain.left() + 0.5f * scaledWidth));
    }

    // Limit Y position within the box
    float scaledHeight = screenSize.height() / totalScale;
    if (scaledHeight <= domain.height())
    {
        m_center.setY(qBound(domain.top() + 0.5f * scaledHeight,
                             m_center.y(),
                             domain.bottom() - 0.5f * scaledHeight));
    }
    else
    {
        m_center.setY(qBound(domain.bottom() - 0.5f * scaledHeight,
                             m_center.y(),
                             domain.top() + 0.5f * scaledHeight));
    }
    updateMatrix();
}

void ProjectionView::shift(QPoint s)
{
    m_center -= QPointF(s.x(), -s.y()) / scale();
    limitCenter();
}

void ProjectionView::setScreenSize(QSize size)
{
    projMatrix.setToIdentity();
    projMatrix.ortho(-size.width() / 2., size.width() / 2.,
                     -size.height() / 2., size.height() / 2.,
                     -10., 10.);

    // Matrix to generate normalized coordinates in OpenGL space (0,0) center
    // from Qt coordinates, where the lower left is (0,0)
    normMatrix.setToIdentity();
    normMatrix.translate(-1.f, 1.f);
    normMatrix.scale(2.f / size.width(), -2.f / size.height());
    screenSize = size;
    updateWorldScale();
    limitCenter();
}
