#include "widget.h"
#include <QApplication>

const QString g_title = "1.0.526";
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    QString strTitle = "MQTT≤‚ ‘π§æﬂ-∑…Ω£…Ò_qq_908462363_";
    strTitle += g_title;
    w.setWindowTitle(strTitle);
    w.show();

    return a.exec();
}
