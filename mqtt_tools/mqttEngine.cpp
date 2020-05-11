#include "mqttEngine.h"
#include <iostream>
#include <QDebug>
#include <functional>
#include <QTextCodec>


using namespace std;

#define TIMEOUT     10000L
CMqttEngine* CMqttEngine::m_selfEngine = nullptr;
volatile MQTTClient_deliveryToken CMqttEngine::deliveredtoken = 0;


QString GBK2UTF8(const QString &str)
{
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    return utf8->toUnicode(str.toUtf8());
}

QString UTF82GBK(const QString &str)
{
    QTextCodec *gbk = QTextCodec::codecForName("GB18030");
    return gbk->toUnicode(str.toLocal8Bit());
}

std::string GBK2UTF8(std::string &str)
{
    QString temp = QString::fromLocal8Bit(str.c_str());
    std::string ret = temp.toUtf8().data();
    return ret;
}

std::string UTF82GBK(std::string &str)
{
    QString temp = QString::fromUtf8(str.c_str());
    std::string ret = temp.toLocal8Bit().data();
    return ret;
}



CMqttEngine::CMqttEngine():
    m_pConfig(nullptr)
{
    m_selfEngine = this;
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
    CMqttMessage *mess = new CMqttMessage();
    mess->qos = message->qos;
    mess->message = (char *)message->payload;
    mess->topic =  topicName;
    mess->topicLen = topicLen;
    mess->udp =  message->dup;

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    emit GetInstance()->sig_msgArrvd(mess);
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

    //typedef void MQTTClient_connectionLost(void* context, char* cause);

    if ((rc = MQTTClient_setCallbacks(m_Client, NULL, &CMqttEngine::ConnLost, CMqttEngine::MsgArrvd, CMqttEngine::Delivered)) != MQTTCLIENT_SUCCESS)
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
        return rc;
    }

    m_vTopic.append(topic);
    return rc;
}


int CMqttEngine::SetUnSubscribe(QString topic)
{
    int rc = 0;
    if ((rc = MQTTClient_unsubscribe(m_Client, topic.toStdString().c_str())) != MQTTCLIENT_SUCCESS)
    {
        qDebug() << "Failed to unsubscribe, return code " << rc;
        return rc;
    }
    auto pos = find(m_vTopic.begin(),m_vTopic.end(),topic);
    if (pos != m_vTopic.end())
    {
        m_vTopic.erase(pos);
    }

    return rc;
}


int CMqttEngine::PublishMessage(QString topic,int qos)
{
    qDebug() <<"PublishMessage" <<topic;
    QString qstr = UTF82GBK(topic);// ×ª³Égbk

    int rc = 0;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = (void *)qstr.toStdString().c_str();

    qDebug() <<"PublishMessage" <<(char* )pubmsg.payload;

    pubmsg.payloadlen = qstr.length();
    qDebug() <<"payloadlen " <<pubmsg.payloadlen;


    pubmsg.qos = qos;
    pubmsg.retained = 0;

    int len = m_vTopic.size();

    for(auto sTopic : m_vTopic)
    {
        if ((rc = MQTTClient_publishMessage(m_Client, sTopic.toStdString().c_str(), &pubmsg,  &token)) != MQTTCLIENT_SUCCESS)
        {
            qDebug() << "Failed to subscribe, return code " <<  rc;
            continue;
        }

        rc = MQTTClient_waitForCompletion(m_Client, token, TIMEOUT);

    }

    return rc;
}
