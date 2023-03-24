#ifndef SPMSG_SAMPLE_H
#define SPMSG_SAMPLE_H

#include <QDBusConnection>
#include <QDBusInterface>
#include "LFSadmin.h"
#include <unistd.h>
#include <QDBusReply>
#include <QVariant>

void postMessage(LPSTR lpstrObject_Name, DWORD uMsg, LPARAM lParam)
{
    //总线服务名
    QString service = LFS_MGR_MSG_NAME_PREF;
    service += "." + QString::number(getpid());

    QString path(lpstrObject_Name);
    QString interface = LFS_MGR_MSG_INTF_NAME;
    QString method = LFS_MGR_MSG_METHOD_NAME;

    QDBusInterface iface(service, path, interface);
    if (!iface.isValid()) {
        //print QDBusConnection::sessionBus().lastError().message();
    } else {
        QDBusReply<void> reply = iface.call(method, QVariant::fromValue(uMsg), QVariant::fromValue(lParam));
        if (reply.isValid()) {
            //post success
        } else {
            //print reply.error();
        }
    }
}



#endif // SPMSG_SAMPLE_H
