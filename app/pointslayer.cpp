#include "pointslayer.h"

#include <QQuickItem>
#include <QQuickTransform>
#include <QtQml>

#include "datacanvas.h"
#include "projectionview.h"

class GeographicTransform : public QQuickTransform
{
    QMatrix4x4 m;

public:
    GeographicTransform(QMatrix4x4 mat) : m(std::move(mat)) {}
    void applyTo(QMatrix4x4 *matrix) const
    {
        *matrix *= m;
    }
};

PointsLayer::PointsLayer(DataCanvas *parent) :
    DrawLayer(parent)
{
    auto dc = static_cast<DataCanvas*>(parent);
    auto root = dc->rootObject();

    auto engine = dc->engine();
    dc->rootContext()->setContextProperty("viewProjection", &projection());
    auto com =  new QQmlComponent(engine, QUrl{"qrc:/qml/label.qml"}, this);

    item = qobject_cast<QQuickItem*>(com->create());
    if (item)
    {
        item->setParentItem(root);
        item->setVisible(true);
        item->setProperty("text", "testing");
        item->setProperty("center", QPointF(0., 0.f));
//        item->setPosition({-97., 35.});
    }
}
