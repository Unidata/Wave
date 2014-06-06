#include "rasterimagelayer.h"

#include <QColor>
#include <QDebug>
#include <QImage>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "gdal_priv.h"
#include "ogr_spatialref.h"

#include "projectionview.h"

RasterImageLayer::RasterImageLayer(DataCanvas *parent) :
    DrawLayer(parent),
    newFile(false),
    tex(QOpenGLTexture::Target2D)
{
    GDALAllRegister();
    connect(this, &RasterImageLayer::filenameChanged,
            this, &RasterImageLayer::loadFile);
}

void RasterImageLayer::loadFile()
{
    GDALDataset *ds = static_cast<GDALDataset*>(GDALOpen(m_filename.toLocal8Bit(), GA_ReadOnly));
    if (ds == nullptr)
    {
        qWarning() << "Error opening file:" << m_filename;
    }
    else
    {
        projection().setGeogCS(new OGRSpatialReference(ds->GetProjectionRef()));
        projection().setProjCS(new OGRSpatialReference(ds->GetProjectionRef()));
        projection().setDomain({-180., -90., 360., 180.});

        std::vector<double> geoTransform(6);
        int xsize = ds->GetRasterXSize();
        int ysize = ds->GetRasterYSize();
        ds->GetGeoTransform(geoTransform.data());
        vertData.resize(4);
        vertData[0] = QVector2D(geoTransform[0], geoTransform[3]);
        vertData[1] = QVector2D(geoTransform[0] + geoTransform[2] * ysize,
                geoTransform[3] + geoTransform[5] * ysize);
        vertData[2] = QVector2D(geoTransform[0] + geoTransform[1] * xsize + geoTransform[2] * ysize,
                geoTransform[3] + geoTransform[4] * xsize + geoTransform[5] * ysize);
        vertData[3] = QVector2D(geoTransform[0] + geoTransform[1] * xsize,
                geoTransform[3] + geoTransform[4] * xsize);
        texData = {{0., 0.}, {0., 1.}, {1., 1.}, {1., 0.}};

        int numBands = ds->GetRasterCount();
        qDebug() << "Bands:" << numBands;

        imData = QImage(xsize, ysize, QImage::QImage::Format_RGBA8888);
        imData.fill(QColor(255, 255, 255, 255));
        // Bands start at 1
        for (int i = 1; i <= numBands; ++i)
        {
            GDALRasterBand *band = ds->GetRasterBand(i);
            switch(band->GetColorInterpretation())
            {
            case GCI_RedBand:
                band->RasterIO(GF_Read, 0, 0, xsize, ysize, imData.bits(),
                               xsize, ysize, GDT_Byte, 4, 0);
                break;
            case GCI_GreenBand:
                band->RasterIO(GF_Read, 0, 0, xsize, ysize, imData.bits() + 1,
                               xsize, ysize, GDT_Byte, 4, 0);
                break;
            case GCI_BlueBand:
                band->RasterIO(GF_Read, 0, 0, xsize, ysize, imData.bits() + 2,
                               xsize, ysize, GDT_Byte, 4, 0);
                break;
            default:
                qWarning() << "Unhandled color interpretation:" << band->GetColorInterpretation();
            }
        }

        GDALClose(ds);
        newFile = true;
    }
}

void RasterImageLayer::makeGL()
{
    newFile = false;
    QOpenGLVertexArrayObject::Binder bind(&vao);

    // Load vertex data
    verts.bind();
    verts.allocate(vertData.data(), vertData.size() * sizeof(QVector2D));
    prog->enableAttributeArray("vertex");
    prog->setAttributeBuffer("vertex", GL_FLOAT, 0, 2);
    verts.release();

    // Load texture coords
    texCoords.bind();
    texCoords.allocate(texData.data(), texData.size() * sizeof(QVector2D));
    prog->enableAttributeArray("texc");
    prog->setAttributeBuffer("texc", GL_FLOAT, 0, 2);
    texCoords.release();

    // Load texture
    tex.setData(imData);
}

void RasterImageLayer::setFilename(QString arg)
{
    if (m_filename != arg) {
        m_filename = arg;
        emit filenameChanged(arg);
    }
}

void RasterImageLayer::draw()
{
    if (newFile)
        makeGL();

    prog->bind();

    prog->setUniformValue("mvp", projection().viewMatrix());
    prog->setUniformValue("texture", 0);

    glEnable(GL_TEXTURE_2D);

    tex.bind(0);

    // Separate block for handling binding/release of VAO
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);
        glFuncs()->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    glDisable(GL_TEXTURE_2D);

    prog->release();
}

void RasterImageLayer::init()
{
    // Create shader program -- must be done with correct context active
    prog = QOpenGLShaderProgramPtr(new QOpenGLShaderProgram);

    // Load vertex shader
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                       ":/shaders/texture.vert"))
    {
        qWarning() << "Error compiling vertex shader: " << prog->log();
    }

    // Load fragment shader
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                       ":/shaders/texture.frag"))
    {
        qWarning() << "Error compiling fragment shader: " << prog->log();
    }

    // Link
    if (!prog->link())
    {
        qWarning() << "Error linking shader: " << prog->log();
    }
    else
    {
        prog->bind();
    }

    // Create Vertex Array Object for storing state. Create block for
    // binding VAO.
    vao.create();
    verts.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!verts.create())
    {
        qWarning() << "Error creating vertex buffer";
    }
    texCoords.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!texCoords.create())
    {
        qWarning() << "Error creating buffer for texture coords";
    }

    // Create texture handle
    if (!tex.create())
    {
        qWarning() << "Error creating texture";
    }
}

void RasterImageLayer::cleanUp()
{
//    tex.destroy();
}
