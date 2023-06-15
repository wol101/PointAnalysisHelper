#-------------------------------------------------
#
# Project created by QtCreator 2013-12-22T13:41:47
#
#-------------------------------------------------

cache()

macx {
    ICON = Icon.icns
    DEFINES += EXTRA_FILE_DIALOG_OPTIONS=0
#    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
#    QMAKE_MAC_SDK = macosx10.11
}
else:win32 {
    RC_FILE = app.rc
    DEFINES += EXTRA_FILE_DIALOG_OPTIONS=0
}
else:unix {
    DEFINES += EXTRA_FILE_DIALOG_OPTIONS=QFileDialog::DontUseNativeDialog
}

QT       += core gui xml printsupport svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PointAnalysisHelper
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    MultiSpectralImage.cpp \
    DataFile.cpp \
    Preferences.cpp \
    GraphicsView.cpp \
    PointLocations.cpp \
    DialogNewPoint.cpp \
    GraphicsItemAnalysisLocation.cpp \
    AboutDialog.cpp \
    GraphicsItemScaleBar.cpp \
    MdiChild.cpp \
    GraphicsViewLineCorrelationChart.cpp \
    ImageExporter.cpp \
    PreferencesDialog.cpp \
    DialogExportImage.cpp

HEADERS  += MainWindow.h \
    MultiSpectralImage.h \
    DataFile.h \
    Preferences.h \
    GraphicsView.h \
    PointLocations.h \
    DialogNewPoint.h \
    GraphicsItemAnalysisLocation.h \
    AboutDialog.h \
    GraphicsItemScaleBar.h \
    MdiChild.h \
    GraphicsViewLineCorrelationChart.h \
    ImageExporter.h \
    PreferencesDialog.h \
    DialogExportImage.h

FORMS    += \
    DialogNewPoint.ui \
    AboutDialog.ui \
    MdiChild.ui \
    PreferencesDialog.ui \
    DialogExportImage.ui

RESOURCES += \
    mdi.qrc

OTHER_FILES += \
    app.rc
