/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Preferences.h"

class MdiChild;
QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
#ifndef QT_NO_CLIPBOARD
    void cut();
    void copy();
    void paste();
#endif
    void about();
    void updateMenus();
    void updateWindowMenu();
    MdiChild *createMdiChild();
    void switchLayoutDirection();
    void setActiveSubWindow(QWidget *window);

    void nextChannel();
    void prevChannel();
    void zoomIn();
    void zoomOut();
    void autoGamma();
    void exportImage();
    void setPrefs();
    void importChannels();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);

    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *windowMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *imageToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
#ifndef QT_NO_CLIPBOARD
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
#endif
    QAction *closeAct;
    QAction *closeAllAct;
    QAction *tileAct;
    QAction *cascadeAct;
    QAction *nextAct;
    QAction *previousAct;
    QAction *separatorAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QAction *nextChannelAct;
    QAction *prevChannelAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *autoGammaAct;
    QAction *exportImageAct;
    QAction *setPrefsAct;
    QAction *importChannelsAct;

    Preferences *m_preferences;
};

#endif

/*
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPointF>

#include "MultiSpectralImage.h"
#include "Preferences.h"
#include "DialogPointInfo.h"
#include "GraphicsView.h"
#include "PointLocations.h"
#include "GraphicsItemScaleBar.h"
#include "DialogLineCorrelations.h"

#define CUSTOM_EVENT_OpenPointLocationsFile ((QEvent::Type)(QEvent::User + 1))
#define CUSTOM_EVENT_OpenRASFile ((QEvent::Type)(QEvent::User + 2))

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void OpenRASFile();
    void OpenPointLocationsFile();

    ImageData *GetCurrentImageData() { return m_multiSpectralImage->GetImageData(m_currentChannel); }

public slots:
    void actionOpen_triggered();
    void actionQuit_triggered();
    void actionZoom_In_triggered();
    void actionZoom_Out_triggered();
    void actionNext_Channel_triggered();
    void actionPrevious_Channel_triggered();
    void actionSave_triggered();
    void actionSave_As_triggered();
    void actionSave_PDF_triggered();
    void actionAbout_PointAnalysisHelper_triggered();
    void actionOpen_Point_Locations_triggered();
    void actionPreferences_triggered();
    void actionNew_Point_Locations_triggered();
    void actionCalculate_Gammas_triggered();

    void CalibrationUpdate();
    void NewMappedLocation(const QPointF &mappedLocation);
    void UpdateStatus();
    void NewGamma(int channel, double gamma);
    void NewChannelAssigment(int channel, QString &label);
    void ScaleBarUpdate();
    void NewChannelSelected(int channel);

protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);
    bool event(QEvent *event);

private:
    Ui::MainWindow *ui;

    Preferences *m_preferences;
    MultiSpectralImage *m_multiSpectralImage;
    GraphicsView *m_graphicsView;
    QGraphicsScene *m_graphicsScene;
    DialogPointInfo *m_dialogPointInfo;
    PointLocations *m_pointLocations;
    DialogLineCorrelations *m_dialogLineCorrelations;

    QGraphicsPixmapItem *m_graphicsPixmapItem;
    GraphicsItemScaleBar *m_graphicsItemScaleBar;

    QString m_fileName;
    QString m_pointLocationsFileName;

    int m_currentZoomLevel;
    int m_currentChannel;
    bool m_unsavedData;
};

#endif // MAINWINDOW_H
*/
