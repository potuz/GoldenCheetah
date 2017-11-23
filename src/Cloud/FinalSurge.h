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

#ifndef GC_FinalSurge_h
#define GC_FinalSurge_h

#include "CloudService.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QImage>

class FinalSurge : public CloudService {

    Q_OBJECT

    public:

        FinalSurge(Context *context);
        CloudService *clone(Context *context) { return new FinalSurge(context); }
        ~FinalSurge();

        QString id() const { return "Final Surge"; }
        QString uiName() const { return tr("Final Surge"); }
        QString description() const { return (tr("The Training Log for Athletes, Coaches, Teams && Clubs")); }
        QImage logo() const { return QImage(":images/services/finalsurge.png"); }

        int capabilities() const { return OAuth | Upload; }

        // open/connect and close/disconnect
        bool open(QStringList &errors);
        bool close();

        // write a file
        bool writeFile(QByteArray &data, QString remotename, RideFile *ride);

    public slots:
      
        // sending data
        void writeFileCompleted();

    private:
        Context *context;
        QNetworkAccessManager *nam;

        QMap<QString, QJsonObject> replyActivity;

    private slots:
        void onSslErrors(QNetworkReply *reply, const QList<QSslError>&error);
};
#endif
