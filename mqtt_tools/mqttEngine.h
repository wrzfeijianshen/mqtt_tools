#ifndef CMQTTENGINE_H
#define CMQTTENGINE_H
#include <mqtt/MQTTClient.h>
#include "mqttConfig.h"

class CMqttEngine
{
public:
    CMqttEngine();
    ~ CMqttEngine();
public:
    void connect();
    CMqttConfig* GetConfig(){return m_pConfig;}
 private:
   CMqttConfig* m_pConfig;
   MQTTClient m_Client;
};

#endif // CMQTTENGINE_H
