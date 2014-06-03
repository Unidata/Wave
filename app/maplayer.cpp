#include "maplayer.h"

#include <QDebug>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>


#include "gdal/ogrsf_frmts.h"

typedef  struct {
    uint  count;
    uint  instanceCount;
    uint  first;
    uint  baseInstance;
} DrawArraysIndirectCommand;

MapLayer::MapLayer(DataCanvas* parent) :
    DrawLayer(parent)
{
    OGRRegisterAll();

    OGRDataSource* ds;
    ds = OGRSFDriverRegistrar::Open("/home/rmay/maps/tl_2013_us_state.shp",
                                    false);

    OGRLayer* layer = ds->GetLayerByName("tl_2013_us_state");
    OGRFeatureDefn* def = layer->GetLayerDefn();
    qDebug() << layer->GetFeatureCount();
    OGRFeature* feat;
    while ((feat = layer->GetNextFeature()) != nullptr)
    {
        OGRGeometry* geom = feat->GetGeometryRef();
        for (int i = 0; i < def->GetFieldCount(); ++i)
        {
            OGRFieldDefn* fdef = def->GetFieldDefn(i);
            switch (fdef->GetType())
            {
            case OFTInteger:
                qDebug() << fdef->GetNameRef() << feat->GetFieldAsInteger(i);
                break;
            case OFTReal:
                qDebug() << fdef->GetNameRef() << feat->GetFieldAsDouble(i);
                break;
            case OFTString:
                qDebug() << fdef->GetNameRef() << feat->GetFieldAsString(i);
                break;
            default:
                qDebug() << "Unknown type:" << fdef->GetType();
            }
        }
        qDebug() << "-----";
    }
}

void MapLayer::draw()
{
    prog->bind();

    QMatrix4x4 mvpMatrix;
    mvpMatrix.ortho(-10.f, 10.f, -10.f, 10.f, -10.f, 10.f);
    prog->setUniformValue("mvp", mvpMatrix);
    prog->setUniformValue("color", QVector4D(0.2f, 1.0f, 1.0f, 1.0f));

    // Separate block for handling binding/release of VAO
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);
//        glFuncs()->glDrawArrays(GL_LINE_STRIP, 0, 5);
//        GLint first[2] = {0, 3};
//        GLint count[2] = {3, 2};
//        funcs->glMultiDrawArrays(GL_LINE_STRIP, first, count, 2);
        funcs->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect.bufferId());
        funcs->glMultiDrawArraysIndirect(GL_LINE_STRIP, 0, 2, 0);
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
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);

        verts.setUsagePattern(QOpenGLBuffer::StaticDraw);
        if (!verts.create())
        {
            qWarning() << "Error creating vertex buffer";
        }
        else
        {
            std::vector<QVector2D> vertData = {{-2., 0.}, {2., 0.}, {0., 2.}, {-5., 4.}, {-5, -4.}};
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
            std::vector<DrawArraysIndirectCommand> indirData = {{3, 1, 0, 1},
                                                                {2, 1, 3, 1}};
            glFuncs()->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect.bufferId());
//            indirect.allocate(indirData.data(),
//                              indirData.size() * sizeof(DrawArraysIndirectCommand));
            funcs->glBufferData(GL_DRAW_INDIRECT_BUFFER,
                                8 * sizeof(GLint),
                                indirData.data(), GL_STATIC_DRAW);
            glFuncs()->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
        }
    }

    prog->release();
}
