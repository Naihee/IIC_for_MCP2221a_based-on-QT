QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    mcp2221a.cpp

HEADERS += \
    mainwindow.h \
    mcp2221a.h

FORMS += \
    mainwindow.ui


#RC_ICONS = qcmcsdt.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lmcp2221_dll_um_x64
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lmcp2221_dll_um_x64

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

