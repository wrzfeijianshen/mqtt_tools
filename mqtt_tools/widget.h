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
private slots:
    void on_pushButton_clicked();

    void on_addr_lineEdit_editingFinished();

    void on_port_lineEdit_editingFinished();

    void on_id_lineEdit_editingFinished();

    void on_user_lineEdit_editingFinished();

    void on_pwd_lineEdit_editingFinished();

    void on_topic_lineEdit_editingFinished();

    void on_public_topic_lineEdit_editingFinished();

private:
    Ui::Widget *ui;
    CMqttEngine *m_pMqtt;
    MQTTBroker m_Broker;
};

#endif // WIDGET_H
