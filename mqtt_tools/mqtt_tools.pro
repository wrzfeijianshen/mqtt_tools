#-------------------------------------------------
#
# Project created by QtCreator 2020-05-06T22:00:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mqtt_tools
TEMPLATE = app
CONFIG(debug, debug|release) {
    DESTDIR = ../bin/debug/
    OBJECTS_DIR = debug/
    MOC_DIR = debug/moc/
    RCC_DIR = debug/rcc/
    UI_DIR = debug/ui/
} else {
    DESTDIR = bin/release/
    OBJECTS_DIR = release/
    MOC_DIR = release/moc/
    RCC_DIR = release/rcc/
    UI_DIR = release/ui/
}


SOURCES += main.cpp\
        widget.cpp \
    mqttEngine.cpp

HEADERS  += widget.h \
    mqttEngine.h

FORMS    += widget.ui



win32: LIBS += -L$$PWD/3rd/eclipse_paho_mqtt_c_win32/lib/ -lpaho-mqtt3a
win32: LIBS += -L$$PWD/3rd/eclipse_paho_mqtt_c_win32/lib/ -lpaho-mqtt3as
win32: LIBS += -L$$PWD/3rd/eclipse_paho_mqtt_c_win32/lib/ -lpaho-mqtt3c
win32: LIBS += -L$$PWD/3rd/eclipse_paho_mqtt_c_win32/lib/ -lpaho-mqtt3cs

INCLUDEPATH += $$PWD/3rd/eclipse_paho_mqtt_c_win32/include
DEPENDPATH += $$PWD/3rd/eclipse_paho_mqtt_c_win32/include
