#include "drawlayer.h"

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "datacanvas.h"

DrawLayer::DrawLayer(DataCanvas *parent) :
    QObject(parent),
    dirty(false)
{
}

void DrawLayer::setDirty(bool d)
{
    dirty = d;
    if (dirty)
        emit needUpdate();
}

QOpenGLFunctions* DrawLayer::glFuncs() const
{
    return static_cast<DataCanvas*>(parent())->glFuncs();
}

ProjectionView& DrawLayer::projection() const
{
    return static_cast<DataCanvas*>(parent())->projection();
}

void DrawLayer::configure(const QVariantMap &config)
{
    for (const auto& key: config.keys())
    {
        this->setProperty(key.toUtf8(), *config.constFind(key));
    }
}

void DrawLayer::draw()
{
    prog->bind();

    QMatrix4x4 mvpMatrix;
    mvpMatrix.ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 10.f);
    prog->setUniformValue("mvp", mvpMatrix);

    // Separate block for handling binding/release of VAO
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);
        glFuncs()->glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    prog->release();
}

void DrawLayer::init()
{
    // Create shader program -- must be done with correct context active
    prog = QOpenGLShaderProgramPtr(new QOpenGLShaderProgram);

    // Load vertex shader
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                       ":/shaders/simple.vert"))
    {
        qWarning() << "Error compiling vertex shader: " << prog->log();
    }

    // Load fragment shader
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                       ":/shaders/simple.frag"))
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
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);

        // Load vertex data
        verts.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (!verts.create())
        {
            qWarning() << "Error creating vertex buffer";
        }
        else
        {
            std::vector<QVector2D> vertData{{-10., 0.}, {-8., 0.}, {-9., 1.}};
            verts.bind();
            verts.allocate(vertData.data(), vertData.size() * sizeof(QVector2D));
            prog->enableAttributeArray("vertex");
            prog->setAttributeBuffer("vertex", GL_FLOAT, 0, 2);
            verts.release();
        }

        // Load color data
        colors.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (!colors.create())
        {
            qWarning() << "Error creating colors buffer";
        }
        else
        {
            std::vector<QVector3D> colorData{{1., 0., 0.}, {0., 1., 0.},
                                             {0., 0., 1.}};
            colors.bind();
            colors.allocate(colorData.data(), colorData.size() * sizeof(QVector3D));
            prog->enableAttributeArray("color");
            prog->setAttributeBuffer("color", GL_FLOAT, 0, 3);
            colors.release();
        }
    }

    prog->release();
}

void DrawLayer::cleanUp()
{
    vao.destroy();
    verts.destroy();
    colors.destroy();
    prog.reset();
}

void DrawLayer::flushState()
{
    dirty = false;
}

