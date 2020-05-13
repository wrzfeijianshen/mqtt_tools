#include "widget.h"
#include "ui_widget.h"
#include "mqttEngine.h"
#include <QDebug>
#include <QDateTime>
#include "base64.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QThread>
#include <List>
#include <string>
#include <QFile>
#include <QFileDialog>
#include "mqtt/pubsub_opts.h"

QString GetFileSize(const qint64 &size)
{
    int integer = 0;  //����λ
    int decimal = 0;  //С��λ��������λ
    char unit ='B';
    qint64 standardSize = size;
    qint64 curSize = size;

    if(standardSize > 1024) {
        curSize = standardSize * 1000;
        curSize /= 1024;
        integer = curSize / 1000;
        decimal = curSize % 1000;
        standardSize /= 1024;
        unit = 'K';
        if(standardSize > 1024) {
            curSize = standardSize * 1000;
            curSize /= 1024;
            integer = curSize / 1000;
            decimal = curSize % 1000;
            standardSize /= 1024;
            unit = 'M';
            if(standardSize > 1024) {
                curSize = standardSize * 1000;
                curSize /= 1024;
                integer = curSize / 1000;
                decimal = curSize % 1000;
                unit = 'G';
            }
        }
    }

    QString dec = "0";
    if (0 <= decimal && decimal <= 9) {
        dec = dec + dec + QString::number(decimal);
    }

    if (10 <= decimal && decimal <= 99) {
        dec = "0" + QString::number(decimal);
    }

    if (100 <= decimal && decimal <= 999) {
        dec = QString::number(decimal);
    }

    return QString::number(integer) + "." + dec + unit;
}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    m_pMqtt(nullptr),
    m_bConnect(false)
{
    ui->setupUi(this);

    m_appFileName = QCoreApplication::applicationDirPath();
    Init();
}

Widget::~Widget()
{
    delete ui;
}

//QStringList g_aList;
 QList<QString> g_aList;
void Widget::msgArrvd(CMqttMessage* mess)
{
    // ��ȡ�յ���ʱ��
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString str;
    QString current_date;
    current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss");

    QString strOutPath = m_appFileName+ "/fileCache/";
    QDir dir(strOutPath);
    if(!dir.exists()){
        bool ismkdir = dir.mkdir(strOutPath);
        if(!ismkdir)
            qDebug() << "Create path fail" << endl;
        else
            qDebug() << "Create fullpath success" << endl;
    }

    // ����json
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(mess->message.toUtf8(), &error);
    if(QJsonParseError::NoError == error.error)
    {
        qDebug() << " recv : topic 1111111111" ;
        qDebug() << " recv : topic "<< mess->topic << ", message:" << mess->message << "len : " << mess->topicLen;

        //map
        QVariantMap map = document.toVariant().toMap();
        if(map.contains("type"))
        {
            int type = map["type"].toInt();
            qDebug() << type;
        }
          QString filename;
        if(map.contains("fileName"))
        {
           filename = map["fileName"].toString();
            qDebug() << filename;
        }

        if(map.contains("data"))
        {
            QString file1 = map["data"].toString();
//            g_aList.append(file);
            qDebug() << file1;
             QByteArray arr;
             arr.append(Base64::decode(file1));
             QFile file(strOutPath + filename);//
             bool isOk = file.open(QFile::Append);
             if (!isOk) {
                 qDebug() <<"file : no ";
                 return;
             }
             file.write(arr);
             file.close();
//             g_aList.clear();
        }

        //        //����
        //        QList<QVariant> list = document.toVariant.toList();
        //        foreach(QVariant item, list)
        //        {
        //            QVariantMap map = item.toMap();
        //            QString addr = map["address"].toString();
        //        }
    }
    else
    {
        // �ַ���
        qDebug() << " recv : topic "<< mess->topic << ", message:" << mess->message << "len : " << mess->topicLen;


        str = current_date;
        str += "\n  topic :[ ";

        str += mess->topic;
        str += " ] \n  Qos : [ ";
        str += QString::number(mess->qos);
        str +=   " ]\n  message :[ ";
        str += mess->message;
        str += " ] \n";
        ui->plainTextEdit->appendPlainText(str);
    }

    delete mess;
}

