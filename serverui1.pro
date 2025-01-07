QT += core gui quick websockets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clientwidget.cpp \
    dialogbox.cpp \
    main.cpp \
    mainwindow.cpp \
    websocketserver.cpp

HEADERS += \
    clientwidget.h \
    dialogbox.h \
    mainwindow.h \
    websocketserver.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Additional configurations for military theme
DEFINES += MILITARY_THEME
DEFINES += QT_MESSAGELOGCONTEXT

# Optimization flags
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_LFLAGS_RELEASE += -s

# Resource files if needed
# RESOURCES += \
#     resources.qrc
