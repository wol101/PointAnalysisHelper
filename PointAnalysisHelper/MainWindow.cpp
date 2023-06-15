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

#include <QtWidgets>

#include "MainWindow.h"
#include "MdiChild.h"
#include "AboutDialog.h"
#include "PreferencesDialog.h"

MainWindow::MainWindow()
{
    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(updateMenus()));
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)),
            this, SLOT(setActiveSubWindow(QWidget*)));

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    updateMenus();

    m_preferences = new Preferences();
    readSettings();

    setWindowTitle(tr("PointAnalysisHelper"));
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
    delete m_preferences;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}

void MainWindow::newFile()
{
    // in this case, all new files must be assocaited with an existing RAS file
    QString fileName = QFileDialog::getOpenFileName(this, "Open RAS File", m_preferences->LastOpenedRASFile, "RAS Files (*.RAS *.ras);;Any File (*.*)", 0, EXTRA_FILE_DIALOG_OPTIONS);
    if (!fileName.isEmpty())
    {
        MdiChild *child = createMdiChild();
        child->SetRASFileName(fileName);
        child->SetPreferences(m_preferences);
        child->newFile();
        child->show();
        //child->showMaximized()
    }
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Point Analysis Locations File", m_preferences->LastOpenedPointLocationsFile, "XML Files (*.XML *.xml);;RAS Files (*.RAS *.ras);;Any File (*.*)", 0, EXTRA_FILE_DIALOG_OPTIONS);
    if (!fileName.isEmpty())
    {
        // if the file ends with .RAS, then treat it as new
        QFileInfo info(fileName);
        if (info.suffix() == "RAS" || info.suffix() == "ras")
        {
            MdiChild *child = createMdiChild();
            child->SetRASFileName(fileName);
            child->SetPreferences(m_preferences);
            child->newFile();
            child->show();
            //child->showMaximized()
        }
        else
        {
            // treat as open
            QMdiSubWindow *existing = findMdiChild(fileName);
            if (existing)
            {
                mdiArea->setActiveSubWindow(existing);
                return;
            }

            MdiChild *child = createMdiChild();
            child->SetPreferences(m_preferences);
            if (child->loadFile(fileName))
            {
                statusBar()->showMessage(tr("File loaded"), 2000);
                child->show();
                //child->showMaximized();
            }
            else
            {
                child->close();
            }
        }
    }
}

void MainWindow::save()
{
    if (activeMdiChild() && activeMdiChild()->save())
        statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::saveAs()
{
    if (activeMdiChild() && activeMdiChild()->saveAs())
        statusBar()->showMessage(tr("File saved"), 2000);
}

#ifndef QT_NO_CLIPBOARD
void MainWindow::cut()
{
    if (activeMdiChild())
        activeMdiChild()->cut();
}

void MainWindow::copy()
{
    if (activeMdiChild())
        activeMdiChild()->copy();
}

void MainWindow::paste()
{
    if (activeMdiChild())
        activeMdiChild()->paste();
}
#endif

void MainWindow::about()
{
    AboutDialog aboutDialog(this);

    int status = aboutDialog.exec();

    if (status == QDialog::Accepted)
    {
    }
}

void MainWindow::updateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
    saveAct->setEnabled(hasMdiChild);
    saveAsAct->setEnabled(hasMdiChild);
#ifndef QT_NO_CLIPBOARD
    pasteAct->setEnabled(hasMdiChild);
#endif
    closeAct->setEnabled(hasMdiChild);
    closeAllAct->setEnabled(hasMdiChild);
    tileAct->setEnabled(hasMdiChild);
    cascadeAct->setEnabled(hasMdiChild);
    nextAct->setEnabled(hasMdiChild);
    previousAct->setEnabled(hasMdiChild);
    separatorAct->setVisible(hasMdiChild);

#ifndef QT_NO_CLIPBOARD
    bool hasSelection = (activeMdiChild() &&
                         activeMdiChild()->textCursor().hasSelection());
    cutAct->setEnabled(hasSelection);
    copyAct->setEnabled(hasSelection);
#endif

    nextChannelAct->setEnabled(hasMdiChild);
    prevChannelAct->setEnabled(hasMdiChild);
    zoomInAct->setEnabled(hasMdiChild);
    zoomOutAct->setEnabled(hasMdiChild);
    autoGammaAct->setEnabled(hasMdiChild);
    exportImageAct->setEnabled(hasMdiChild);
    importChannelsAct->setEnabled(hasMdiChild);
}

void MainWindow::updateWindowMenu()
{
    windowMenu->clear();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAct);
    windowMenu->addAction(cascadeAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(separatorAct);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    separatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child->userFriendlyCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(child->userFriendlyCurrentFile());
        }
        QAction *action  = windowMenu->addAction(text);
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
        windowMapper->setMapping(action, windows.at(i));
    }
}

