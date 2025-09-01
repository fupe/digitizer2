QT       += core gui serialport widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
INCLUDEPATH += /usr/include/eigen3

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    3rdparty/dxflib/src/dl_dxf.cpp \
    3rdparty/dxflib/src/dl_writer_ascii.cpp \
    CustomToolButton.cpp \
    GraphicsItems.cpp \
    InfoDialog.cpp \
    MeasureDialog.cpp \
    SettingsDialog.cpp \
    appmanager.cpp \
    calibratewindow.cpp \
    calibrationengine.cpp \
    main.cpp \
    mainwindow.cpp \
    serialmanager.cpp \
    serialworker.cpp \
    settings.cpp \
    settingsmanager.cpp \
    shapemanager.cpp \
    shortcutsdialog.cpp \
    simworker.cpp

HEADERS += \
    3rdparty/dxflib/src/dl_codes.h \
    3rdparty/dxflib/src/dl_creationadapter.h \
    3rdparty/dxflib/src/dl_dxf.h \
    3rdparty/dxflib/src/dl_writer_ascii.h \
    CustomToolButton.h \
    GraphicsItems.h \
    InfoDialog.h \
    MeasureDialog.h \
    SettingsDialog.h \
    appmanager.h \
    calibratewindow.h \
    calibrationengine.h \
    idatasourceworker.h \
    mainwindow.h \
    modbusworker.h \
    serialmanager.h \
    serialworker.h \
    settings.h \
    settingsmanager.h \
    shapemanager.h \
    shortcutsdialog.h \
    simworker.h

FORMS += \
    InfoDialog.ui \
    MeasureDialog.ui \
    SettingsDialog.ui \
    calibratewindow.ui \
    mainwindow.ui \
    shortcutsdialog.ui

TRANSLATIONS += translations/english.ts \
                translations/czech.ts \
                translations/german.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
