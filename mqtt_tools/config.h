#ifndef CONFIG_H
#define CONFIG_H

#include <QVariant>
#include <QSettings>

class Config
{
public:
    Config(QString qstrfilename = "");
    virtual ~Config(void);
    bool IsFileExist(){return m_bFileExist;}
    QVariant Get(QString qstrnodename, QString qstrkeyname, QVariant qvarvalue);
    void Set(QString qstrnodename, QString qstrkeyname, QVariant qvarvalue);
private:
    QString m_qstrFileName;
    QSettings *m_psetting;
    bool m_bFileExist;
};


#endif // CONFIG_H
