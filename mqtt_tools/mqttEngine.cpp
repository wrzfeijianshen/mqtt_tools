#include "mqttEngine.h"
#include <iostream>
#include <QDebug>
#include <functional>
using namespace std;

#define TIMEOUT     10000L

 volatile MQTTClient_deliveryToken CMqttEngine::deliveredtoken = 0;
CMqttEngine::CMqttEngine():
    m_pConfig(nullptr)
{
    m_pConfig = new CMqttConfig();
    m_pConfig->InitINIConfig();
}

CMqttEngine::~CMqttEngine()
{

}

void CMqttEngine::Delivered(void *context, MQTTClient_deliveryToken dt)
{
    qDebug() << "Message with token value  delivery confirmed" <<  dt;
    deliveredtoken = dt;
}


int CMqttEngine::MsgArrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    qDebug() << "Message arrived";
     qDebug() << "     topic: "<<  topicName;
    qDebug() << "   message: " <<  message->payloadlen <<  (char *)message->payload << message->dup <<message->msgid;
    qDebug() << message->properties.length <<message->qos << message->retained << message->struct_id << message->struct_version;

    qDebug() << (char *)message->properties.array;
    qDebug() << message->properties.count;


    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
//    QString a= "111111111111111111111111";
//    emit this->message2(a);
    return 1;
}

void CMqttEngine::ConnLost(void *context, char *cause)
{
    qDebug() << "\nConnection lost";
  qDebug() << "     cause: "<<cause;
}

int CMqttEngine::Connect()
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    MQTTBroker broker = m_pConfig->GetBroker();

    QString addr = "tcp://" + broker.addr+":"+ QString::number(broker.port);

    qDebug() << broker.addr << broker.id << broker.port << broker.topic << addr;
    if ((rc = MQTTClient_create(&m_Client, addr.toStdString().c_str(), broker.id.toStdString().c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        qDebug() << "Failed to create client, return code " <<  rc;
        return rc;
    }

    if ((rc = MQTTClient_setCallbacks(m_Client, this, &CMqttEngine::ConnLost, &CMqttEngine::MsgArrvd, &CMqttEngine::Delivered)) != MQTTCLIENT_SUCCESS)
    {
          qDebug() << "Failed to set callbacks, return code" <<  rc ;
          return rc;
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(m_Client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        qDebug() << "Failed to connect, return code "<< rc;
        return rc;
    }
    return rc;
}

void CMqttEngine::Destroy()
{
    int rc;
    if ((rc = MQTTClient_disconnect(m_Client, 10000)) != MQTTCLIENT_SUCCESS)
        qDebug() << "Failed to disconnect, return code " <<  rc;

    MQTTClient_destroy(&m_Client);
}


int CMqttEngine::SetSubscribe(QString topic,int qos)
{
    int rc = 0;

    if ((rc = MQTTClient_subscribe(m_Client, topic.toStdString().c_str(), qos)) != MQTTCLIENT_SUCCESS)
    {
        qDebug() << "Failed to subscribe, return code " <<  rc;
    }

    return rc;
}


int CMqttEngine::SetUnSubscribe(QString topic)
{
    int rc = 0;
    if ((rc = MQTTClient_unsubscribe(m_Client, topic.toStdString().c_str())) != MQTTCLIENT_SUCCESS)
    {
        qDebug() << "Failed to unsubscribe, return code " << rc;
    }

    return rc;
}
