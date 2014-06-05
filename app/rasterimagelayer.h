#ifndef RASTERIMAGELAYER_H
#define RASTERIMAGELAYER_H

#include "drawlayer.h"

#include <vector>

#include <QOpenGLTexture>
#include <QVector2D>

class QOpenGLFunctions;
class QOpenGLShaderProgram;

class RasterImageLayer : public DrawLayer
{
    Q_OBJECT
    QString m_filename;
    bool newFile;

    QOpenGLBuffer verts, texCoords;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgramPtr prog;
    QOpenGLTexture tex;

    std::vector<QVector2D> vertData, texData;
    QImage imData;

    void makeGL();

private slots:
    void loadFile();

public:
    explicit RasterImageLayer(DataCanvas *parent = 0);

    Q_PROPERTY(QString filename READ filename WRITE setFilename NOTIFY filenameChanged)

    QString filename() const
    {
        return m_filename;
    }

signals:
    void filenameChanged(QString arg);

public slots:
    void setFilename(QString arg);

    void draw() override;
    void init() override;
    void cleanUp() override;
};

#endif // RASTERIMAGELAYER_H
