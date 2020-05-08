#ifndef CMQTTCONFIG_H
#define CMQTTCONFIG_H
#if _MSC_VER >=1600 //VS2010版本号是1600，强制MSVC编译器采用UTF-8编码生成可执行文件
#pragma execution_character_set("utf-8")
#endif
#include "config.h"

typedef struct tMQTTBroker
{
    QString addr;
    int port;
    QString id;
    QString user;
    QString pwd;
    QString topic;
    QString publish_topic;
}MQTTBroker;

class CMqttConfig
{
public:
    CMqttConfig();
    ~CMqttConfig();
public:
    void InitINIConfig();
    MQTTBroker GetBroker(){return m_tBroker;}
    void SetBroker(MQTTBroker broker){ m_tBroker = broker; Save();}
    void Save();
private:
    Config* m_pConfig;
    MQTTBroker m_tBroker;
};

#endif // CMQTTCONFIG_H
