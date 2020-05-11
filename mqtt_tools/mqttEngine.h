#ifndef CMQTTENGINE_H
#define CMQTTENGINE_H
#if _MSC_VER >=1600 //VS2010版本号是1600，强制MSVC编译器采用UTF-8编码生成可执行文件
#pragma execution_character_set("utf-8")
#endif
#include <QObject>

#include <mqtt/MQTTClient.h>
#include "mqttConfig.h"
#include <QVector>

typedef struct _tCMqttMessage
{
    QString topic;
    int topicLen;
    QString message;
    int qos;
    int udp;

}CMqttMessage;

class CMqttEngine: public QObject
{
        Q_OBJECT

public:
    CMqttEngine();
    ~ CMqttEngine();
public:

    CMqttConfig* GetConfig(){return m_pConfig;}
    int Connect();// 连接
    void Destroy();// 销毁
    int SetSubscribe(QString topic,int qos);// 订阅
    int SetUnSubscribe(QString topic);// 取消订阅
    static void ConnLost(void *context, char *cause);
    static void Delivered(void *context, MQTTClient_deliveryToken dt);
    static int MsgArrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);

    static CMqttEngine* GetInstance()
    {
        return m_selfEngine;
    }
signals:
     void sig_msgArrvd(CMqttMessage* message);

public:
 static volatile MQTTClient_deliveryToken deliveredtoken;
 static CMqttEngine* m_selfEngine;// 自身

 int PublishMessage(QString topic, int qos);

private:
    CMqttConfig* m_pConfig;
    MQTTClient m_Client;
    QVector<QString> m_vTopic;// 主题

};


#endif // CMQTTENGINE_H
