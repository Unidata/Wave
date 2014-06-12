#include "projectionview.h"

#include <QRectF>

#include "ogr_spatialref.h"

static const float zoomIncrement = 1.5f;

static const QRectF worldSpace = {-1.f, -1.f, 2.f, 2.f};

ProjectionView::ProjectionView(QObject *parent) :
    QObject(parent),
    geogCoords(new OGRSpatialReference),
    projCoords(new OGRSpatialReference),
    zoom(1.f),
    worldScale(1.f),
    aspect(1.f),
    matrixLocked(false),
    matrixChanged(true)
{
    geogCoords->SetWellKnownGeogCS("WGS84");
    projCoords->SetEquirectangular(35.f, -97.f, 0.f, 0.f);
    projCoords->SetLinearUnits("Kilometers", 1000.f);
    domain = {-1e3, 1e3, 2e3, 2e3};
    setupTransforms();
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
    worldScale = qMin(worldSpace.width() / domain.width(),
                      aspect * worldSpace.height() / domain.height());
    matrixChanged = true;
    updateMatrix();
}


void ProjectionView::updateMatrix()
{
    if (!matrixLocked && matrixChanged)
    {
        projMatrix.setToIdentity();
        // Bottom/Top flipped on QRect since they assume origin is upper left,
        // instead of the lower left we gave it.
        projMatrix.ortho(worldSpace.left(), worldSpace.right(),
                         aspect * worldSpace.top(), aspect * worldSpace.bottom(),
                         -10.f, 10.f);
        projMatrix.scale(scale(), scale(), 1.f);

        mvpMatrix = projMatrix;
        mvpMatrix.lookAt(QVector3D{-m_center.x(), -m_center.y(), 0.f},
                         QVector3D{-m_center.x(), -m_center.y(), -1.f},
                         QVector3D{0.f, 1.f, 0.f});

        screenToProj = mvpMatrix.inverted() * normMatrix;
//        projToScreen = normMatrix.inverted() * mvpMatrix;
        projToScreen = screenToProj.inverted();

        matrixChanged = false;
        emit viewMatrixChanged(mvpMatrix);
        emit screenMatrixChanged(projToScreen);
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
    matrixChanged = true;
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
    float scaledWidth = worldSpace.width() / totalScale;
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
    float scaledHeight = worldSpace.height() * aspect / totalScale;
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
    matrixChanged = true;
    updateMatrix();
}

void ProjectionView::shift(QPoint s)
{
    m_center -= QPointF(s.x(), -s.y()) / (scale() * screenSize.width() / worldSpace.width());
    limitCenter();
}

void ProjectionView::setScreenSize(QSize size)
{
    screenSize = size;
    aspect = static_cast<float>(size.height()) / size.width();
    updateWorldScale();
    qDebug() << size << aspect << m_center;

//    projMatrix.setToIdentity();
//    projMatrix.ortho(-size.width() / 2., size.width() / 2.,
//                     -size.height() / 2., size.height() / 2.,
//                     -10., 10.);
//    projMatrix.ortho(-size.width() / 2., size.width() / 2.,
//                     -size.height() / 2., size.height() / 2.,
//                     -10., 10.);
//    projMatrix.scale(scale(), scale());

    // Matrix to generate normalized coordinates in OpenGL space (0,0) center
    // from Qt coordinates, where the lower left is (0,0)
    normMatrix.setToIdentity();
    normMatrix.translate(-1.f, 1.f);
    normMatrix.scale(2.f / size.width(), -2.f / size.height());

    matrixChanged = true;
    limitCenter();
}
