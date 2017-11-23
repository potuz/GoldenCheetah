/*
 * Copyright (c) 2016 Damien.Grauser (damien.grauser@pev-geneve.ch)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "FinalSurge.h"
#include "MainWindow.h"
#include "Athlete.h"
#include "Settings.h"
#include <QByteArray>
#include <QtDebug>
#include <QHttpMultiPart>

#ifndef FINALSURGE_DEBUG
#define FINALSURGE_DEBUG false
#endif

FinalSurge::FinalSurge(Context *context) : CloudService(context), context(context) {
    if (context) {
        nam = new QNetworkAccessManager(this);
        connect(nam, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this, SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> & )));
    }

    uploadCompression = none; // TODO: ask for compression methods
    downloadCompression = none;
    filetype = uploadType::FIT;
    useMetric = false; // distance and duration metadata

    // config
    settings.insert(OAuthToken, GC_FINALSURGE_TOKEN);
}

FinalSurge::~FinalSurge() {
    if (context) delete nam;
}

void
FinalSurge::onSslErrors(QNetworkReply *reply, const QList<QSslError>&)
{
    reply->ignoreSslErrors();
}

bool
FinalSurge::open(QStringList &errors)
{
    qDebug() << "FinalSurge::open\n"; 

    // do we have a token
    QString token = getSetting(GC_FINALSURGE_TOKEN, "").toString();
    if (token == "") {
        errors << "You must authorise with FinalSurge first";
        return false;
    }

    return true;
}

bool
FinalSurge::close()
{
    qDebug() << "FinalSurge::close\n";
    // nothing to do for now
    return true;
}

bool
FinalSurge::writeFile(QByteArray &data, QString remotename, RideFile *ride)
{
    qDebug() << "FinalSurge::writeFile(" << remotename << ")\n";

    // this must be performed asyncronously and call made
    // to notifyWriteCompleted(QString remotename, QString message) when done

    // do we have a token ?
    QString token = getSetting(GC_FINALSURGE_TOKEN, "").toString();
    if (token == "") return false;

    // lets connect and get basic info on the root directory
    QString url = QString("https://log.finalsurge.com/api/v1/uploads");

    qDebug() << "URL used: " << url << "\n";

    QNetworkRequest request = QNetworkRequest(url);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QString boundary = QVariant(qrand()).toString() +
        QVariant(qrand()).toString() + QVariant(qrand()).toString();
    multiPart->setBoundary(boundary.toLatin1());

    request.setRawHeader("authorization", (QString("Bearer %1").arg(token)).toLatin1());
    request.setRawHeader("client-id", GC_FINALSURGE_CLIENT_ID);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(
          QString("form-data; name=\"file\"; filename=\"%1\""
            ).arg(remotename).toLatin1()));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, 
        "application/octet-stream");
    filePart.setBody(data);
    multiPart->append(filePart);

    QNetworkReply *reply;
    reply = nam->post(request, multiPart);

    connect(reply, SIGNAL(finished()), this, SLOT(writeFileCompleted()));
    mapReply(reply,remotename);
    return true;
}

void
FinalSurge::writeFileCompleted()
{
    qDebug() << "FinalSurge::writeFileCompleted()\n"; 

    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());

    QByteArray r = reply->readAll();
    qDebug() << "reply:" << r << "\n";

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(r, &parseError);

    if (reply->error() == QNetworkReply::NoError) {
        QString name = replyName(static_cast<QNetworkReply*>(QObject::sender()));

        QJsonObject result = document.object();
        replyActivity.insert(name, result);

        notifyWriteComplete(
            name,
            tr("Completed."));
    } else {
        notifyWriteComplete(
            replyName(static_cast<QNetworkReply*>(QObject::sender())),
            tr("Network Error - Upload failed."));
    }
}

static bool addFinalSurge() {
    CloudServiceFactory::instance().addService(new FinalSurge(NULL));
    return true;
}

static bool add = addFinalSurge();

