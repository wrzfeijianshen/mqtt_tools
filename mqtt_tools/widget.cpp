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
    CMD_S2C_CONNECT = 0x10,    //服务收到连接tcp信息，向client发送连接命令
    CMD_S2C_DOWNFILE,          //
};


QString GetFileSize(const qint64 &size)
{
    int integer = 0;  //整数位
    int decimal = 0;  //小数位，保留三位
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
    // 获取收到的时间
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

    // 解析json
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
            // 输出信息
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
            ui->plainTextEdit_3->appendPlainText(fileId +"/"+ fileName +",接收进度为 "  + QString::number(fileIndex2) +'/' +QString::number(fileCount2) );

            if(fileCount2 == fileIndex2)
            {
                ui->plainTextEdit_3->appendPlainText(fileId +"/"+ fileName +",接收成功");
                qDebug() << "接收成功";
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
        // 字符串
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
    SetStatText("连接已断开","red");
    ui->pushButton->setText("连接");
    m_pMqtt->Destroy();
}

void Widget::Init()
{
    if(m_pMqtt == nullptr)
    {
        m_pMqtt = new CMqttEngine();

        connect(m_pMqtt,SIGNAL(sig_msgArrvd(CMqttMessage*)),this,SLOT(msgArrvd(CMqttMessage*)));// 绑定
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
        ui->pushButton->setText("连接");
        m_pMqtt->Destroy();
        SetStatText("已断开服务器");
    }
    else
    {
        // 连接服务器
        m_pMqtt->GetConfig()->SetBroker(m_Broker);
        int ret =  m_pMqtt->Connect();
        if(ret != 0)
        {
            qDebug() <<"m_pMqtt 连接失败 ";
            SetStatText("连接失败,状态码:[" +QString::number(ret)+"]","red");
        }
        else
        {
            m_bConnect = true;
            ui->pushButton->setText("断开连接");
            SetStatText("已连接到服务器");
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
    // 连接服务器
    int ret = m_pMqtt->SetSubscribe(m_Broker.topic,index);
    if(ret != 0)
    {
        qDebug() <<"m_pMqtt 连接失败 ";
        ui->pushButton_2->setText("订阅");
    }
}

void Widget::on_pushButton_6_clicked()
{
    // 发布主题
    int ret = 0;
    QString str = ui->plainTextEdit_2->toPlainText();
    int index =  ui->comboBox_3->currentIndex();
    int check = ui->checkBox->isChecked()== true ?1:0;
    // 判断是否json串发送.

    // 连接服务器
    ret = m_pMqtt->PublishMessage(m_Broker.publish_topic,str,index,check);
    if(ret != 0)
    {
        qDebug() <<"m_pMqtt 连接失败 ";
        return;
    }
    return ;
    qDebug() << "发送完毕";
}

void Widget::on_pushButton_3_clicked()
{
    // 清空
    ui->plainTextEdit->clear();
}

void Widget::on_pushButton_5_clicked()
{
    // 清空
    ui->plainTextEdit_2->clear();
}

void Widget::on_pushButton_4_clicked()
{
    // 取消订阅
    int ret = m_pMqtt->SetUnSubscribe(m_Broker.topic);
    if(ret != 0)
    {
        qDebug() << "订阅不存在 " << ret;
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

        // 据测试 小于可能1M的可以发送完毕 917.119K,如果发送超过1M字节大小则造成发送失败,断开连接
        // 如果固定大小发送500k数据二进制文件,
        // 100M 东西大概1分钟.
        // 下载端和接收端则边读边发送,不然都读取到内存,受不了.

        // 猜测: 如果心跳包存在,同步客户端,可能会导致心跳包发送或者接收,在指定时间内,断开与服务器的连接,导致文件发送失败.
        // conn_opts.keepAliveInterval = 0; 则不发送心跳包,如果是正常的话,文件传输应该没有什么问题.不会中途断开连接.

        if(m_SendfileName == "")
        {
            SetStatText("文件为空,请选择文件","red");
            return;
        }
        QFileInfo fileinfo;
        fileinfo = QFileInfo(m_SendfileName);
        qint64 size = fileinfo.size();

        int count = size /(1024*500);
        int count1 = size %(1024*500);
        if(count1 > 0)
            count++;
        qDebug() <<"文件大小 size : " <<size <<"发送次数 "<< count;

        QDateTime time = QDateTime::currentDateTime();   //获取当前时间
        int timeT = time.toTime_t();
        qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
        int irand =  qrand() % (99999 - 10000) + 10000;

        QString fileId = QString::number(timeT)+'_'+QString::number(irand);


        QString file_name = fileinfo.fileName();

        QFile file(m_SendfileName);//
        bool isOk = file.open(QFile::ReadOnly);
        if (!isOk) {
            SetStatText("文件为空,请选择文件","red");
            qDebug() <<"file : no ";
            return;
        }

#if 1
        QString pubTopoc = ui->public_topic_lineEdit_2->text();
        qDebug() << pubTopoc;
        int index =  ui->comboBox_5->currentIndex();
        int check = ui->checkBox->isChecked()== true ?1:0;

        // 读文件
        int len = 0;
        int num = 0;
        do
        {
            // 分片
            QByteArray array = file.read(1024*500);

            len =array.size();
            if(len == 0)
            {
                qDebug() << "file : size 0";
                if(count == num)
                   emit RecvAddText(fileId +"/"+ file_name +",发送完成 "  + QString::number(num) +'/' +QString::number(count) );
                break;
            }
            num++;
            qDebug() <<"file : size [ " << GetFileSize(len) << "发送次数 " << num;
            QString str = Base64::encode(array);

            QJsonArray arr;

            QJsonObject o
            {
                { "fileID", fileId},
                { "fileName", file_name},//发送的文件名
                { "fileSize", size},// 文件大小
                { "fileCount", count},// 当前的传输的总次数
                { "fileIndex", num},// 当前传输的第几次
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
            QThread::msleep(10);// 休息一段时间
            int ret = m_pMqtt->PublishJsonMessage(pubTopoc,barr.data(),index,check);
            if(ret != 0)
            {
                qDebug() <<"m_pMqtt push " << ret;
                return;
            }
            emit RecvAddText(fileId +"/"+ file_name +",已发送 "  + QString::number(num) +'/' +QString::number(count) );// 阻塞暂定
        }
        while(1);

        SetStatText("发送文件成功");
        qDebug() <<"m_pMqtt push 发送文件成功 : ";

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
    // 选择发送文件
    m_SendfileName = QFileDialog::getOpenFileName(
                this, tr("open file"),
                "./", tr("files All files (*.*)"));

    if(m_SendfileName.isEmpty())
    {
        SetStatText("打开文件失败","red");
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
    // 下次收到后会转换
}

void Widget::on_comboBox_4_currentIndexChanged(int index)
{
    // 选择json串,清空内容
    if ( index == 1)
    {
        QJsonObject json
        {
            { "type", 1 },
            { "data", "江湖再见"}
        };
        QJsonDocument document;
        document.setObject(json);
        ui->plainTextEdit_2->clear();
        ui->plainTextEdit_2->appendPlainText(document.toJson());
    }
}

void Widget::on_pushButton_10_clicked()
{
    // 一个 Topic 只能有 1 条 Retained 消息，发布新的 Retained 消息将覆盖老的 Retained 消息
    // Retained 消息是 Broker 为每一个 Topic 单独存储的，而持久性会话是 Broker 为每一个 Client 单独存储的。
    // 删除一个 Retained 消息也很简单 : 向这个主题发布一个 Payload 长度为 0 的 Retained 消息就可以了
    // 连接服务器
    int index =  ui->comboBox_3->currentIndex();
    int ret = m_pMqtt->PublishMessage(m_Broker.publish_topic,"",index,1);
    if(ret != 0)
    {
        qDebug() <<"m_pMqtt 连接失败 ";
        return;
    }
    return ;
    qDebug() << "发送完毕";
}
QString getRandomString(int length)
{
    qsrand(QDateTime::currentMSecsSinceEpoch());//为随机值设定一个seed

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
//    QUuid id = QUuid::createUuid();// 或者
//    QString = id.toString();

    QString irand = getRandomString(8) + "_"+getRandomString(6);

    ui->id_lineEdit->setText(irand);
}
