QT += widgets network widgets websockets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BCS_CONNECTOR/BCS_Exc.cpp \
    BCS_CONNECTOR/Instrument.cpp \
    Order.cpp \
    SetupKey.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    BCS_CONNECTOR/BCS_Exc.h \
    BCS_CONNECTOR/Instrument.h \
    BCS_CONNECTOR/MoexExchangeConnector.h \
    Order.h \
    SetupKey.h \
    mainwindow.h
FORMS += \
    Order.ui \
    SetupKey.ui \
    mainwindow.ui
ANDROID_PERMISSIONS = \
        android.permission.INTERNET \
        android.permission.ACCESS_NETWORK_STATE
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
android: include(D:/Qt/Tools/OpenSslAndroid/openssl.pri)