MdiChild *MainWindow::createMdiChild()
{
    MdiChild *child = new MdiChild;
    mdiArea->addSubWindow(child);

#ifndef QT_NO_CLIPBOARD
    connect(child, SIGNAL(copyAvailable(bool)),
            cutAct, SLOT(setEnabled(bool)));
    connect(child, SIGNAL(copyAvailable(bool)),
            copyAct, SLOT(setEnabled(bool)));
#endif

    return child;
}

void MainWindow::createActions()
{
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(QIcon(":/images/save-as.png"), tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

//! [0]
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
//! [0]

#ifndef QT_NO_CLIPBOARD
    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));
#endif

    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, SIGNAL(triggered()),
            mdiArea, SLOT(closeActiveSubWindow()));

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, SIGNAL(triggered()),
            mdiArea, SLOT(closeAllSubWindows()));

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, SIGNAL(triggered()),
            mdiArea, SLOT(activateNextSubWindow()));

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(previousAct, SIGNAL(triggered()),
            mdiArea, SLOT(activatePreviousSubWindow()));

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

#ifdef SHOW_ABOUT_QT
    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
#endif

    zoomInAct = new QAction(QIcon(":/images/zoom-in-3.png"), tr("Zoom In"), this);
    zoomInAct->setShortcuts(QKeySequence::ZoomIn);
    zoomInAct->setStatusTip(tr("Zooms in by a factor of 2"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(QIcon(":/images/zoom-out-3.png"), tr("Zoom Out"), this);
    zoomOutAct->setShortcuts(QKeySequence::ZoomOut);
    zoomOutAct->setStatusTip(tr("Zooms out by a factor of 2"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    nextChannelAct = new QAction(QIcon(":/images/go-next-view.png"), tr("Next Channel"), this);
    nextChannelAct->setShortcuts(QKeySequence::MoveToNextChar);
    nextChannelAct->setStatusTip(tr("Displays the next channel"));
    connect(nextChannelAct, SIGNAL(triggered()), this, SLOT(nextChannel()));

    prevChannelAct = new QAction(QIcon(":/images/go-previous-view.png"), tr("Previous Channel"), this);
    prevChannelAct->setShortcuts(QKeySequence::MoveToPreviousChar);
    prevChannelAct->setStatusTip(tr("Displays the previous channel"));
    connect(prevChannelAct, SIGNAL(triggered()), this, SLOT(prevChannel()));

    autoGammaAct = new QAction(QIcon(":/images/tools-wizard.png"), tr("Auto Gamma"), this);
    autoGammaAct->setStatusTip(tr("Calculates optimal gamma values to maximise contrast"));
    connect(autoGammaAct, SIGNAL(triggered()), this, SLOT(autoGamma()));

    exportImageAct = new QAction(QIcon(":/images/export-image-2.png"), tr("&Export Image..."), this);
    exportImageAct->setShortcut(QKeySequence(tr("Ctrl+E"))); // note that this is a single shortcut
    exportImageAct->setStatusTip(tr("Export the file as a standard image"));
    connect(exportImageAct, SIGNAL(triggered()), this, SLOT(exportImage()));

    setPrefsAct = new QAction(QIcon(":/images/preferences-desktop.png"), tr("&Preferences..."), this);
    setPrefsAct->setShortcuts(QKeySequence::Preferences);
    setPrefsAct->setStatusTip(tr("Set the allication preferences"));
    connect(setPrefsAct, SIGNAL(triggered()), this, SLOT(setPrefs()));

    importChannelsAct = new QAction(QIcon(":/images/import-channels.png"), tr("&Import Channels..."), this);
    importChannelsAct->setStatusTip(tr("Import the channel assignments from another file"));
    connect(importChannelsAct, SIGNAL(triggered()), this, SLOT(importChannels()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(importChannelsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(exportImageAct);
    fileMenu->addSeparator();
    //QAction *action = fileMenu->addAction(tr("Switch layout direction"));
    //connect(action, SIGNAL(triggered()), this, SLOT(switchLayoutDirection()));
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
#ifndef QT_NO_CLIPBOARD
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
#endif

    windowMenu = menuBar()->addMenu(tr("&Window"));
    updateWindowMenu();
    connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
#ifdef SHOW_ABOUT_QT
    helpMenu->addAction(aboutQtAct);
#endif
    helpMenu->addAction(setPrefsAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

#ifndef QT_NO_CLIPBOARD
    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
#endif
    imageToolBar = addToolBar(tr("Image"));
    imageToolBar->addAction(zoomInAct);
    imageToolBar->addAction(zoomOutAct);
    imageToolBar->addAction(prevChannelAct);
    imageToolBar->addAction(nextChannelAct);
    imageToolBar->addAction(autoGammaAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    m_preferences->Read();
    move(m_preferences->MainWindowPosition);
    resize(m_preferences->MainWindowSize);
}

void MainWindow::writeSettings()
{
    m_preferences->MainWindowPosition = pos();
    m_preferences->MainWindowSize = size();
    m_preferences->Write();
}

MdiChild *MainWindow::activeMdiChild()
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
        return qobject_cast<MdiChild *>(activeSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        if (mdiChild->currentFile() == canonicalFilePath)
            return window;
    }
    return 0;
}

void MainWindow::switchLayoutDirection()
{
    if (layoutDirection() == Qt::LeftToRight)
        qApp->setLayoutDirection(Qt::RightToLeft);
    else
        qApp->setLayoutDirection(Qt::LeftToRight);
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window)
        return;
    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::nextChannel()
{
    if (activeMdiChild())
        activeMdiChild()->nextChannel();
}

void MainWindow::prevChannel()
{
    if (activeMdiChild())
        activeMdiChild()->prevChannel();
}

void MainWindow::zoomIn()
{
    if (activeMdiChild())
        activeMdiChild()->zoomIn();
}

void MainWindow::zoomOut()
{
    if (activeMdiChild())
        activeMdiChild()->zoomOut();
}

void MainWindow::autoGamma()
{
    if (activeMdiChild())
        activeMdiChild()->autoGamma();
}

void MainWindow::exportImage()
{
    if (activeMdiChild())
        activeMdiChild()->exportImage();
}

void MainWindow::setPrefs()
{
    PreferencesDialog dialogPreferences(this);
    dialogPreferences.setValues(*m_preferences);

    int status = dialogPreferences.exec();
    if (status == QDialog::Accepted)
    {
        dialogPreferences.getValues(m_preferences);
    }
}

void MainWindow::importChannels()
{
    if (activeMdiChild())
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Open Point Analysis Locations File", m_preferences->LastOpenedPointLocationsFile, "XML Files (*.XML *.xml);;Any File (*.*)", 0, EXTRA_FILE_DIALOG_OPTIONS);
        if (!fileName.isEmpty())
        {
            activeMdiChild()->importChannels(fileName);
        }
    }
}



/*

#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QImage>
#include <QGraphicsScene>
#include <QPixmap>
#include <QPointF>
#include <QCursor>
#include <QPrinter>
#include <QProgressDialog>

#include <math.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "GraphicsView.h"
#include "DialogNewPoint.h"
#include "AboutDialog.h"
#include "GraphicsItemAnalysisLocation.h"
#include "GraphicsItemScaleBar.h"
#include "DialogLineCorrelations.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_preferences = 0;
    m_multiSpectralImage = 0;
    m_graphicsView = 0;
    m_graphicsScene = 0;
    m_dialogPointInfo = 0;
    m_pointLocations = 0;

    m_graphicsPixmapItem = 0;
    m_graphicsItemScaleBar = 0;

    m_preferences = new Preferences();
    m_preferences->Read();
    move(m_preferences->MainWindowPosition);
    resize(m_preferences->MainWindowSize);

    m_graphicsScene = new QGraphicsScene();
    m_graphicsView = new GraphicsView(this);
    m_graphicsView->setScene(m_graphicsScene);
    setCentralWidget(m_graphicsView);
    m_graphicsView->setCursor(QCursor(Qt::CrossCursor));

    m_dialogPointInfo = new DialogPointInfo(this);
    m_dialogPointInfo->setWindowFlags(Qt::Tool);
    m_dialogPointInfo->resize(m_preferences->DialogPointInfoSize);
    m_dialogPointInfo->move(m_preferences->DialogPointInfoPosition);
    m_dialogPointInfo->SetImageSize(-1, -1);
    m_dialogPointInfo->SetImageResolution(-1, -1);
    m_dialogPointInfo->SetScaleBar(m_preferences->ScaleBarLength, m_preferences->ScaleBarLocation, m_preferences->ScaleBarUnits);

    m_dialogLineCorrelations = new DialogLineCorrelations(this);
    m_dialogLineCorrelations->setWindowFlags(Qt::Tool);
    m_dialogLineCorrelations->resize(m_preferences->DialogLineCorrelationsSize);
    m_dialogLineCorrelations->move(m_preferences->DialogLineCorrelationsPosition);

    m_currentZoomLevel = 0;
    m_currentChannel = 0;
    m_unsavedData = false;

    QObject::connect(m_dialogPointInfo, SIGNAL(EmitCalibrationUpdate()), this, SLOT(CalibrationUpdate()));
    QObject::connect(m_dialogPointInfo, SIGNAL(EmitScaleBarUpdate()), this, SLOT(ScaleBarUpdate()));
    QObject::connect(m_dialogPointInfo, SIGNAL(EmitNewGamma(int, double)), this, SLOT(NewGamma(int, double)));
    QObject::connect(m_dialogPointInfo, SIGNAL(EmitNewChannelAssigment(int, QString &)), this, SLOT(NewChannelAssigment(int, QString &)));
    QObject::connect(m_dialogPointInfo, SIGNAL(EmitNewChannelSelected(int)), this, SLOT(NewChannelSelected(int)));
    QObject::connect(m_graphicsView, SIGNAL(EmitNewMappedLocation(const QPointF &)), this, SLOT(NewMappedLocation(const QPointF &)));

    m_dialogPointInfo->show();
    m_dialogLineCorrelations->show();

    m_preferences->LoadPreviousFileAtStartup = true;
    if (m_preferences->LoadPreviousFileAtStartup) // for some reason this resizes the window
    {
        m_pointLocationsFileName = m_preferences->LastSavedPointLocationsFile;
        QEvent *event = new QEvent(CUSTOM_EVENT_OpenPointLocationsFile);
        QCoreApplication::postEvent (this, event);
    }
}

MainWindow::~MainWindow()
{
    if (m_preferences) delete m_preferences;
    if (m_multiSpectralImage) delete m_multiSpectralImage;
    if (m_graphicsView) delete m_graphicsView;
    if (m_graphicsScene) delete m_graphicsScene;
    if (m_dialogPointInfo) delete m_dialogPointInfo;
    // if (m_graphicsPixmapItem) delete m_graphicsPixmapItem; // this gets deleted when m_graphicsScene is deleted
    if (m_pointLocations) delete m_pointLocations;

    delete ui;
}

void MainWindow::actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open RAS File", ".", "RAS Files (*.RAS *.ras)");

    if (fileName.isNull() == false)
    {
        m_fileName = fileName;
        QEvent *event = new QEvent(CUSTOM_EVENT_OpenRASFile);
        QCoreApplication::postEvent (this, event);
    }
}

void MainWindow::actionQuit_triggered()
{
    close();
}

void MainWindow::actionAbout_PointAnalysisHelper_triggered()
{
    AboutDialog aboutDialog(this);

    int status = aboutDialog.exec();

    if (status == QDialog::Accepted)
    {
    }
}

void MainWindow::actionZoom_In_triggered()
{
    m_currentZoomLevel++;
    m_graphicsView->scale(2.0, 2.0);
    UpdateStatus();
    update();
}

void MainWindow::actionZoom_Out_triggered()
{
    m_currentZoomLevel--;
    m_graphicsView->scale(0.5, 0.5);
    UpdateStatus();
    update();
}

void MainWindow::actionNext_Channel_triggered()
{
    m_currentChannel++;
    if (m_currentChannel >= m_multiSpectralImage->GetChannels()) m_currentChannel = 0;
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_multiSpectralImage->GetPreview(m_currentChannel)));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
    UpdateStatus();
    update();
}

void MainWindow::actionPrevious_Channel_triggered()
{
    m_currentChannel--;
    if (m_currentChannel < 0) m_currentChannel = m_multiSpectralImage->GetChannels() - 1;
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_multiSpectralImage->GetPreview(m_currentChannel)));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
    UpdateStatus();
    update();
}

void MainWindow::actionSave_triggered()
{
    if (m_pointLocationsFileName.isEmpty())
    {
        QString fileName = QFileDialog::getSaveFileName(this, "Save point locations data to file",
                                                        m_preferences->LastSavedPointLocationsFile, "XML Files (*.xml)");
        if (fileName.isNull()) return;
        m_pointLocationsFileName = fileName;
    }

    m_pointLocations->Write(m_pointLocationsFileName);
    m_preferences->LastSavedPointLocationsFile = m_pointLocationsFileName;
    m_unsavedData = false;
}

void MainWindow::actionSave_As_triggered()
{
    QString suggestedPath = m_pointLocationsFileName;
    if (suggestedPath.isEmpty()) suggestedPath = m_preferences->LastSavedPointLocationsFile;
    QString fileName = QFileDialog::getSaveFileName(this, "Save point locations data to file",
                                                    suggestedPath, "XML Files (*.xml)");
    if (fileName.isNull() == false)
    {
        m_pointLocationsFileName = fileName;
        actionSave_triggered();
    }
}

void MainWindow::actionOpen_Point_Locations_triggered()
{
    if (m_unsavedData)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, QApplication::applicationName(),
                                                                  "You have unsaved point locations.\nAre you sure you want to load new values?",
                                                                  QMessageBox::No | QMessageBox::Yes);
        if (reply == QMessageBox::No)
        {
            return;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, "Open Point Analysis Locations File", ".", "XML Files (*.XML *.xml)");

    if (fileName.isNull() == false)
    {
        m_pointLocationsFileName = fileName;
        QEvent *event = new QEvent(CUSTOM_EVENT_OpenPointLocationsFile);
        QCoreApplication::postEvent (this, event);
    }
}

void MainWindow::actionPreferences_triggered()
{

}

void MainWindow::actionNew_Point_Locations_triggered()
{
    if (m_unsavedData)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, QApplication::applicationName(),
                                                                  "You have unsaved point locations.\nAre you sure you want to delete them?",
                                                                  QMessageBox::No | QMessageBox::Yes);
        if (reply == QMessageBox::No)
        {
            return;
        }
    }

    m_pointLocationsFileName = "";
    if (m_pointLocations) delete m_pointLocations;
    m_pointLocations = new PointLocations();
    m_graphicsScene->clear();
    m_graphicsPixmapItem = 0;
    m_graphicsItemScaleBar = 0;

    if (m_multiSpectralImage)
    {
        m_pointLocations->SetMultiSpectralImage(m_multiSpectralImage);
        QImage *preview = m_multiSpectralImage->GetPreview(m_currentChannel);
        if (preview == 0)
        {
            QMessageBox::warning(this, "Warning", QString("Error %1 generating preview %2").arg(__LINE__).arg(m_fileName));
            return;
        }
        m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*preview));
        m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
    }
    m_dialogPointInfo->ClearPoints();

    ScaleBarUpdate();
    m_unsavedData = false;
}

void MainWindow::actionCalculate_Gammas_triggered()
{
    if (m_multiSpectralImage == 0) return;
    QStringList channelLabels;
    QList<double> gammas;
    int channel;
    double gamma;

    QProgressDialog progressDialog("Calculating optimal Gamma values...", "Abort calculation", 0, m_multiSpectralImage->GetChannels(), this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    for (channel = 0; channel < m_multiSpectralImage->GetChannels(); channel++)
    {
        progressDialog.setValue(channel);
        QApplication::processEvents();
        if (progressDialog.wasCanceled()) break;

        gamma = m_multiSpectralImage->OptimalGamma(channel);
        m_multiSpectralImage->RefreshPreview(channel, gamma);
        channelLabels.append(m_multiSpectralImage->GetImageData(channel)->label);
        gammas.append(m_multiSpectralImage->GetImageData(channel)->gamma);
    }
    m_dialogPointInfo->SetChannelAssigments(channelLabels, gammas);
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_multiSpectralImage->GetPreview(m_currentChannel)));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
    UpdateStatus();
    update();
    m_unsavedData = false;

    progressDialog.hide();
}

void MainWindow::actionSave_PDF_triggered()
{
    QFileInfo fileInfo(m_fileName);
    QString suggestedPath = fileInfo.path();
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select folder to save channels as a set of PDF files", suggestedPath, QFileDialog::ShowDirsOnly);

    if (dirPath.isNull()) return;

    QProgressDialog progressDialog("Generating PDF files...", "Abort Generation", 0, m_multiSpectralImage->GetChannels(), this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    int channel;
    for (channel = 0; channel < m_multiSpectralImage->GetChannels(); channel++)
    {
        progressDialog.setValue(channel);
        QApplication::processEvents();
        if (progressDialog.wasCanceled()) break;

        QGraphicsScene scene;
        QGraphicsPixmapItem *graphicsPixmapItem = scene.addPixmap(QPixmap::fromImage(*m_multiSpectralImage->GetPreview(channel)));
        scene.setSceneRect(graphicsPixmapItem->boundingRect());

        QMap<QString, PointAnalysisLocation> *pointMap = m_pointLocations->GetPointMap();
        QMap<QString, PointAnalysisLocation>::const_iterator it = pointMap->constBegin();
        while (it != pointMap->constEnd())
        {
            GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(it.key());
            newLocation->setPos(QPointF(it.value().imagePos.x() + 0.5f, it.value().imagePos.y() + 0.5f));
            newLocation->setZValue(1);
            scene.addItem(newLocation);
            ++it;
        }

        QPointF topLeft, bottomRight;
        double scaleBarLength, pixelSize;
        QPoint scaleBarLocation;
        QString scaleBarUnits, scaleBarLabel;
        m_pointLocations->GetCalibrations(&topLeft, &bottomRight);
        m_pointLocations->GetScaleBar(&scaleBarLength, &scaleBarLocation, &scaleBarUnits);
        scaleBarLabel = QString("%1 %2").arg(scaleBarLength).arg(scaleBarUnits);
        pixelSize = (scaleBarLength / (bottomRight.x() - topLeft.x())) * m_multiSpectralImage->GetImageData(channel)->width;
        GraphicsItemScaleBar *scaleBar = new GraphicsItemScaleBar(pixelSize, scaleBarLabel);
        scaleBar->setPos(QPointF(scaleBarLocation.x(), scaleBarLocation.y()));
        scaleBar->setZValue(1);
        scene.addItem(scaleBar);

        QString printFileName = QString("%1_ch%2.pdf").arg(fileInfo.completeBaseName()).arg(channel);
        QPrinter printer(QPrinter::ScreenResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setPaperSize(QSizeF(scene.sceneRect().width(), scene.sceneRect().height()), QPrinter::Millimeter);
        printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
        printer.setOutputFileName(QDir(dirPath).filePath(printFileName));
        QPainter painter(&printer);
        scene.render(&painter);
    }
    progressDialog.hide();
}

void MainWindow::OpenRASFile()
{
    QApplication::processEvents();
    QProgressDialog progressDialog("Opening RAS File...", QString(), 0, 3, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();
    QApplication::processEvents();

    int i;
    if (m_multiSpectralImage) delete m_multiSpectralImage;
    m_multiSpectralImage = new MultiSpectralImage();
    m_multiSpectralImage->SetDefaultGamma(m_preferences->DefaultGamma);
    int err = m_multiSpectralImage->ReadRASFile(m_fileName);
    if (err)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 reading %2").arg(err).arg(m_fileName));
        delete m_multiSpectralImage;
        m_multiSpectralImage = 0;
        return;
    }
    setWindowTitle(m_fileName);

    progressDialog.setValue(1);
    QApplication::processEvents();

    m_multiSpectralImage->CreatePreviews();
    m_preferences->LastOpenedRASFile = m_fileName;

    progressDialog.setValue(1);
    QApplication::processEvents();

    m_graphicsScene->clear();
    m_graphicsPixmapItem = 0;
    m_graphicsItemScaleBar = 0;

    QImage *preview = m_multiSpectralImage->GetPreview(m_currentChannel);
    if (preview == 0)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 generating preview %2").arg(__LINE__).arg(m_fileName));
        return;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*preview));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());

    QPointF topLeft(0, 0), bottomRight(1, 1);
    if (m_pointLocations) delete m_pointLocations;
    m_pointLocations = new PointLocations();
    m_pointLocations->SetMultiSpectralImage(m_multiSpectralImage);
    m_pointLocations->SetScaleBar(m_preferences->ScaleBarLength, m_preferences->ScaleBarLocation, m_preferences->ScaleBarUnits);
    m_pointLocations->SetCalibrations(topLeft, bottomRight);

    ImageData *imageData = m_multiSpectralImage->GetImageData(m_currentChannel);
    if (imageData == 0)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 accessing image data %2").arg(__LINE__).arg(m_fileName));
        return;
    }
    m_dialogPointInfo->ClearPoints();
    m_dialogPointInfo->SetScaleBar(m_preferences->ScaleBarLength, m_preferences->ScaleBarLocation, m_preferences->ScaleBarUnits);
    m_dialogPointInfo->SetCalibrations(topLeft, bottomRight);
    m_dialogPointInfo->SetImageSize(imageData->width, imageData->height);
    m_dialogPointInfo->SetImageResolution((bottomRight.x() - topLeft.x()) / (imageData->width - 1), (bottomRight.y() - topLeft.y()) / (imageData->height - 1));

    QStringList channelLabels;
    QList<double> gammas;
    for (i = 0; i < m_multiSpectralImage->GetChannels(); i++)
    {
        channelLabels.append(m_multiSpectralImage->GetImageData(i)->label);
        gammas.append(m_multiSpectralImage->GetImageData(i)->gamma);
    }
    m_dialogPointInfo->SetChannelAssigments(channelLabels, gammas);

    ScaleBarUpdate();

    progressDialog.setValue(2);
    QApplication::processEvents();
    progressDialog.hide();

    UpdateStatus();
    update();
    m_unsavedData = false;
}

void MainWindow::OpenPointLocationsFile()
{
    QProgressDialog progressDialog("Opening Point Location File...", QString(), 0, 3, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();
    QApplication::processEvents();

    if (m_pointLocations) delete m_pointLocations;
    m_pointLocations = new PointLocations();
    int err = m_pointLocations->Read(m_pointLocationsFileName);
    if (err)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 reading %2").arg(err).arg(m_pointLocationsFileName));
        delete m_pointLocations;
        m_pointLocations = 0;
        return;
    }
    if (m_multiSpectralImage) delete m_multiSpectralImage;
    m_multiSpectralImage = m_pointLocations->GetMultiSpectralImage();
    m_multiSpectralImage->CreatePreviews();
    m_fileName = m_multiSpectralImage->GetRASFileName();
    m_preferences->LastOpenedRASFile = m_fileName;
    m_preferences->LastSavedPointLocationsFile = m_pointLocationsFileName;
    setWindowTitle(m_fileName);

    progressDialog.setValue(1);
    QApplication::processEvents();

    m_graphicsScene->clear();
    m_graphicsPixmapItem = 0;
    m_graphicsItemScaleBar = 0;

    QImage *preview = m_multiSpectralImage->GetPreview(m_currentChannel);
    if (preview == 0)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 generating preview %2").arg(__LINE__).arg(m_fileName));
        return;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*preview));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());

    progressDialog.setValue(2);
    QApplication::processEvents();

    QPointF topLeft, bottomRight;
    ImageData *imageData = m_multiSpectralImage->GetImageData(m_currentChannel);
    if (imageData == 0)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 accessing image data %2").arg(__LINE__).arg(m_fileName));
        return;
    }
    m_pointLocations->GetCalibrations(&topLeft, &bottomRight);
    m_dialogPointInfo->SetCalibrations(topLeft, bottomRight);
    m_dialogPointInfo->SetImageSize(imageData->width, imageData->height);
    m_dialogPointInfo->SetImageResolution((bottomRight.x() - topLeft.x()) / (imageData->width - 1), (bottomRight.y() - topLeft.y()) / (imageData->height - 1));
    m_dialogPointInfo->ClearPoints();
    QMap<QString, PointAnalysisLocation> *pointMap = m_pointLocations->GetPointMap();
    QMap<QString, PointAnalysisLocation>::const_iterator it = pointMap->constBegin();
    while (it != pointMap->constEnd())
    {
        m_dialogPointInfo->AddPoint(it.key(), &it.value());
        GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(it.key());
        newLocation->setPos(QPointF(it.value().imagePos.x() + 0.5f, it.value().imagePos.y() + 0.5f));
        newLocation->setZValue(1);
        m_graphicsScene->addItem(newLocation);
        ++it;
    }

    QStringList channelLabels;
    QList<double> gammas;
    int i;
    for (i = 0; i < m_multiSpectralImage->GetChannels(); i++)
    {
        channelLabels.append(m_multiSpectralImage->GetImageData(i)->label);
        gammas.append(m_multiSpectralImage->GetImageData(i)->gamma);
    }
    m_dialogPointInfo->SetChannelAssigments(channelLabels, gammas);

    QPoint scaleBarLocation;
    double scaleBarLength;
    QString units;
    m_pointLocations->GetScaleBar(&scaleBarLength, &scaleBarLocation, &units);
    m_dialogPointInfo->SetScaleBar(scaleBarLength, scaleBarLocation, units);
    ScaleBarUpdate();

    progressDialog.setValue(3);
    QApplication::processEvents();
    progressDialog.hide();

    UpdateStatus();
    update();
    m_unsavedData = false;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    if (m_multiSpectralImage == 0) return;
    if (m_multiSpectralImage->GetChannels() == 0) return;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
   if (m_unsavedData)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, QApplication::applicationName(),
                                                                  "You have unsaved point locations.\nAre you sure you want to quit the application?",
                                                                  QMessageBox::No | QMessageBox::Yes);
        if (reply == QMessageBox::No)
        {
            return;
        }
    }

   m_preferences->DialogPointInfoSize = m_dialogPointInfo->size();
   m_preferences->DialogPointInfoPosition = m_dialogPointInfo->pos();
   m_preferences->DialogLineCorrelationsSize = m_dialogLineCorrelations->size();
   m_preferences->DialogLineCorrelationsPosition = m_dialogLineCorrelations->pos();

    m_preferences->MainWindowSize = size();
    m_preferences->MainWindowPosition = pos();
    m_preferences->Write();

    QMainWindow::closeEvent(event);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == CUSTOM_EVENT_OpenPointLocationsFile)
    {
        OpenPointLocationsFile();
        return true;
    }
    else if (event->type() == CUSTOM_EVENT_OpenRASFile)
    {
        OpenRASFile();
        return true;
    }

    return QMainWindow::event(event);
}

void MainWindow::UpdateStatus()
{
    ImageData *image = m_multiSpectralImage->GetImageData(m_currentChannel);
    QString status;
    if (image->label.size() == 0)
        status = QString("Channel %1 Zoom %2 Min %3 Max %4 Gamma %5").arg(m_currentChannel).arg(pow(2.0, m_currentZoomLevel)).arg(image->min).arg(image->max).arg(image->gamma);
    else
        status = QString("Channel %1 %2 Zoom %3 Min %4 Max %5 Gamma %6").arg(m_currentChannel).arg(image->label).arg(pow(2.0, m_currentZoomLevel)).arg(image->min).arg(image->max).arg(image->gamma);
    ui->statusBar->showMessage(status);
}

void MainWindow::NewMappedLocation(const QPointF &mappedLocation)
{
    QPoint p(int(mappedLocation.x()), int(mappedLocation.y()));
    QString newName = QString("Point (%1,%2)").arg(p.x()).arg(p.y());

    DialogNewPoint dialogNewPoint(this);
    dialogNewPoint.SetName(newName);

    int status = dialogNewPoint.exec();

    if (status == QDialog::Accepted)
    {
        newName = dialogNewPoint.GetName();
        PointAnalysisLocation *location = m_pointLocations->GetPoint(newName);
        if (location)
        {
            QMessageBox::StandardButton reply = QMessageBox::question(this, QApplication::applicationName(),
                                                                      QString("%1 exists. Do you wish to replace it?").arg(newName),
                                                                      QMessageBox::No | QMessageBox::Yes);
            if (reply == QMessageBox::No) return;
        }
        QPointF topLeft, bottomRight;
        m_dialogPointInfo->GetCalibrations(&topLeft, &bottomRight);
        m_pointLocations->SetCalibrations(topLeft, bottomRight);
        m_pointLocations->AddPoint(newName, p);
        m_dialogPointInfo->AddPoint(newName, m_pointLocations->GetPoint(newName));
        m_unsavedData = true;

        GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(newName);
        newLocation->setPos(QPointF(p.x() + 0.5, p.y() + 0.5));
        newLocation->setZValue(1);
        m_graphicsScene->addItem(newLocation);
        // m_graphicsScene->update(m_graphicsScene->sceneRect());
        update();
    }
}

void MainWindow::CalibrationUpdate()
{
    QPointF topLeft, bottomRight;
    ImageData *imageData = m_multiSpectralImage->GetImageData(m_currentChannel);
    if (imageData == 0) return;
    m_dialogPointInfo->GetCalibrations(&topLeft, &bottomRight);
    m_dialogPointInfo->SetImageSize(imageData->width, imageData->height);
    m_dialogPointInfo->SetImageResolution((bottomRight.x() - topLeft.x()) / (imageData->width - 1), (bottomRight.y() - topLeft.y()) / (imageData->height - 1));
    m_unsavedData = true;
    ScaleBarUpdate();
    update();
}

void MainWindow::NewGamma(int channel, double gamma)
{
    if (fabs(m_multiSpectralImage->GetImageData(channel)->gamma - gamma) > 0.00001)
    {
        m_multiSpectralImage->RefreshPreview(channel, gamma);
        if (m_graphicsPixmapItem)
        {
            m_graphicsScene->removeItem(m_graphicsPixmapItem);
            delete m_graphicsPixmapItem;
        }
        m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_multiSpectralImage->GetPreview(m_currentChannel)));
        m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
        m_unsavedData = true;
        UpdateStatus();
        update();
    }
}

void MainWindow::NewChannelAssigment(int channel, QString &label)
{
    if (m_multiSpectralImage->GetImageData(channel)->label != label)
    {
        m_multiSpectralImage->SetImageLabel(channel, label);
        m_unsavedData = true;
        UpdateStatus();
        update();
    }
}

void MainWindow::ScaleBarUpdate()
{
    if (m_graphicsItemScaleBar)
    {
        m_graphicsScene->removeItem(m_graphicsItemScaleBar);
        delete m_graphicsItemScaleBar;
    }
    QPointF topLeft, bottomRight;
    QPoint scaleBarLocation;
    double scaleBarLength;
    QString units;
    m_dialogPointInfo->GetCalibrations(&topLeft, &bottomRight);
    m_dialogPointInfo->GetScaleBar(&scaleBarLength, &scaleBarLocation, &units);
    m_pointLocations->SetScaleBar(scaleBarLength, scaleBarLocation, units);

    ImageData *imageData = m_multiSpectralImage->GetImageData(m_currentChannel);
    double pixelSize = (scaleBarLength / (bottomRight.x() - topLeft.x())) * imageData->width;
    QString scaleBarLabel = QString("%1 %2").arg(scaleBarLength).arg(units);
    m_graphicsItemScaleBar = new GraphicsItemScaleBar(pixelSize, scaleBarLabel);
    m_graphicsItemScaleBar->setPos(QPointF(scaleBarLocation.x(), scaleBarLocation.y()));
    m_graphicsItemScaleBar->setZValue(1);
    m_graphicsScene->addItem(m_graphicsItemScaleBar);
    m_unsavedData = true;
    update();
}

void MainWindow::NewChannelSelected(int channel)
{
    m_currentChannel = channel;
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_multiSpectralImage->GetPreview(m_currentChannel)));
    UpdateStatus();
    update();
}
 */
