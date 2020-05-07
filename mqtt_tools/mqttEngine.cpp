#include "mqttEngine.h"
#include <iostream>
using namespace std;

#define QOS         1
#define TIMEOUT     10000L


CMqttEngine::CMqttEngine():
    m_pConfig(nullptr)
{
    m_pConfig = new CMqttConfig();
    m_pConfig->InitINIConfig();
}

CMqttEngine::~CMqttEngine()
{

}

void CMqttEngine::connect()
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
    MQTTBroker broker = m_pConfig->GetBroker();
    if ((rc = MQTTClient_create(&m_Client, broker.addr.toStdString().c_str(), broker.id.toStdString().c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(m_Client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    char* plyload = "hello world";

    pubmsg.payload = plyload;
    pubmsg.payloadlen = (int)strlen(plyload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    if ((rc = MQTTClient_publishMessage(m_Client, broker.topic.toStdString().c_str(), &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to publish message, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for up to %d seconds for publication of %s\n"
           "on topic %s for client with ClientID: %s\n",
           (int)(TIMEOUT/1000), plyload, broker.topic.toStdString().c_str(), broker.id.toStdString());
    rc = MQTTClient_waitForCompletion(m_Client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);

    if ((rc = MQTTClient_disconnect(m_Client, 10000)) != MQTTCLIENT_SUCCESS)
        printf("Failed to disconnect, return code %d\n", rc);
    MQTTClient_destroy(&m_Client);
}

