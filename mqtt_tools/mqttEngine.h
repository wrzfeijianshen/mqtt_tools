#ifndef CMQTTENGINE_H
#define CMQTTENGINE_H
#if _MSC_VER >=1600 //VS2010�汾����1600��ǿ��MSVC����������UTF-8�������ɿ�ִ���ļ�
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
    int Connect();// ����
    void Destroy();// ����
    int SetSubscribe(QString topic,int qos);// ����
    int SetUnSubscribe(QString topic);// ȡ������
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
 static CMqttEngine* m_selfEngine;// ����

 int PublishMessage(QString topic, int qos);

private:
    CMqttConfig* m_pConfig;
    MQTTClient m_Client;
    QVector<QString> m_vTopic;// ����

};


#endif // CMQTTENGINE_H
