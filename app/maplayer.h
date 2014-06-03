#ifndef MAPLAYER_H
#define MAPLAYER_H

#include "drawlayer.h"

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>

class MapLayer : public DrawLayer
{
    Q_OBJECT

    QOpenGLBuffer verts, indirect;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgramPtr prog;
    QOpenGLFunctions_4_3_Core *funcs;

public:
    explicit MapLayer(DataCanvas *parent = 0);

    void draw() override;
    void init() override;

signals:

public slots:

};

#endif // MAPLAYER_H
