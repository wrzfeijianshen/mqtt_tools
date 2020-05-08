#ifndef CONFIG_H
#define CONFIG_H
#if _MSC_VER >=1600 //VS2010�汾����1600��ǿ��MSVC����������UTF-8�������ɿ�ִ���ļ�
#pragma execution_character_set("utf-8")
#endif
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
