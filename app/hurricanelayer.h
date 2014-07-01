#ifndef HURRICANELAYER_H
#define HURRICANELAYER_H

#include "drawlayer.h"

#include <QByteArray>
#include <QColor>
#include <QMap>
#include <QNetworkAccessManager>
#include <QPointF>

class QNetworkReply;

class DataCanvas;

class HurricaneLayer : public DrawLayer
{
    QNetworkAccessManager nam;
    QByteArray buffer;
    QMap<QString, QString> storms;
    std::vector<QVector2D> track;
    QNetworkReply *reply;
    bool newData;
    QOpenGLBuffer verts;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgramPtr prog;
    QColor m_color;

    void makeGL();

private slots:
    void loadData();
    void finishedList();
    void readyRead();
    void downloadProgress(qint64 recv, qint64 total);
    void finishedTrack();

public:
    HurricaneLayer(DataCanvas *parent);

public slots:
    void draw() override;
    void init() override;
//    void cleanUp() override;
};

#endif // HURRICANELAYER_H
