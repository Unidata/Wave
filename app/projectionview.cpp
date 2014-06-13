#include "projectionview.h"

#include <QRectF>
#include <QVector2D>

#include "ogr_spatialref.h"

static const float zoomIncrement = 1.5f;

static const QRectF worldSpace = {-1.f, -1.f, 2.f, 2.f};

ProjectionView::ProjectionView(QObject *parent) :
    QObject(parent),
    geogCoords(new OGRSpatialReference),
    projCoords(new OGRSpatialReference),
    cameraLoc{0.f, 0.f, 1.f},
    lookAt{0.f, 0.f, -10.f},
    up{0.f, 1.f, 0.f},
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
                         1.f, -1.f);
        projMatrix.scale(scale(), scale());

        mvpMatrix = projMatrix;
        mvpMatrix.lookAt(cameraLoc, lookAt, up);

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
    QPointF delta = (cameraLoc.toPointF() - screenToProj.map(pt)) * (factor - 1);
    shift(QVector3D(delta));
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
    QPointF center = cameraLoc.toPointF();
    float totalScale = scale();

    // Limit X position within the box
    float scaledWidth = worldSpace.width() / totalScale;
    if (scaledWidth <= domain.width())
    {
        center.setX(qBound(domain.left() + 0.5f * scaledWidth,
                             center.x(),
                             domain.right() - 0.5f * scaledWidth));
    }
    else
    {
        center.setX(qBound(domain.right() - 0.5f * scaledWidth,
                             center.x(),
                             domain.left() + 0.5f * scaledWidth));
    }

    // Limit Y position within the box
    float scaledHeight = worldSpace.height() * aspect / totalScale;
    if (scaledHeight <= domain.height())
    {
        center.setY(qBound(domain.top() + 0.5f * scaledHeight,
                             center.y(),
                             domain.bottom() - 0.5f * scaledHeight));
    }
    else
    {
        center.setY(qBound(domain.bottom() - 0.5f * scaledHeight,
                             center.y(),
                             domain.top() + 0.5f * scaledHeight));
    }
    cameraLoc = QVector3D(QVector2D(center), cameraLoc.z());
    lookAt = QVector3D(QVector2D(center), lookAt.z());
    matrixChanged = true;
    updateMatrix();
}

void ProjectionView::shift(QVector3D delta)
{
    cameraLoc += delta;
    lookAt += delta;
    limitCenter();
}

void ProjectionView::shift(QPoint s)
{
    // Using the conversion from QPoint to QVector4D sets the w component to
    // 0, which effectively disables the translation aspect of the projection
    // giving only the scale, which is what we want (since the point coming
    // in is the amount of shift.
    shift(QVector3D(screenToProj.map(QVector4D(s))));
}

void ProjectionView::setScreenSize(QSize size)
{
    if (size.width() > 0 && size.height() > 0)
    {
        aspect = static_cast<float>(size.height()) / size.width();
        screenSize = size;
        updateWorldScale();

        // Matrix to generate normalized coordinates in OpenGL space (0,0) center
        // from Qt coordinates, where the lower left is (0,0)
        normMatrix.setToIdentity();
        normMatrix.translate(-1.f, 1.f);
        normMatrix.scale(2.f / size.width(), -2.f / size.height());

        matrixChanged = true;
        limitCenter();
    }
}
