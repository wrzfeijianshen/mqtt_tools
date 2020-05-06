#include "widget.h"
#include "ui_widget.h"
#include "mqttEngine.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    // 连接服务器
    CMqttEngine mqtt;
    mqtt.connect();

}
