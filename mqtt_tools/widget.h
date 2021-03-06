#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "mqttEngine.h"

namespace Ui {
class Widget;
}

class CMqttEngine;
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
public:
    void Init();

    void SetStatText(QString str, QString color = "black");
signals:
    void sig_RecvAddText(QString);
private slots:
    void RecvAddText(QString str);
private slots:
    void msgArrvd(CMqttMessage* mess);
    void msgConnLost();
private slots:
    void on_pushButton_clicked();

    void on_addr_lineEdit_editingFinished();

    void on_port_lineEdit_editingFinished();

    void on_id_lineEdit_editingFinished();

    void on_user_lineEdit_editingFinished();

    void on_pwd_lineEdit_editingFinished();

    void on_topic_lineEdit_editingFinished();

    void on_public_topic_lineEdit_editingFinished();

    void on_pushButton_2_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_8_clicked();

    void on_comboBox_6_currentIndexChanged(const QString &arg1);

    void on_comboBox_2_currentIndexChanged(int index);

    void on_comboBox_4_currentIndexChanged(int index);

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

private:
    Ui::Widget *ui;
    CMqttEngine *m_pMqtt;
    MQTTBroker m_Broker;

private:
    bool m_bConnect;// 连接
    QString m_SendfileName;
    QString m_appFileName;
    int m_iFileShardSize;// 设置分片大小
};

#endif // WIDGET_H
