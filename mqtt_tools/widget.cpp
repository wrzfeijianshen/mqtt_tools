#include "widget.h"
#include "ui_widget.h"
#include "mqttEngine.h"
#include <QDebug>
#include <QDateTime>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    m_pMqtt(nullptr),
    m_bConnect(false),
    m_bSubscribe(false)
{
    ui->setupUi(this);

    Init();
}

Widget::~Widget()
{
    delete ui;
}
void Widget::msgArrvd(CMqttMessage* mess)
{
    qDebug() << " recv : topic "<< mess->topic << ", message:" << mess->message << "len : " << mess->topicLen;

    // 获取收到的时间
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString str;
    QString current_date;
    current_date = current_date_time.toString("yyyy-MM-dd hh:mm::ss.zzz");
    str = current_date;
    str += "\t topic :[ ";

    str += mess->topic;
    str += " ] \n   message :[ ";
    str += mess->message;
    str += " ] \n";

    ui->plainTextEdit->appendPlainText(str);
}

void Widget::Init()
{
    if(m_pMqtt == nullptr)
    {
        m_pMqtt = new CMqttEngine();

        connect(m_pMqtt,SIGNAL(sig_msgArrvd(CMqttMessage*)),this,SLOT(msgArrvd(CMqttMessage*)));// 绑定
        m_Broker = m_pMqtt->GetConfig()->GetBroker();
        ui->addr_lineEdit->setText(m_Broker.addr);
        ui->port_lineEdit->setText(QString::number(m_Broker.port));
        ui->id_lineEdit->setText(m_Broker.id);
        ui->user_lineEdit->setText(m_Broker.user);
        ui->pwd_lineEdit->setText(m_Broker.pwd);
        ui->topic_lineEdit->setText(m_Broker.topic);
        ui->public_topic_lineEdit->setText(m_Broker.publish_topic);
    }
}

void Widget::on_pushButton_clicked()
{
    if(m_bConnect)
    {
        ui->pushButton->setText("连接");
        m_pMqtt->Destroy();
    }
    else
    {
        // 连接服务器
        m_pMqtt->GetConfig()->SetBroker(m_Broker);
        int ret =  m_pMqtt->Connect();
        if(ret != 0)
            qDebug() <<"m_pMqtt 连接失败 ";
        else
        {
            m_bConnect = true;
            ui->pushButton->setText("断开连接");
        }
    }

}


void Widget::on_addr_lineEdit_editingFinished()
{
    m_Broker.addr = ui->addr_lineEdit->text();
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}

void Widget::on_port_lineEdit_editingFinished()
{
    m_Broker.port = ui->port_lineEdit->text().toInt();
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}

void Widget::on_id_lineEdit_editingFinished()
{
    m_Broker.id = ui->id_lineEdit->text();
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}

void Widget::on_user_lineEdit_editingFinished()
{
    m_Broker.user = ui->user_lineEdit->text();
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}

void Widget::on_pwd_lineEdit_editingFinished()
{
    m_Broker.pwd = ui->pwd_lineEdit->text();
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}

void Widget::on_topic_lineEdit_editingFinished()
{
    m_Broker.topic = ui->topic_lineEdit->text();
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}

void Widget::on_public_topic_lineEdit_editingFinished()
{
    m_Broker.publish_topic = ui->public_topic_lineEdit->text();
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}

void Widget::on_pushButton_2_clicked()
{
    int ret = 0;
    // 订阅主题和取消订阅
    if(m_bSubscribe)
    {
        m_bSubscribe = false;
        ret = m_pMqtt->SetUnSubscribe(m_Broker.topic);
        if(ret != 0)
        {
            qDebug() << "订阅不存在 " << ret;
        }

        ui->pushButton_2->setText("订阅");
    }
    else
    {
        int index =  ui->comboBox->currentIndex();
        qDebug() << "index : "<<index;
        // 连接服务器
        ret = m_pMqtt->SetSubscribe(m_Broker.topic,index);
        if(ret != 0)
        {
            qDebug() <<"m_pMqtt 连接失败 ";
            ui->pushButton_2->setText("订阅");
        }
        else
        {
            m_bSubscribe = true;
            ui->pushButton_2->setText("取消订阅");
        }
    }
}

void Widget::on_pushButton_6_clicked()
{
    // 发布主题
   int ret = 0;
    QString str = ui->plainTextEdit_2->toPlainText();
   // 连接服务器
   ret = m_pMqtt->PublishMessage(str,0);
   if(ret != 0)
   {
       qDebug() <<"m_pMqtt 连接失败 ";
   }
   else
   {
       m_bSubscribe = true;
   }
}
