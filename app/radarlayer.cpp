#include "radarlayer.h"

#include <QColor>
#include <QDebug>
#include <QImage>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <string>
#include <vector>

#include <QDomDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

#include <netcdf>

#include "projectionview.h"

using namespace netCDF;

QDebug operator<<(QDebug dbg, const std::string& str)
{
    return dbg << str.c_str();
}

RadarLayer::RadarLayer(DataCanvas *parent) :
    DrawLayer(parent),
    tex(QOpenGLTexture::Target1D),
    reply(nullptr)
{
    connect(this, &RadarLayer::filenameChanged, this, &RadarLayer::loadFile);
}


void RadarLayer::loadFile()
{
    QUrlQuery query;
    query.addQueryItem("var", "N0R");
    query.addQueryItem("stn", "FTG");
    query.addQueryItem("time", "present");

    QUrl url;
    url.setScheme("http");
    url.setHost("thredds.ucar.edu");
    url.setPath("/thredds/radarServer/nexrad/level3/IDD");
    url.setQuery(query);
    reply = nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &RadarLayer::httpFinished);
    connect(reply, &QNetworkReply::readyRead, this, &RadarLayer::httpReadyRead);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &RadarLayer::updateDataReadProgress);
}

void RadarLayer::httpFinished()
{
    qDebug() << "httpFinished";
    reply->deleteLater();

    QDomDocument xml;
    xml.setContent(buffer);
    buffer.clear();

    auto elems = xml.elementsByTagName("service");
    QString entryPoint;
    for (int i=0; i < elems.count(); ++i)
    {
        auto attrs = elems.at(i).attributes();
        auto serviceName = attrs.namedItem("name").toAttr().value();
        if (serviceName == "OPENDAP")
            entryPoint = attrs.namedItem("base").toAttr().value();
    }
    qDebug() << entryPoint;

    elems = xml.elementsByTagName("dataset");
    QStringList paths;
    for (int i=0; i < elems.count(); ++i)
    {
        auto attrs = elems.at(i).attributes();
        if (attrs.contains("urlPath"))
        {
            qDebug() << "dataset:" << attrs.namedItem("name").toAttr().value();
            paths.append(attrs.namedItem("urlPath").toAttr().value());
        }
    }
    qDebug() << paths;

    QUrl url;
    url.setScheme("http");
    url.setHost("thredds.ucar.edu");
    url.setPath(entryPoint + paths[0]);
    qDebug() << url;

    try
    {
        NcFile nc(url.toString().toStdString(), NcFile::read);
        NcVar data;
        NcVar latVar = nc.getVar("latitude");
        std::vector<float> buf(latVar.getDim(0).getSize());
        latVar.getVar(buf.data());
        float lat = buf[0];

        NcVar lonVar = nc.getVar("longitude");
        buf.resize(lonVar.getDim(0).getSize());
        lonVar.getVar(buf.data());
        float lon = buf[0];

        QPointF center = projection().projectPoint({lon, lat});
        qDebug() << center;

        for (auto& varInfo : nc.getVars())
        {
            qDebug() << varInfo.first;
            NcVar var = varInfo.second;
            if (var.getDimCount() == 2)
                data = var;
        }

        auto dims = data.getDims();
        qDebug() << data.getName();
        for (NcDim dim : dims)
            qDebug() << dim.getName();

        auto numAz = nc.getDim("azimuth").getSize();
        auto numGates = nc.getDim("gate").getSize();
        setDirty(true);
    }
    catch (exceptions::NcException& e)
    {
        qDebug() << "NetCDF Error:" << e.what();
    }
}

void RadarLayer::httpReadyRead()
{
    qDebug() << "httpReadyRead";
    buffer += reply->readAll();
}

void RadarLayer::updateDataReadProgress(qint64 recv, qint64 total)
{
    qDebug() << "downloaded" << recv << "of" << total;
}

void RadarLayer::flushState()
{
    if(isDirty())
    {
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
        setDirty(false);
    }
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
