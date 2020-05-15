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
#include <QJsonArray>
#include <QUuid>

enum scmdtype{
    CMD_S2C_CONNECT = 0x10,    //�����յ�����tcp��Ϣ����client������������
    CMD_S2C_DOWNFILE,          //
};


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
    m_bConnect(false),
    m_iFileShardSize(1024*500)
{
    ui->setupUi(this);

    m_appFileName = QCoreApplication::applicationDirPath();
    Init();
    connect(this,SIGNAL(sig_RecvAddText(QString)),this,SLOT(RecvAddText(QString)));

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
        qDebug() << " recv : topic "<< mess->topic << ", message:" << mess->message << "len : " << mess->topicLen;

        //map
        QVariantMap map = document.toVariant().toMap();

        int type = -1;
        if(map.contains("type"))
        {
            type = map["type"].toInt();
            qDebug() << type;
        }
        switch (type)
        {
        case -1:
        {
            // �����Ϣ
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
            break;
        case (int)CMD_S2C_DOWNFILE:
        {

            QJsonObject o2(document.object());

            QJsonArray databyte2 = o2.value("data").toArray();
            QJsonValue databyte =  databyte2.first();

            if (!databyte.isObject())
            {
                break;
            }
            QJsonObject data(databyte.toObject());

            QString fileName = data.take("fileName").toString();

            QString fileId = data.take("fileID").toString();
            int fileCount2 = data.take("fileCount").toInt();

            int fileIndex2 = data.take("fileIndex").toInt();
            //            auto fileSize2 = data.take("fileSize").toVariant();

            qDebug() << "fileId" << fileId<<"fileCount2 " << fileCount2;
            //            << fileIndex2 <<fileSize2 << "fileSize2";

            QString file1 = data.take("fileData").toString();
            QByteArray arr;
            arr.append(Base64::decode(file1));
            dir.mkdir(strOutPath +fileId);

            QFile file(strOutPath +fileId +"/"+ fileName);
            bool isOk = file.open(QFile::Append);
            if (!isOk) {
                qDebug() <<"file : no ";
                return;
            }
            file.write(arr);
            file.close();
            ui->plainTextEdit_3->appendPlainText(fileId +"/"+ fileName +",���ս���Ϊ "  + QString::number(fileIndex2) +'/' +QString::number(fileCount2) );

            if(fileCount2 == fileIndex2)
            {
                ui->plainTextEdit_3->appendPlainText(fileId +"/"+ fileName +",���ճɹ�");
                qDebug() << "���ճɹ�";
            }

            break;
        }
        default:
        {
            QString data;
            if(map.contains("data"))
            {
                data = map["data"].toString();
            }

            str = current_date;
            str += "\n  topic :[ ";

            str += mess->topic;
            str += " ] \n  Qos : [ ";
            str += QString::number(mess->qos);
            str +=   " ]\n  message data:[ ";
            str += data;
            str += " ] \n";
            ui->plainTextEdit->appendPlainText(str);
        }

        }


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
        m_bConnect = false;
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
    if(m_pMqtt == nullptr)
        return;
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
    int check = ui->checkBox->isChecked()== true ?1:0;
    // �ж��Ƿ�json������.

    // ���ӷ�����
    ret = m_pMqtt->PublishMessage(m_Broker.publish_topic,str,index,check);
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

void Widget::RecvAddText(QString str)
{
        ui->plainTextEdit_3->appendPlainText(str);
}

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

        // �²�: �������������,ͬ���ͻ���,���ܻᵼ�����������ͻ��߽���,��ָ��ʱ����,�Ͽ��������������,�����ļ�����ʧ��.
        // conn_opts.keepAliveInterval = 0; �򲻷���������,����������Ļ�,�ļ�����Ӧ��û��ʲô����.������;�Ͽ�����.

        if(m_SendfileName == "")
        {
            SetStatText("�ļ�Ϊ��,��ѡ���ļ�","red");
            return;
        }
        QFileInfo fileinfo;
        fileinfo = QFileInfo(m_SendfileName);
        qint64 size = fileinfo.size();

        int count = size /(1024*500);
        int count1 = size %(1024*500);
        if(count1 > 0)
            count++;
        qDebug() <<"�ļ���С size : " <<size <<"���ʹ��� "<< count;

        QDateTime time = QDateTime::currentDateTime();   //��ȡ��ǰʱ��
        int timeT = time.toTime_t();
        qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
        int irand =  qrand() % (99999 - 10000) + 10000;

        QString fileId = QString::number(timeT)+'_'+QString::number(irand);


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
        int check = ui->checkBox->isChecked()== true ?1:0;

        // ���ļ�
        int len = 0;
        int num = 0;
        do
        {
            // ��Ƭ
            QByteArray array = file.read(1024*500);

            len =array.size();
            if(len == 0)
            {
                qDebug() << "file : size 0";
                if(count == num)
                   emit RecvAddText(fileId +"/"+ file_name +",������� "  + QString::number(num) +'/' +QString::number(count) );
                break;
            }
            num++;
            qDebug() <<"file : size [ " << GetFileSize(len) << "���ʹ��� " << num;
            QString str = Base64::encode(array);

            QJsonArray arr;

            QJsonObject o
            {
                { "fileID", fileId},
                { "fileName", file_name},//���͵��ļ���
                { "fileSize", size},// �ļ���С
                { "fileCount", count},// ��ǰ�Ĵ�����ܴ���
                { "fileIndex", num},// ��ǰ����ĵڼ���
                { "fileData", str}

            };
            QJsonValue v(o);
            arr.append(v);

            QJsonObject json
            {
                { "type", CMD_S2C_DOWNFILE },
                { "data", arr }
            };

            QJsonDocument document;
            document.setObject(json);
            QByteArray barr = document.toJson(QJsonDocument::Compact);

            qDebug() <<"barr size: [ "  << GetFileSize(barr.size());
            QThread::msleep(10);// ��Ϣһ��ʱ��
            int ret = m_pMqtt->PublishJsonMessage(pubTopoc,barr.data(),index,check);
            if(ret != 0)
            {
                qDebug() <<"m_pMqtt push " << ret;
                return;
            }
            emit RecvAddText(fileId +"/"+ file_name +",�ѷ��� "  + QString::number(num) +'/' +QString::number(count) );// �����ݶ�
        }
        while(1);

        SetStatText("�����ļ��ɹ�");
        qDebug() <<"m_pMqtt push �����ļ��ɹ� : ";

        m_SendfileName ="";

#endif
    }
    else if (m_Broker.appMode == 1)
    {
        int index =  ui->comboBox_5->currentIndex();
        int check = ui->checkBox->isChecked()== true ?1:0;
        m_pMqtt->PublishSendMessage(m_Broker.publish_topic,m_SendfileName,index,check);
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
Q_UNUSED(arg1);
    int index =  ui->comboBox_6->currentIndex();
    m_Broker.appMode = index;
    m_pMqtt->GetConfig()->SetBroker(m_Broker);
}

void Widget::on_comboBox_2_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    // �´��յ����ת��
}

