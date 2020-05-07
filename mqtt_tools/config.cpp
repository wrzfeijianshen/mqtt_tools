#include "config.h"
#include <QtCore/QtCore>
#include <QDebug>

bool isFileExist(QString fullFileName)
{
    QFileInfo fileInfo(fullFileName);
    if(fileInfo.isFile())
        {
        return true;
        }
    return false;
}

Config::Config(QString qstrfilename):m_bFileExist(false)
{

    if (qstrfilename.isEmpty())
    {
        m_qstrFileName = QCoreApplication::applicationDirPath() + "/Config.ini";
    }
    else
    {
        m_qstrFileName = qstrfilename;
    }
    if(isFileExist(qstrfilename))
        m_bFileExist = true;
    m_psetting = new QSettings(m_qstrFileName, QSettings::IniFormat);
    qDebug() << m_qstrFileName;

}

Config::~Config()
{
    delete m_psetting;
    m_psetting = 0;
}

void Config::Set(QString qstrnodename,QString qstrkeyname,QVariant qvarvalue)
{
    m_psetting->setValue(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname), qvarvalue);
}

QVariant Config::Get(QString qstrnodename,QString qstrkeyname,QVariant qvarvalue)
{
    QVariant qvar = m_psetting->value(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname));
    if (qvar.isNull())
    {
        m_psetting->setValue(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname), qvarvalue);
        return qvarvalue;
    }
    return qvar;
}
