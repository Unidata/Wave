#ifndef MAPLAYER_H
#define MAPLAYER_H

#include "drawlayer.h"

#include <vector>

#include <QColor>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_4_3_Core>
#include <QVector2D>

typedef struct {
    uint  count;
    uint  instanceCount;
    uint  first;
    uint  baseInstance;
} DrawArraysIndirectCommand;

class MapLayer : public DrawLayer
{
    Q_OBJECT

    std::vector<DrawArraysIndirectCommand> indirData;
    std::vector<QVector2D> vertData;

    QOpenGLBuffer verts, indirect;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgramPtr prog;
    QOpenGLFunctions_4_3_Core *funcs;

    QColor m_color;
    bool newFile;

    void mapToGL();

public:
    explicit MapLayer(DataCanvas *parent = 0);

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    void loadFile(const QString &fname);

    QColor color() const
    {
        return m_color;
    }

signals:
    void colorChanged(QColor arg);

public slots:
    void draw() override;
    void init() override;
    void cleanUp() override;
    void setColor(QColor arg);

};

#endif // MAPLAYER_H
