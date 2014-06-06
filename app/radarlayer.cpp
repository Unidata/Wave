#include "radarlayer.h"

#include <QColor>
#include <QDebug>
#include <QImage>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <QDomDocument>

#include "projectionview.h"

RadarLayer::RadarLayer(DataCanvas *parent) :
    DrawLayer(parent),
    newFile(false),
    tex(QOpenGLTexture::Target1D)
{
}


void RadarLayer::loadFile()
{
    newFile = false;

//    QXMLDoc http://thredds.ucar.edu/thredds/radarServer/nexrad/level3/IDD?var=N0R&stn=FTG&time=present
}

void RadarLayer::makeGL()
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
    texCoords.allocate(texData.data(), texData.size() * sizeof(GLfloat));
    prog->enableAttributeArray("texc");
    prog->setAttributeBuffer("texc", GL_FLOAT, 0, 1);
    texCoords.release();

    // Load texture
//    tex.setData(imData);
}

void RadarLayer::setFilename(QString arg)
{
    if (m_filename != arg) {
        m_filename = arg;
        emit filenameChanged(arg);
    }
}

void RadarLayer::draw()
{
    if (newFile)
        makeGL();

    prog->bind();

    prog->setUniformValue("mvp", projection().viewMatrix());
//    prog->setUniformValue("texture", 0);
//    glEnable(GL_TEXTURE_2D);
//    tex.bind(0);

    // Separate block for handling binding/release of VAO
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);
        glFuncs()->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
//    glDisable(GL_TEXTURE_2D);

    prog->release();
}

void RadarLayer::init()
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

void RadarLayer::cleanUp()
{
//    tex.destroy();
}