void Widget::msgConnLost()
{
    SetStatText("�����ѶϿ�","red");
    ui->pushButton->setText("����");
    m_pMqtt->Destroy();
}

void Widget::Init()
{
    if(m_pMqtt == nullptr)
    {
        m_pMqtt = new CMqttEngine();

        connect(m_pMqtt,SIGNAL(sig_msgArrvd(CMqttMessage*)),this,SLOT(msgArrvd(CMqttMessage*)));// ��
        connect(m_pMqtt,SIGNAL(sig_msgConnLost()),this,SLOT(msgConnLost()));


        m_Broker = m_pMqtt->GetConfig()->GetBroker();
        ui->addr_lineEdit->setText(m_Broker.addr);
        ui->port_lineEdit->setText(QString::number(m_Broker.port));
        ui->id_lineEdit->setText(m_Broker.id);
        ui->user_lineEdit->setText(m_Broker.user);
        ui->pwd_lineEdit->setText(m_Broker.pwd);
        ui->topic_lineEdit->setText(m_Broker.topic);
        ui->public_topic_lineEdit->setText(m_Broker.publish_topic);
        ui->comboBox_6->setCurrentIndex(m_Broker.appMode);
    }
}

void Widget::SetStatText(QString str,QString color)
{
    ui->label_stat->setText( str);
    QString s = "color:" + color+";";
    ui->label_stat->setStyleSheet(s);

}

