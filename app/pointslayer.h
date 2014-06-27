#ifndef POINTSLAYER_H
#define POINTSLAYER_H

#include "drawlayer.h"

class QQuickItem;

class PointsLayer : public DrawLayer
{
    Q_OBJECT

    QQuickItem *item;

public:
    explicit PointsLayer(DataCanvas *parent = 0);

signals:

public slots:
//    void draw() override;

};

#endif // POINTSLAYER_H
