#include "hurricanelayer.h"

#include <QNetworkReply>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QUrl>

#include "projectionview.h"

HurricaneLayer::HurricaneLayer(DataCanvas *parent)
    : DrawLayer(parent),
      m_color("light green")
{
    QUrl url;
    url.setScheme("ftp");
    url.setHost("ftp.nhc.noaa.gov");
    url.setPath("atcf/index/master_list.txt");
    reply = nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &HurricaneLayer::finishedList);
    connect(reply, &QNetworkReply::readyRead, this, &HurricaneLayer::readyRead);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &HurricaneLayer::downloadProgress);
}

void HurricaneLayer::readyRead()
{
    qDebug() << "listReadyRead";
    buffer += reply->readAll();
}

void HurricaneLayer::downloadProgress(qint64 recv, qint64 total)
{
    qDebug() << "downloaded" << recv << "of" << total;
}

void HurricaneLayer::finishedList()
{
    qDebug() << "hurricane list finished";
    reply->deleteLater();
    for (QString line : QString(buffer).split('\n'))
    {
        auto items = line.split(',');
        if (items.size() == 2)
            storms[items[1].trimmed()] = items[0].trimmed();
    }

    qDebug() << storms;
    buffer.clear();

    QUrl url;
    url.setScheme("ftp");
    url.setHost("ftp.nhc.noaa.gov");
    url.setPath(QString("atcf/btk/b%1.dat").arg(storms[QString("Arthur").toUpper()].toLower()));
    qDebug() << url.toString();
    reply = nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &HurricaneLayer::finishedTrack);
    connect(reply, &QNetworkReply::readyRead, this, &HurricaneLayer::readyRead);
    connect(reply, &QNetworkReply::downloadProgress,
            this, &HurricaneLayer::downloadProgress);
}

void HurricaneLayer::finishedTrack()
{
    qDebug() << "hurricane track finished";
    reply->deleteLater();
    reply = nullptr;
    for (QString line : QString(buffer).split('\n'))
    {
        auto items = line.split(',');
        if (items.size() >= 11)
        {
            QString lonStr = items[7].trimmed();
            bool flip = lonStr[3] == QChar('W');
            lonStr.truncate(3);
            float lon = lonStr.toInt() / 10.f;
            if (flip)
                lon = -lon;

            QString latStr = items[6].trimmed();
            flip = latStr[3] == 'S';
            latStr.truncate(3);
            float lat = latStr.toInt() / 10.f;
            if (flip)
                lat = -lat;

            auto projPoint = projection().projectPoint({lon, lat});
            track.push_back(QVector2D(projPoint.x(), projPoint.y()));
        }
    }

    for (auto point : track)
        qDebug() << point;
    buffer.clear();
    setDirty(true);
}

void HurricaneLayer::init()
{
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
    }
    prog->release();
}

void HurricaneLayer::flushState()
{
    if (isDirty())
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);
        verts.bind();
        verts.allocate(track.data(), track.size() * sizeof(QVector2D));
        prog->enableAttributeArray("vertex");
        prog->setAttributeBuffer("vertex", GL_FLOAT, 0, 2);
        verts.release();
        setDirty(false);
    }
}

void HurricaneLayer::draw()
{
    prog->bind();

    prog->setUniformValue("mvp", projection().viewMatrix());
    prog->setUniformValue("color", m_color);

    // Separate block for handling binding/release of VAO
    {
        QOpenGLVertexArrayObject::Binder bind(&vao);
        glFuncs()->glDrawArrays(GL_LINE_STRIP, 0, track.size());
    }

    prog->release();
}
