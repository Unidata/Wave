#include "maplayer.h"

#include <QDebug>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QRectF>

#include "ogrtools.h"
#include "projectionview.h"

MapLayer::MapLayer(DataCanvas* parent) :
    DrawLayer(parent),
    newFile(false)
{
    OGRRegisterAll();
    loadFile(QLatin1Literal("/home/rmay/maps/tl_2013_us_state.shp"));
}

void MapLayer::loadFile(const QString& fname)
{
    auto fileInfo = extractAllPoints(fname, projection().transGeogToProj());
    vertData = std::move(fileInfo.verts);
    indirData.clear();
    indirData.reserve(fileInfo.counts.size());
    for (size_t i=0; i < fileInfo.counts.size(); ++i)
    {
        indirData.push_back({fileInfo.counts[i], 1, fileInfo.starts[i], 1});
    }
    newFile = true;
}

void MapLayer::draw()
{
    if (newFile)
        mapToGL();

    prog->bind();

    prog->setUniformValue("mvp", projection().viewMatrix());
    prog->setUniformValue("color", m_color);

    // Separate block for handling binding/release of VAO
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);
//        glFuncs()->glDrawArrays(GL_LINE_STRIP, 0, 5);
//        GLint first[2] = {0, 3};
//        GLint count[2] = {3, 2};
//        funcs->glMultiDrawArrays(GL_LINE_STRIP, first, count, 2);
        funcs->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect.bufferId());
        funcs->glMultiDrawArraysIndirect(GL_LINE_STRIP, 0, indirData.size(), 0);
        funcs->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }

    prog->release();
}

void MapLayer::init()
{
    funcs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_4_3_Core>();
    funcs->initializeOpenGLFunctions();
    // Create shader program -- must be done with correct context active
    prog = QOpenGLShaderProgramPtr(new QOpenGLShaderProgram);

    // Load vertex shader
    if (!prog->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                       ":/shaders/map.vert"))
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

    vao.create();

    prog->release();
}

void MapLayer::mapToGL()
{
    newFile = false;
    QOpenGLVertexArrayObject::Binder bind(&vao);

    verts.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!verts.create())
    {
        qWarning() << "Error creating vertex buffer";
    }
    else
    {
        verts.bind();
        verts.allocate(vertData.data(), vertData.size() * sizeof(QVector2D));
        prog->enableAttributeArray("vertex");
        prog->setAttributeBuffer("vertex", GL_FLOAT, 0, 2);
        verts.release();
    }

    indirect.setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!indirect.create())
    {
        qWarning() << "Error creating vertex buffer";
    }
    else
    {
        funcs->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect.bufferId());
        funcs->glBufferData(GL_DRAW_INDIRECT_BUFFER,
                            indirData.size() * sizeof(DrawArraysIndirectCommand),
                            indirData.data(), GL_STATIC_DRAW);
        funcs->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }
}

void MapLayer::cleanUp()
{
    verts.destroy();
    indirect.destroy();
    prog.reset();
}

void MapLayer::setColor(QColor arg)
{
    if (m_color != arg) {
        m_color = arg;
        emit colorChanged(arg);
    }
}
