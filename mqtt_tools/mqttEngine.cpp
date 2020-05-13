#include "mqttEngine.h"
#include <iostream>
#include <QDebug>
#include <functional>
#include <QTextCodec>
#include <string>
#include <QByteArray>
#include <QTextCodec>

using namespace std;

#define TIMEOUT     10000L
CMqttEngine* CMqttEngine::m_selfEngine = nullptr;
volatile MQTTClient_deliveryToken CMqttEngine::deliveredtoken = 0;


#ifdef _WIN32
#include <windows.h>

string GbkToUtf8(const char *src_str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    string strTemp = str;
    if (wstr) delete[] wstr;
    if (str) delete[] str;
    return strTemp;
}

string Utf8ToGbk(const char *src_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    string strTemp(szGBK);
    if (wszGBK) delete[] wszGBK;
    if (szGBK) delete[] szGBK;
    return strTemp;
}
#else
#include <iconv.h>

int GbkToUtf8(char *str_str, size_t src_len, char *dst_str, size_t dst_len)
{
    iconv_t cd;
    char **pin = &str_str;
    char **pout = &dst_str;

    cd = iconv_open("utf8", "gbk");
    if (cd == 0)
        return -1;
    memset(dst_str, 0, dst_len);
    if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}

int Utf8ToGbk(char *src_str, size_t src_len, char *dst_str, size_t dst_len)
{
    iconv_t cd;
    char **pin = &src_str;
    char **pout = &dst_str;

    cd = iconv_open("gbk", "utf8");
    if (cd == 0)
        return -1;
    memset(dst_str, 0, dst_len);
    if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}


#endif

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
//    qDebug() << " MsgArrvd qos" << QString::number( message->qos) <<",message " << (char *)message->payload<< ",topic " <<topicName <<",topicLen " << QString::number(topicLen)
//             << ",dup " <<  QString::number( message->dup) ;
    char* pTopic;
    pTopic = (char *)message->payload;
    QByteArray bPtopic = QByteArray(pTopic, message->payloadlen);

    CMqttMessage *mess = new CMqttMessage();
    mess->qos = message->qos;
    mess->message = QString(bPtopic);
    mess->topic =  topicName;
    mess->topicLen = topicLen;
    mess->dup =  message->dup;

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    emit GetInstance()->sig_msgArrvd(mess);
    return 1;
}

void CMqttEngine::ConnLost(void *context, char *cause)
{
    qDebug() << "\nConnection lost";
    qDebug() << "     cause: "<<cause;
    emit GetInstance()->sig_msgConnLost();
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


int CMqttEngine::PublishMessage(QString pubTopic,QString topic,int qos)
{
    qDebug() << "pubTopic " << pubTopic << topic << qos;
    int rc = 0;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    QTextCodec*t = QTextCodec::codecForName("Utf8");

    QString str=t->toUnicode(topic.toUtf8());

    string s1 = str.toStdString();

    pubmsg.payload = (void *)s1.c_str();

    pubmsg.payloadlen = s1.length();


    pubmsg.qos = qos;
    pubmsg.retained = 1;

    if ((rc = MQTTClient_publishMessage(m_Client, pubTopic.toStdString().c_str(), &pubmsg,  &token)) != MQTTCLIENT_SUCCESS)
    {
        qDebug() << "Failed to subscribe, return code " <<  rc;
    }

    rc = MQTTClient_waitForCompletion(m_Client, token, TIMEOUT);

    return rc;
}


int CMqttEngine::PublishJsonMessage(QString pubTopic,char *msg,int qos)
{
    qDebug() << "pubTopic " << pubTopic << qos;
    int rc = 0;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    pubmsg.payload = msg;

    pubmsg.payloadlen = strlen(msg);
    qDebug() <<"strlen " << strlen(msg) << pubmsg.payloadlen;

    pubmsg.qos = qos;
    pubmsg.retained = 0;



    if ((rc = MQTTClient_publishMessage(m_Client, pubTopic.toStdString().c_str(), &pubmsg,  &token)) != MQTTCLIENT_SUCCESS)
    {
        qDebug() << "Failed to subscribe, return code " <<  rc;
    }

    rc = MQTTClient_waitForCompletion(m_Client, token, TIMEOUT);

    return rc;
}

