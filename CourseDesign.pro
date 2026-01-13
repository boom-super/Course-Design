QT       += core gui sql network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    approvaldialog.cpp \
    databasemanager.cpp \
    exportdialog.cpp \
    formreportview.cpp \
    loginview.cpp \
    main.cpp \
    mainwindow.cpp \
    publishview.cpp \
    workerthread.cpp

HEADERS += \
    approvaldialog.h \
    databasemanager.h \
    exportdialog.h \
    formreportview.h \
    loginview.h \
    mainwindow.h \
    publishview.h \
    workerthread.h

FORMS += \
    approvaldialog.ui \
    exportdialog.ui \
    formreportview.ui \
    loginview.ui \
    mainwindow.ui \
    publishview.ui

# 生成的UI头文件路径
INCLUDEPATH += $$PWD

# 目标文件名
TARGET = CampusActivitySystem
TEMPLATE = app

# 部署（Windows）
win32 {
    DESTDIR = $$PWD/bin
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/bin
    } else {
        LIBS += -L$$PWD/bin
    }
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
