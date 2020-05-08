#ifndef CONFIG_H
#define CONFIG_H
#if _MSC_VER >=1600 //VS2010版本号是1600，强制MSVC编译器采用UTF-8编码生成可执行文件
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
