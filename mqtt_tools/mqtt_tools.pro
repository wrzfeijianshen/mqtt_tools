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
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui


