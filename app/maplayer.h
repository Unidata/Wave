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
    QString m_filename;
    bool newFile;

    void mapToGL();

private slots:
    void loadFile();

public:
    explicit MapLayer(DataCanvas *parent = 0);

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString filename READ filename WRITE setFilename NOTIFY filenameChanged)


    QColor color() const
    {
        return m_color;
    }

    QString filename() const
    {
        return m_filename;
    }

signals:
    void colorChanged(QColor arg);
    void filenameChanged(QString arg);

public slots:
    void draw() override;
    void init() override;
    void cleanUp() override;

    void setColor(QColor arg);
    void setFilename(QString arg);
};

#endif // MAPLAYER_H
