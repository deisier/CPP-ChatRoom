#-------------------------------------------------
#
# Project created by QtCreator 2023-01-07T11:55:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetDisk
TEMPLATE = app

RC_ICONS = ./images/logo.ico


SOURCES += main.cpp\
    chatdialog.cpp \
        maindialog.cpp \
    ckernel.cpp \
    logindialog.cpp \
    mychat.cpp \
    mytablewightitem.cpp \
    useritem.cpp

HEADERS  += maindialog.h \
    chatdialog.h \
    ckernel.h \
    logindialog.h \
    common.h \
    mychat.h \
    mytablewightitem.h \
    useritem.h

FORMS    += maindialog.ui \
    chatdialog.ui \
    logindialog.ui \
    mychat.ui \
    useritem.ui


include(./netapi/netapi.pri)

INCLUDEPATH += ./netapi/net
INCLUDEPATH += ./netapi/mediator

include(./md5/md5.pri)
INCLUDEPATH +=./md5

RESOURCES += \
    resource.qrc

DISTFILES += \
    face/btn_avatar_a30.png \
    face/btn_avatar_a31.png \
    face/btn_avatar_a32.png \
    face/btn_avatar_a33.png
