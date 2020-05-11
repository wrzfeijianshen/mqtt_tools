#include "mqttConfig.h"

CMqttConfig::CMqttConfig():
    m_pConfig(nullptr)
{
    m_tBroker.addr = "broker.emqx.io";
    m_tBroker.port = 1883;
    m_tBroker.id = "emqx_test";
    m_tBroker.topic = "/topic/test/1";
    m_tBroker.user = "fjs1";
    m_tBroker.pwd = "123456";

}

CMqttConfig::~CMqttConfig()
{
    if(m_pConfig)
    {
        delete m_pConfig;
        m_pConfig = nullptr;
    }
}

void CMqttConfig::InitINIConfig()
{
    if(m_pConfig == nullptr)
        m_pConfig = new Config("Config.ini");

    // 如果配置文件不存在则初始化

    m_tBroker.addr =  m_pConfig->Get("broker","addr",m_tBroker.addr).toString();
    m_tBroker.port =   m_pConfig->Get("broker","port",m_tBroker.port).toInt();
    m_tBroker.id =  m_pConfig->Get("broker","id",m_tBroker.id).toString();
    m_tBroker.topic =   m_pConfig->Get("broker","topic",m_tBroker.topic).toString();
    m_tBroker.user =   m_pConfig->Get("broker","user",m_tBroker.user).toString();
    m_tBroker.pwd =  m_pConfig->Get("broker","pwd",m_tBroker.pwd).toString();
    m_tBroker.publish_topic =  m_pConfig->Get("broker","publish_topic",m_tBroker.publish_topic).toString();
}

void CMqttConfig::Save()
{
    m_pConfig->Set("broker","addr",m_tBroker.addr);
    m_pConfig->Set("broker","port",m_tBroker.port);
    m_pConfig->Set("broker","id",m_tBroker.id);
    m_pConfig->Set("broker","topic",m_tBroker.topic);
    m_pConfig->Set("broker","user",m_tBroker.user);
    m_pConfig->Set("broker","pwd",m_tBroker.pwd);
    m_pConfig->Set("broker","publish_topic",m_tBroker.publish_topic);
}


