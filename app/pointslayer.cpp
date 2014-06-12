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

    auto base = new QQuickItem(root);
    auto engine = dc->engine();
    dc->rootContext()->setContextProperty("viewProjection", &projection());
    auto com =  new QQmlComponent(engine, QUrl{"qrc:/qml/label.qml"}, this);

    item = qobject_cast<QQuickItem*>(com->create());
    if (item)
    {
        item->setParentItem(base);
        item->setVisible(true);
        item->setProperty("text", "testing");
        item->setPosition({0., 0.});
    }
}

void PointsLayer::draw()
{
//    QQmlListProperty<QQuickTransform> trans;
//    trans.
//    trans.append(new GeographicTransform(projection().viewMatrix()));
//    item->setProperty("transform", trans);
//    item->setProperty("transform", new GeographicTransform(projection().viewMatrix()));
//    item->transform() << GeographicTransform(projection().viewMatrix());
//    auto dc = static_cast<DataCanvas*>(parent());
//    dc->rootContext()->setContextProperty("geogTransform", projection().viewMatrix());
//    item->polish();
    qDebug() << projection().screenMatrix();
}
