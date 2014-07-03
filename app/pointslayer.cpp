#include "pointslayer.h"

#include <QQuickItem>
#include <QQuickTransform>
#include <QtQml>

#include "datacanvas.h"
#include "projectionview.h"

PointsLayer::PointsLayer(DataCanvas *parent) :
    DrawLayer(parent)
{
    auto dc = static_cast<DataCanvas*>(parent);
    auto com =  new QQmlComponent(dc->engine(),
                                  QUrl{"qrc:/qml/label.qml"}, this);

    item = qobject_cast<QQuickItem*>(com->create());
    if (item)
    {
        item->setParentItem(dc->rootObject());
        item->setVisible(true);
        item->setProperty("text", "testing");
        item->setProperty("center", QPointF(0., 0.f));
//        item->setPosition({-97., 35.});
    }
}