void Widget::on_pushButton_clicked()
{
    if(m_bConnect)
    {
        ui->pushButton->setText("����");
        m_pMqtt->Destroy();
        SetStatText("�ѶϿ�������");
    }
    else
    {
        // ���ӷ�����
        m_pMqtt->GetConfig()->SetBroker(m_Broker);
        int ret =  m_pMqtt->Connect();
        if(ret != 0)
        {
            qDebug() <<"m_pMqtt ����ʧ�� ";
            SetStatText("����ʧ��,״̬��:[" +QString::number(ret)+"]","red");
        }
        else
        {
            m_bConnect = true;
            ui->pushButton->setText("�Ͽ�����");
            SetStatText("�����ӵ�������");
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
    int index =  ui->comboBox->currentIndex();
    qDebug() << "index : "<<index;
    // ���ӷ�����
    int ret = m_pMqtt->SetSubscribe(m_Broker.topic,index);
    if(ret != 0)
    {
        qDebug() <<"m_pMqtt ����ʧ�� ";
        ui->pushButton_2->setText("����");
    }
}

void Widget::on_pushButton_6_clicked()
{
    // ��������
    int ret = 0;
    QString str = ui->plainTextEdit_2->toPlainText();
    int index =  ui->comboBox_3->currentIndex();
    // ���ӷ�����
    ret = m_pMqtt->PublishMessage(m_Broker.publish_topic,str,index);
    if(ret != 0)
    {
        qDebug() <<"m_pMqtt ����ʧ�� ";
        return;
    }
    return ;
    qDebug() << "�������";
}

void Widget::on_pushButton_3_clicked()
{
    // ���
    ui->plainTextEdit->clear();
}

void Widget::on_pushButton_5_clicked()
{
    // ���
    ui->plainTextEdit_2->clear();
}

void Widget::on_pushButton_4_clicked()
{
    // ȡ������
    int ret = m_pMqtt->SetUnSubscribe(m_Broker.topic);
    if(ret != 0)
    {
        qDebug() << "���Ĳ����� " << ret;
    }

}

enum scmdtype{
    CMD_S2C_CONNECT = 0x10,    //�����յ�����tcp��Ϣ����client������������
    CMD_S2C_DOWNFILE,          //
};

void Widget::on_pushButton_7_clicked()
{
    if (m_Broker.appMode == 0)
    {
    // file --> base64
    // Config.ini

    // �ݲ��� С�ڿ���1M�Ŀ��Է������ 917.119K,������ͳ���1M�ֽڴ�С����ɷ���ʧ��,�Ͽ�����
    // ����̶���С����500k���ݶ������ļ�,
    // 100M �������1����.
    // ���ض˺ͽ��ն���߶��߷���,��Ȼ����ȡ���ڴ�,�ܲ���.
    if(m_SendfileName == "")
    {
        SetStatText("�ļ�Ϊ��,��ѡ���ļ�","red");
        return;
    }
    // ���Զ���,Ӧ�û�ҵ�
    int sNum = 1000;
    while(sNum--)
    {
    QFileInfo fileinfo;
    fileinfo = QFileInfo(m_SendfileName);
    QString file_name = fileinfo.fileName();

    QFile file(m_SendfileName);//
    bool isOk = file.open(QFile::ReadOnly);
    if (!isOk) {
        SetStatText("�ļ�Ϊ��,��ѡ���ļ�","red");
        qDebug() <<"file : no ";
        return;
    }




#if 1
    QString pubTopoc = ui->public_topic_lineEdit_2->text();
    qDebug() << pubTopoc;
    int index =  ui->comboBox_5->currentIndex();

    // ���ļ�
    int len = 0;
    int num = 0;
    do
    {
        QByteArray array = file.read(1024*500);

        len =array.size();
        if(len == 0)
        {
            qDebug() << "file : size 0";
            break;
        }
        num++;
        qDebug() <<"file : size [ " << GetFileSize(len) << num;
        QString str = Base64::encode(array);

//        qDebug() <<"Base64 str size: [ "  << GetFileSize(x.size());
        QJsonObject json{
            { "type", CMD_S2C_DOWNFILE },
            { "fileName", file_name +QString::number(sNum)},
            { "fileName1", QString::number(num)},

            { "data", str}
        };
        num++;

        QJsonDocument document;
        document.setObject(json);
        QByteArray barr = document.toJson(QJsonDocument::Compact);

        qDebug() <<"barr size: [ "  << GetFileSize(barr.size());
        QThread::msleep(30);
        int ret = m_pMqtt->PublishJsonMessage(pubTopoc,barr.data(),index);
        if(ret != 0)
        {
            qDebug() <<"m_pMqtt push " << ret;
             return;
        }

//        QString * ss =  new QString(str);
//        sList.push_back(ss);
    }
    while(1);
    SetStatText("�����ļ��ɹ�");
     qDebug() <<"m_pMqtt push 1111 : "  << sNum;

    }
     m_SendfileName ="";

#endif
#if 0
    QByteArray array = file.readAll();
    QString str =  Base64::encode(array);
    file.close();

    //        array.toBase64();


    //    qDebug() <<"str" << str;

    //    QByteArray arr =  Base64::decode(str);


    QJsonObject json{
        { "type", CMD_S2C_DOWNFILE },
        { "fileName", "conf_zip222.zip1"},
        { "fileName1", QString::number(num)},

        { "data", str}
    };


    QJsonDocument document;
    document.setObject(json);
    QByteArray barr = document.toJson(QJsonDocument::Compact);

    qDebug() <<"barr size: [ "  << GetFileSize(barr.size()) << barr.size();

    int ret = m_pMqtt->PublishJsonMessage(m_Broker.publish_topic,barr.data(),0);
    if(ret != 0)
    {
        qDebug() <<"m_pMqtt push " << ret;

    }
#endif
    }
    else if (m_Broker.appMode == 1)
    {
        m_pMqtt->PublishSendMessage(m_Broker.publish_topic,m_SendfileName,0);
    }
}

void Widget::on_pushButton_9_clicked()
{

}

void Widget::on_pushButton_8_clicked()
{
    // ѡ�����ļ�
   m_SendfileName = QFileDialog::getOpenFileName(
                  this, tr("open file"),
                  "./", tr("files All files (*.*)"));

  if(m_SendfileName.isEmpty())
  {
       SetStatText("���ļ�ʧ��","red");
      return;
  }

}

void Widget::on_comboBox_6_currentIndexChanged(const QString &arg1)
{
    int index =  ui->comboBox_6->currentIndex();
    m_Broker.appMode = index;
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}