void Widget::on_comboBox_4_currentIndexChanged(int index)
{
    // ѡ��json��,�������
    if ( index == 1)
    {
        QJsonObject json
        {
            { "type", 1 },
            { "data", "�����ټ�"}
        };
        QJsonDocument document;
        document.setObject(json);
        ui->plainTextEdit_2->clear();
        ui->plainTextEdit_2->appendPlainText(document.toJson());
    }
}

void Widget::on_pushButton_10_clicked()
{
    // һ�� Topic ֻ���� 1 �� Retained ��Ϣ�������µ� Retained ��Ϣ�������ϵ� Retained ��Ϣ
    // Retained ��Ϣ�� Broker Ϊÿһ�� Topic �����洢�ģ����־��ԻỰ�� Broker Ϊÿһ�� Client �����洢�ġ�
    // ɾ��һ�� Retained ��ϢҲ�ܼ� : ��������ⷢ��һ�� Payload ����Ϊ 0 �� Retained ��Ϣ�Ϳ�����
    // ���ӷ�����
    int index =  ui->comboBox_3->currentIndex();
    int ret = m_pMqtt->PublishMessage(m_Broker.publish_topic,"",index,1);
    if(ret != 0)
    {
        qDebug() <<"m_pMqtt ����ʧ�� ";
        return;
    }
    return ;
    qDebug() << "�������";
}
QString getRandomString(int length)
{
    qsrand(QDateTime::currentMSecsSinceEpoch());//Ϊ���ֵ�趨һ��seed

    const char chrs[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int chrs_size = sizeof(chrs);

    char* ch = new char[length + 1];
    memset(ch, 0, length + 1);
    int randomx = 0;
    for (int i = 0; i < length; ++i)
    {
        randomx= rand() % (chrs_size - 1);
        ch[i] = chrs[randomx];
    }

    QString ret(ch);
    delete[] ch;
    return ret;
}

void Widget::on_pushButton_11_clicked()
{
//    QUuid id = QUuid::createUuid();// ����
//    QString = id.toString();

    QString irand = getRandomString(8) + "_"+getRandomString(6);

    ui->id_lineEdit->setText(irand);
}
