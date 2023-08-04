QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addfriend.cpp \
    addfriendrequest.cpp \
    chatwidget.cpp \
    loginwidget.cpp \
    main.cpp \
    mainwidget.cpp \
    protoclo.cpp \
    recvfile.cpp \
    register.cpp \
    sendfilelist.cpp \
    sharefile.cpp \
    tcpclient.cpp \
    userinfowidget.cpp

HEADERS += \
    addfriend.h \
    addfriendrequest.h \
    chatwidget.h \
    loginwidget.h \
    mainwidget.h \
    protoclo.h \
    recvfile.h \
    register.h \
    sendfilelist.h \
    sharefile.h \
    tcpclient.h \
    userinfowidget.h

FORMS += \
    addfriend.ui \
    addfriendrequest.ui \
    chatwidget.ui \
    loginwidget.ui \
    mainwidget.ui \
    recvfile.ui \
    register.ui \
    sharefile.ui \
    tcpclient.ui \
    userinfowidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Icon.qrc \
    clientConfig.qrc

DISTFILES += \
    img/log.png
