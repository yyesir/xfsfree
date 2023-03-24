#include "inisetting.h"

#include <QSettings>
#include <QString>

#define PISA_CONFIG_PATH "/etc/LFS/"

std::string inisetting::getSPIPath(const char* logical_services)
{
    //先得到spi文件名称
    QString strPath = QString("%1logical_services/%2.ini").arg(PISA_CONFIG_PATH).arg(logical_services);
    QString strBlock = "default/provider";
    QSettings setting(strPath, QSettings::IniFormat);
    QString strSpiFileName = setting.value(strBlock).toString();

    //在根据SPI配置文件路径得到spi库全路径
    strPath = QString("%1service_providers/%2.ini").arg(PISA_CONFIG_PATH).arg(strSpiFileName);
    strBlock = "default/lib_name";
    QSettings setting2(strPath, QSettings::IniFormat);
    return setting2.value(strBlock).toString().toStdString();
}
