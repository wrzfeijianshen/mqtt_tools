#ifndef CMQTTCONFIG_H
#define CMQTTCONFIG_H

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
