#ifndef RADARLAYER_H
#define RADARLAYER_H

#include "drawlayer.h"

#include <vector>

#include <QOpenGLTexture>
#include <QVector2D>

class QOpenGLFunctions;
class QOpenGLShaderProgram;

class RadarLayer : public DrawLayer
{
    Q_OBJECT

    QString m_filename;
    bool newFile;

    QOpenGLBuffer verts, texCoords;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgramPtr prog;
    QOpenGLTexture tex;

    std::vector<QVector2D> vertData;
    std::vector<GLfloat> texData;
    QImage imData;

    void makeGL();

private slots:
    void loadFile();
public:
    explicit RadarLayer(DataCanvas *parent = 0);

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

#endif // RADARLAYER_H
