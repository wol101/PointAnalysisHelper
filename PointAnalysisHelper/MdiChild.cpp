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
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QImage>
#include <QGraphicsScene>
#include <QPixmap>
#include <QPointF>
#include <QCursor>
#include <QProgressDialog>
#include <QBoxLayout>
#include <QHeaderView>

#include <math.h>

#include "MdiChild.h"
#include "ui_MdiChild.h"
#include "PointLocations.h"
#include "GraphicsView.h"
#include "DialogNewPoint.h"
#include "GraphicsItemAnalysisLocation.h"
#include "GraphicsItemScaleBar.h"
#include "MainWindow.h"
#include "ImageExporter.h"
#include "PreferencesDialog.h"
#include "DialogExportImage.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

MdiChild::MdiChild(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MdiChild)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;
    isModified = false;

    m_graphicsView = 0;
    m_graphicsScene = 0;
    m_pointLocations = 0;
    m_preferences = 0;
    m_graphicsItemScaleBar = 0;
    m_graphicsPixmapItem = 0;

    m_currentZoomLevel = 0;
    m_currentChannel = 0;

    // set the default behaviour for spin boxes to keyboardTracking = false
    QList<QAbstractSpinBox *> spinBoxes = findChildren<QAbstractSpinBox *>();
    for (int i = 0; i < spinBoxes.size(); i++) spinBoxes[i]->setKeyboardTracking(false);

#ifdef TIGHTER_LAYOUT
    // opt for a tighter layout
    QList<QLayout *> layouts = findChildren<QLayout *>();
    int spacing = 6;
    for (int i = 0; i < layouts.size(); i++)
    {
        layouts[i]->setSpacing(spacing); // 0 is minimum
        layouts[i]->setMargin(spacing); // 0 is minimum
        layouts[i]->setContentsMargins(spacing,spacing,spacing,spacing); // 0,0,0,0 is minimum
    }
#endif

    ui->tableWidgetPointCountLocations->setRowCount(0);
    ui->tableWidgetPointCountLocations->setColumnCount(5);
    QStringList labels;
    labels << "Name" << "XPix" << "YPix" << "Xmm" << "Ymm";
    ui->tableWidgetPointCountLocations->setHorizontalHeaderLabels(labels);
    ui->tableWidgetPointCountLocations->resizeColumnsToContents();
    ui->tableWidgetPointCountLocations->resizeRowsToContents();

    QStringList channelLabels;
    QList<double> gammas, percentiles;
    QList<uint32_t> mins, maxs;
    SetChannelAssignments(channelLabels, gammas, percentiles, mins, maxs);

    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsScene->setSceneRect(0, 0, 10000, 10000); // this ensures that the m_graphicsView is as big as it can be
    m_graphicsView = new GraphicsView(this);
    m_graphicsView->setScene(m_graphicsScene);
    m_graphicsView->setCursor(Qt::CrossCursor);
    QBoxLayout *boxLayout = new QBoxLayout(QBoxLayout::LeftToRight, ui->widgetGraphicsViewPlaceholder);
    boxLayout->setMargin(0);
    boxLayout->addWidget(m_graphicsView);

    m_graphicsSceneLineCorrelationChart = new QGraphicsScene(this);
    m_graphicsViewLineCorrelationChart = new GraphicsViewLineCorrelationChart(this);
    m_graphicsViewLineCorrelationChart->setScene(m_graphicsSceneLineCorrelationChart);
    boxLayout = new QBoxLayout(QBoxLayout::LeftToRight, ui->widgetLineCorrelationChartPlaceholder);
    boxLayout->setMargin(0);
    boxLayout->addWidget(m_graphicsViewLineCorrelationChart);

    // this line connects a single click in the vertical header (row select) to the channel changing slot
    connect(ui->tableWidgetChannelAssignments->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(NewChannelSelected(int)));

    ui->tableWidgetPointCountLocations->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidgetPointCountLocations->verticalHeader(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(menuRequestPointCountLocations(const QPoint &)));
    ui->tableWidgetPointCountLocations->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidgetPointCountLocations, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(menuRequestPointCountLocations(const QPoint &)));
    connect(ui->tableWidgetPointCountLocations, SIGNAL(cellChanged(int, int)), this, SLOT(PointLocationsCellChanged(int, int)));

    connect(m_graphicsView, SIGNAL(EmitNewMappedLocation(const QPointF &)), this, SLOT(NewMappedLocation(const QPointF &)));

    connect(ui->doubleSpinBoxXMin, SIGNAL(valueChanged(double)), this, SLOT(CalibrationUpdate()));
    connect(ui->doubleSpinBoxYMin, SIGNAL(valueChanged(double)), this, SLOT(CalibrationUpdate()));
    connect(ui->doubleSpinBoxXMax, SIGNAL(valueChanged(double)), this, SLOT(CalibrationUpdate()));
    connect(ui->doubleSpinBoxYMax, SIGNAL(valueChanged(double)), this, SLOT(CalibrationUpdate()));

    connect(ui->spinBoxMarkerFontSize, SIGNAL(valueChanged(int)), this, SLOT(RecalculatePoints()));

    connect(ui->spinBoxScaleBarPosX, SIGNAL(valueChanged(int)), this, SLOT(ScaleBarUpdate()));
    connect(ui->spinBoxScaleBarPosY, SIGNAL(valueChanged(int)), this, SLOT(ScaleBarUpdate()));
    connect(ui->spinBoxScaleBarFontSize, SIGNAL(valueChanged(int)), this, SLOT(ScaleBarUpdate()));
    connect(ui->doubleSpinBoxScaleBarLength, SIGNAL(valueChanged(double)), this, SLOT(ScaleBarUpdate()));
    connect(ui->lineEditScaleBarUnits, SIGNAL(textChanged(QString)), this, SLOT(ScaleBarUpdate()));
    connect(ui->pushButtonMarkerColor, SIGNAL(clicked()), this, SLOT(markerColorClicked()));
    connect(ui->pushButtonScaleBarColor, SIGNAL(clicked()), this, SLOT(scaleBarColorClicked()));

    connect(ui->tableWidgetChannelAssignments, SIGNAL(cellChanged(int, int)), this, SLOT(ChannelAssignmentsCellChanged(int, int)));

    connect(ui->checkBoxMean, SIGNAL(clicked()), this, SLOT(CalculateClicked()));
    connect(ui->spinBoxRowPairs, SIGNAL(valueChanged(int)), this, SLOT(CalculateClicked()));
    connect(ui->spinBoxNumberOffsets, SIGNAL(valueChanged(int)), this, SLOT(CalculateClicked()));
    connect(m_graphicsViewLineCorrelationChart, SIGNAL(EmitClicked()), this, SLOT(CalculateClicked()));

    connect(ui->spinBoxLineOffset, SIGNAL(valueChanged(int)), this, SLOT(lineOffsetValueClicked()));

    connect(ui->checkBoxApplyOffset, SIGNAL(clicked()), this, SLOT(applyOffsetClicked()));

    connect(ui->pushButtonMaxPairs, SIGNAL(clicked()), this, SLOT(maxPairsButtonClicked()));

}

MdiChild::~MdiChild()
{
    if (m_graphicsView) delete m_graphicsView;
    if (m_graphicsScene) delete m_graphicsScene;
    if (m_graphicsViewLineCorrelationChart) delete m_graphicsViewLineCorrelationChart;
    if (m_graphicsSceneLineCorrelationChart) delete m_graphicsSceneLineCorrelationChart;
    if (m_pointLocations) delete m_pointLocations;

    delete ui;
}

void MdiChild::newFile()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    OpenRASFile();
    QApplication::restoreOverrideCursor();

    static int sequenceNumber = 1;

    isUntitled = true;
    isModified = false;
    curFile = QString("document%1.xml").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");
}

bool MdiChild::loadFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    isModified = false;
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool err = OpenPointLocationsFile();
    QApplication::restoreOverrideCursor();
    return err;
}

bool MdiChild::save()
{
    if (isUntitled)
    {
        return saveAs();
    }
    else
    {
        return saveFile(curFile);
    }
}

bool MdiChild::saveAs()
{
    QString suggestedFile = curFile;
    if (isUntitled)
    {
        QFileInfo fileInfo(m_RASFileName);
        QString parentPath = fileInfo.path();
        QString baseName = fileInfo.completeBaseName();
        suggestedFile = QDir(parentPath).filePath(baseName) + ".xml";
    }
    // qWarning("Suggested Path %s\n", qPrintable(suggestedFile));
    QString fileName = QFileDialog::getSaveFileName(this, "Save point locations data to file", suggestedFile, "XML Files (*.xml)", 0, EXTRA_FILE_DIALOG_OPTIONS);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_pointLocations->Write(fileName);
    m_preferences->LastOpenedPointLocationsFile = curFile;
    QApplication::restoreOverrideCursor();
    curFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = false;
    isModified = false;
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");

    return true;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void MdiChild::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MdiChild::documentWasModified()
{
    setWindowModified(isModified);
}

bool MdiChild::maybeSave()
{
    if (isModified)
    {
    QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("MDI"),
                     tr("'%1' has been modified.\n"
                        "Do you want to save your changes?")
                     .arg(userFriendlyCurrentFile()),
                     QMessageBox::Save | QMessageBox::Discard
             | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

QString MdiChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MdiChild::zoomIn()
{
    m_currentZoomLevel++;
    m_graphicsView->scale(2.0, 2.0);
    update();
}

void MdiChild::zoomOut()
{
    m_currentZoomLevel--;
    m_graphicsView->scale(0.5, 0.5);
    update();
}

void MdiChild::nextChannel()
{
    m_currentChannel++;
    if (m_currentChannel >= m_pointLocations->GetMultiSpectralImage()->GetChannels()) m_currentChannel = 0;
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel)));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
    CalculateClicked();
    ui->tableWidgetChannelAssignments->selectRow(m_currentChannel);
    update();
}

void MdiChild::prevChannel()
{
    m_currentChannel--;
    if (m_currentChannel < 0) m_currentChannel = m_pointLocations->GetMultiSpectralImage()->GetChannels() - 1;
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel)));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
    CalculateClicked();
    ui->tableWidgetChannelAssignments->selectRow(m_currentChannel);
    update();
}

void MdiChild::autoGamma()
{
    if (m_pointLocations->GetMultiSpectralImage() == 0) return;
    QStringList channelLabels;
    QList<double> gammas, percentiles;
    QList<uint32_t> mins, maxs;
    int channel;
    double gamma;

    QProgressDialog progressDialog("Calculating optimal Gamma values...", "Abort calculation", 0, m_pointLocations->GetMultiSpectralImage()->GetChannels(), this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    for (channel = 0; channel < m_pointLocations->GetMultiSpectralImage()->GetChannels(); channel++)
    {
        progressDialog.setValue(channel);
        QApplication::processEvents();
        if (progressDialog.wasCanceled()) break;

        gamma = m_pointLocations->GetMultiSpectralImage()->OptimalGamma(channel);
        m_pointLocations->GetMultiSpectralImage()->RefreshPreview(channel, gamma, m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->percentileClip);
        channelLabels.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->label);
        gammas.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->gamma);
        percentiles.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->percentileClip);
        mins.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->min);
        maxs.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->max);
    }
    SetChannelAssignments(channelLabels, gammas, percentiles, mins, maxs);
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel)));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
    update();
    isModified = true;

    progressDialog.hide();
}

void MdiChild::exportImage()
{
    DialogExportImage dialogExportImage(this);
    dialogExportImage.setValues(*m_preferences);

    int status = dialogExportImage.exec();

    if (status == QDialog::Accepted)
    {
        dialogExportImage.getValues(m_preferences);
        QDir dir(m_preferences->ExportFolderPath);
        if (!dir.exists()) dir.mkpath(".");
        QFileInfo fileInfo(curFile);
        QString fileNameRoot = m_preferences->ExportFolderPath + QDir::separator() + fileInfo.completeBaseName();
        ImageExporter imageExporter(m_pointLocations, this);

        int nChannels = m_pointLocations->GetMultiSpectralImage()->GetChannels();
        double *gammaStore = new double[nChannels];
        double *percentileStore = new double[nChannels];
        for (int channel = 0; channel < nChannels; channel ++)
        {
            gammaStore[channel] = m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->gamma;
            percentileStore[channel] = m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->percentileClip;
        }

        if (m_preferences->ExportJPG)
        {
            if (m_preferences->ExportDisplayedPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".jpg", 0, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 0, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".jpg", 0, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 0, 1.5);
            }
            if (m_preferences->Export95thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".jpg", 95, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 95, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".jpg", 95, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 95, 1.5);
            }
            if (m_preferences->Export99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".jpg", 99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".jpg", 99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 99, 1.5);
            }
            if (m_preferences->Export99p9thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".jpg", 99.9, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 99.9, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".jpg", 99.9, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 99.9, 1.5);
            }
            if (m_preferences->Export99p99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".jpg", 99.99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 99.99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".jpg", 99.99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".jpg", 99.99, 1.5);
            }
        }
        if (m_preferences->ExportTIFF)
        {
            if (m_preferences->ExportDisplayedPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".tif", 0, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".tif", 0, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".tif", 0, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".tif", 0, 1.5);
            }
            if (m_preferences->Export95thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".tif", 95, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".tif", 95, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".tif", 95, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".tif", 95, 1.5);
            }
            if (m_preferences->Export99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".tif", 99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".tif", 99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".tif", 99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".tif", 99, 1.5);
            }
            if (m_preferences->Export99p9thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".tif", 99.9, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".tif", 99.9, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".tif", 99.9, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".tif", 99.9, 1.5);
            }
            if (m_preferences->Export99p99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportImage(fileNameRoot + ".tif", 99.99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportImage(fileNameRoot + ".tif", 99.99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportImage(fileNameRoot + ".tif", 99.99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportImage(fileNameRoot + ".tif", 99.99, 1.5);
            }
        }
        if (m_preferences->ExportPDF)
        {
            if (m_preferences->ExportDisplayedPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPDF(fileNameRoot + ".pdf", 0, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 0, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPDF(fileNameRoot + ".pdf", 0, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 0, 1.5);
            }
            if (m_preferences->Export95thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPDF(fileNameRoot + ".pdf", 95, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 95, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPDF(fileNameRoot + ".pdf", 95, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 95, 1.5);
            }
            if (m_preferences->Export99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99, 1.5);
            }
            if (m_preferences->Export99p9thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99.9, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99.9, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99.9, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99.9, 1.5);
            }
            if (m_preferences->Export99p99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99.99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99.99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99.99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPDF(fileNameRoot + ".pdf", 99.99, 1.5);
            }
        }
        if (m_preferences->ExportPNG)
        {
            if (m_preferences->ExportDisplayedPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPNG(fileNameRoot + ".png", 0, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPNG(fileNameRoot + ".png", 0, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPNG(fileNameRoot + ".png", 0, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPNG(fileNameRoot + ".png", 0, 1.5);
            }
            if (m_preferences->Export95thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPNG(fileNameRoot + ".png", 95, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPNG(fileNameRoot + ".png", 95, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPNG(fileNameRoot + ".png", 95, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPNG(fileNameRoot + ".png", 95, 1.5);
            }
            if (m_preferences->Export99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPNG(fileNameRoot + ".png", 99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPNG(fileNameRoot + ".png", 99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPNG(fileNameRoot + ".png", 99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPNG(fileNameRoot + ".png", 99, 1.5);
            }
            if (m_preferences->Export99p9thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPNG(fileNameRoot + ".png", 99.9, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPNG(fileNameRoot + ".png", 99.9, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPNG(fileNameRoot + ".png", 99.9, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPNG(fileNameRoot + ".png", 99.9, 1.5);
            }
            if (m_preferences->Export99p99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportPNG(fileNameRoot + ".png", 99.99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportPNG(fileNameRoot + ".png", 99.99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportPNG(fileNameRoot + ".png", 99.99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportPNG(fileNameRoot + ".png", 99.99, 1.5);
            }
        }
        if (m_preferences->ExportSVG)
        {
            if (m_preferences->ExportDisplayedPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportSVG(fileNameRoot + ".svg", 0, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 0, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportSVG(fileNameRoot + ".svg", 0, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 0, 1.5);
            }
            if (m_preferences->Export95thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportSVG(fileNameRoot + ".svg", 95, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 95, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportSVG(fileNameRoot + ".svg", 95, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 95, 1.5);
            }
            if (m_preferences->Export99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportSVG(fileNameRoot + ".svg", 99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportSVG(fileNameRoot + ".svg", 99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 99, 1.5);
            }
            if (m_preferences->Export99p9thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportSVG(fileNameRoot + ".svg", 99.9, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 99.9, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportSVG(fileNameRoot + ".svg", 99.9, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 99.9, 1.5);
            }
            if (m_preferences->Export99p99thPercentile)
            {
                if (m_preferences->ExportGammaDisplayed) imageExporter.ExportSVG(fileNameRoot + ".svg", 99.99, 0);
                if (m_preferences->ExportGamma0p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 99.99, 0.5);
                if (m_preferences->ExportGamma1) imageExporter.ExportSVG(fileNameRoot + ".svg", 99.99, 1);
                if (m_preferences->ExportGamma1p5) imageExporter.ExportSVG(fileNameRoot + ".svg", 99.99, 1.5);
            }
        }

        for (int channel = 0; channel < nChannels; channel ++)
        {
            m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->gamma = gammaStore[channel];
            m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->percentileClip = percentileStore[channel];
            m_pointLocations->GetMultiSpectralImage()->RefreshPreview(channel, gammaStore[channel], percentileStore[channel]);
        }
        delete [] gammaStore;
        delete [] percentileStore;
    }

//    QFileInfo fileInfo(curFile);
//    QString parentPath = fileInfo.path();
//    QString baseName = fileInfo.completeBaseName();
//    QString suggestedPath = QDir(parentPath).filePath(baseName);
//    QString fileName = QFileDialog::getSaveFileName(this, tr("Export a set of image files"), suggestedPath, "PNG files (*.png);;PDF files (*.pdf);;TIFF files (*.tif);;JPEG files (*.jpg);;SVG files (*.svg)", 0, EXTRA_FILE_DIALOG_OPTIONS);

//    if (fileName.isEmpty()) return;

//    QFileInfo fileInfo2(fileName);
//    QString suffix = fileInfo2.suffix();
//    ImageExporter imageExporter(m_pointLocations, this);
//    if (suffix == "pdf") imageExporter.ExportPDF(fileName);
//    if (suffix == "svg") imageExporter.ExportSVG(fileName);
//    if (suffix == "png") imageExporter.ExportPNG(fileName);
//    if (suffix == "tif") imageExporter.ExportImage(fileName);
//    if (suffix == "jpg") imageExporter.ExportImage(fileName);
}

bool MdiChild::importChannels(const QString &fileName)
{
    PointLocations *pointLocations = new PointLocations();
    int err = pointLocations->Read(fileName);
    if (err)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 reading %2").arg(err).arg(fileName));
        delete pointLocations;
        return false;
    }
    QStringList channelLabels;
    QList<double> gammas, percentiles;
    QList<uint32_t> mins, maxs;
    for (int i = 0; i < m_pointLocations->GetMultiSpectralImage()->GetChannels(); i++)
    {
        m_pointLocations->GetMultiSpectralImage()->SetImageLabel(i, pointLocations->GetMultiSpectralImage()->GetImageData(i)->label);
        channelLabels.append(pointLocations->GetMultiSpectralImage()->GetImageData(i)->label);
        gammas.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->gamma);
        percentiles.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->percentileClip);
        mins.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->min);
        maxs.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->max);
    }
    SetChannelAssignments(channelLabels, gammas, percentiles, mins, maxs);

    delete pointLocations;
    isModified = true;
    update();
    return true; // success
}

bool MdiChild::OpenRASFile()
{
    QProgressDialog progressDialog("Opening RAS File...", QString(), 0, 3, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();
    QApplication::processEvents();

    int i;
    if (m_pointLocations) delete m_pointLocations;
    m_pointLocations = new PointLocations();
    m_pointLocations->SetMultiSpectralImage(new MultiSpectralImage());
    m_pointLocations->GetMultiSpectralImage()->SetDefaultGamma(m_preferences->DefaultGamma);
    m_pointLocations->GetMultiSpectralImage()->SetDefaultPercentileClip(m_preferences->DefaultPercentileClip);
    int err = m_pointLocations->GetMultiSpectralImage()->ReadRASFile(m_RASFileName);
    if (err)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 reading %2").arg(err).arg(m_RASFileName));
        delete m_pointLocations->GetMultiSpectralImage();
        m_pointLocations->SetMultiSpectralImage(0);
        return false;
    }
    for (i = 0; i < m_pointLocations->GetMultiSpectralImage()->GetChannels(); i++)
    {
        if (m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->max - m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->min > 0)
        {
            m_currentChannel = i;
            break;
        }
    }

    progressDialog.setValue(1);
    QApplication::processEvents();

    m_pointLocations->GetMultiSpectralImage()->CreatePreviews();
    m_preferences->LastOpenedRASFile = m_RASFileName;

    progressDialog.setValue(2);
    QApplication::processEvents();

    QImage *preview = m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel);
    if (preview == 0)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 generating preview %2").arg(__LINE__).arg(m_RASFileName));
        return false;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*preview));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());

    m_pointLocations->SetOrnaments(m_preferences->ScaleBarLength, m_preferences->ScaleBarLocation, m_preferences->ScaleBarUnits, m_preferences->ScaleBarFontSize, m_preferences->MarkerFontSize);
    m_pointLocations->SetScaleBarColor(m_preferences->ScaleBarColor);
    m_pointLocations->SetMarkerColor(m_preferences->MarkerColor);
    const QString Color_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");
    ui->pushButtonMarkerColor->setStyleSheet(Color_STYLE.arg(m_preferences->MarkerColor.name()).arg(PreferencesDialog::getIdealTextColor(m_preferences->MarkerColor).name()).arg(PreferencesDialog::getAlphaColorHint(m_preferences->MarkerColor).name()));
    ui->pushButtonScaleBarColor->setStyleSheet(Color_STYLE.arg(m_preferences->ScaleBarColor.name()).arg(PreferencesDialog::getIdealTextColor(m_preferences->ScaleBarColor).name()).arg(PreferencesDialog::getAlphaColorHint(m_preferences->ScaleBarColor).name()));

    ImageData *imageData = m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel);
    if (imageData == 0)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 accessing image data %2").arg(__LINE__).arg(m_RASFileName));
        return false;
    }
    ClearPoints();
    SetScaleBar(m_preferences->ScaleBarLength, m_preferences->ScaleBarLocation, m_preferences->ScaleBarUnits, m_preferences->ScaleBarFontSize, m_preferences->MarkerFontSize);
    QPointF topLeft, bottomRight;
    m_pointLocations->GetCalibrations(&topLeft, &bottomRight);
    SetImageCorners(topLeft, bottomRight, imageData->width, imageData->height);

    QStringList channelLabels;
    QList<double> gammas, percentiles;
    QList<uint32_t> mins, maxs;
    for (i = 0; i < m_pointLocations->GetMultiSpectralImage()->GetChannels(); i++)
    {
        channelLabels.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->label);
        gammas.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->gamma);
        percentiles.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->percentileClip);
        mins.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->min);
        maxs.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->max);
    }
    SetChannelAssignments(channelLabels, gammas, percentiles, mins, maxs);

    ScaleBarUpdate();

    progressDialog.setValue(2);
    QApplication::processEvents();
    progressDialog.hide();

    CalculateClicked();
    ui->tableWidgetChannelAssignments->selectRow(m_currentChannel);

    ui->checkBoxApplyOffset->setChecked(m_pointLocations->GetMultiSpectralImage()->GetApplyLineOffset());
    ui->spinBoxLineOffset->setValue(m_pointLocations->GetMultiSpectralImage()->GetLineOffset());

    ui->spinBoxRowPairs->setValue(MIN(int(m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel)->height / 2), m_preferences->NumberOfRowPairs));
    ui->spinBoxNumberOffsets->setValue(m_preferences->NumberOfOffsets);

    update();
    return true;
}

bool MdiChild::OpenPointLocationsFile()
{
    QProgressDialog progressDialog("Opening Point Location File...", QString(), 0, 4, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();
    QApplication::processEvents();
    int i;

    if (m_pointLocations) delete m_pointLocations;
    m_pointLocations = new PointLocations();
    int err = m_pointLocations->Read(curFile);
    if (err)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 reading %2").arg(err).arg(curFile));
        delete m_pointLocations;
        m_pointLocations = 0;
        return false;
    }
    for (i = 0; i < m_pointLocations->GetMultiSpectralImage()->GetChannels(); i++)
    {
        if (m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->max - m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->min > 0)
        {
            m_currentChannel = i;
            break;
        }
    }
    progressDialog.setValue(1);
    QApplication::processEvents();

    m_pointLocations->GetMultiSpectralImage()->CreatePreviews();
    m_RASFileName = m_pointLocations->GetMultiSpectralImage()->GetRASFileName();
    m_preferences->LastOpenedRASFile = m_RASFileName;
    m_preferences->LastOpenedPointLocationsFile = curFile;

    progressDialog.setValue(2);
    QApplication::processEvents();

    m_graphicsScene->clear();
    m_graphicsPixmapItem = 0;
    m_graphicsItemScaleBar = 0;

    QImage *preview = m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel);
    if (preview == 0)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 generating preview %2").arg(__LINE__).arg(m_RASFileName));
        return false;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*preview));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());

    progressDialog.setValue(3);
    QApplication::processEvents();

    QPointF topLeft, bottomRight;
    ImageData *imageData = m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel);
    if (imageData == 0)
    {
        QMessageBox::warning(this, "Warning", QString("Error %1 accessing image data %2").arg(__LINE__).arg(m_RASFileName));
        return false;
    }
    m_pointLocations->GetCalibrations(&topLeft, &bottomRight);
    SetImageCorners(topLeft, bottomRight, imageData->width, imageData->height);

    RecalculatePoints();

    QStringList channelLabels;
    QList<double> gammas, percentiles;
    QList<uint32_t> mins, maxs;
    for (i = 0; i < m_pointLocations->GetMultiSpectralImage()->GetChannels(); i++)
    {
        channelLabels.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->label);
        gammas.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->gamma);
        percentiles.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->percentileClip);
        mins.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->min);
        maxs.append(m_pointLocations->GetMultiSpectralImage()->GetImageData(i)->max);
    }
    SetChannelAssignments(channelLabels, gammas, percentiles, mins, maxs);

    QPoint scaleBarLocation;
    double scaleBarLength;
    QString units;
    int scaleBarFontSize, markerFontSize;
    m_pointLocations->GetOrnaments(&scaleBarLength, &scaleBarLocation, &units, &scaleBarFontSize, &markerFontSize);
    const QString Color_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");
    ui->pushButtonMarkerColor->setStyleSheet(Color_STYLE.arg(m_pointLocations->GetMarkerColor().name()).arg(PreferencesDialog::getIdealTextColor(m_pointLocations->GetMarkerColor()).name()).arg(PreferencesDialog::getAlphaColorHint(m_pointLocations->GetMarkerColor()).name()));
    ui->pushButtonScaleBarColor->setStyleSheet(Color_STYLE.arg(m_pointLocations->GetScaleBarColor().name()).arg(PreferencesDialog::getIdealTextColor(m_pointLocations->GetScaleBarColor()).name()).arg(PreferencesDialog::getAlphaColorHint(m_pointLocations->GetScaleBarColor()).name()));
    SetScaleBar(scaleBarLength, scaleBarLocation, units, scaleBarFontSize, markerFontSize);
    ScaleBarUpdate();

    progressDialog.setValue(3);
    QApplication::processEvents();
    progressDialog.hide();

    CalculateClicked();
    ui->tableWidgetChannelAssignments->selectRow(m_currentChannel);

    ui->checkBoxApplyOffset->setChecked(m_pointLocations->GetMultiSpectralImage()->GetApplyLineOffset());
    ui->spinBoxLineOffset->setValue(m_pointLocations->GetMultiSpectralImage()->GetLineOffset());

    ui->spinBoxRowPairs->setValue(MIN(int(m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel)->height / 2), m_preferences->NumberOfRowPairs));
    ui->spinBoxNumberOffsets->setValue(m_preferences->NumberOfOffsets);

    update();
    isModified = false;
    return true;
}

void MdiChild::NewMappedLocation(const QPointF &mappedLocation)
{
    QPoint p(int(mappedLocation.x()), int(mappedLocation.y()));
    QString newName = QString("Point (%1,%2)").arg(p.x()).arg(p.y());

    DialogNewPoint dialogNewPoint(this);
    dialogNewPoint.SetName(newName);

    int status = dialogNewPoint.exec();

    if (status == QDialog::Accepted)
    {
        newName = dialogNewPoint.GetName();
        if (m_pointLocations->PointLocationExists(newName))
        {
            QMessageBox::StandardButton reply = QMessageBox::question(this, QApplication::applicationName(),
                                                                      QString("%1 exists. Do you wish to replace it?").arg(newName),
                                                                      QMessageBox::No | QMessageBox::Yes);
            if (reply == QMessageBox::No) return;
        }
        QPointF topLeft, bottomRight;
        topLeft.setX(ui->doubleSpinBoxXMin->value());
        topLeft.setY(ui->doubleSpinBoxYMin->value());
        bottomRight.setX(ui->doubleSpinBoxXMax->value());
        bottomRight.setY(ui->doubleSpinBoxYMax->value());
        m_pointLocations->SetCalibrations(topLeft, bottomRight);
        m_pointLocations->AddPoint(newName, p);
        AddPoint(newName, p);
        isModified = true;

        GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(newName, m_pointLocations->GetMarkerFontSize(), m_pointLocations->GetMarkerColor());
        m_graphicsItemAnalysisLocations.append(newLocation);
        newLocation->setPos(QPointF(p.x() + 0.5, p.y() + 0.5));
        newLocation->setZValue(1);
        m_graphicsScene->addItem(newLocation);
        update();
    }
}

void MdiChild::CalibrationUpdate()
{
    QPointF topLeft, bottomRight;
    ImageData *imageData = m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel);
    Q_ASSERT(imageData);
    topLeft.setX(ui->doubleSpinBoxXMin->value());
    topLeft.setY(ui->doubleSpinBoxYMin->value());
    bottomRight.setX(ui->doubleSpinBoxXMax->value());
    bottomRight.setY(ui->doubleSpinBoxYMax->value());
    SetImageCorners(topLeft, bottomRight, imageData->width, imageData->height);
    m_pointLocations->SetCalibrations(topLeft, bottomRight);
    isModified = true;
    RecalculatePoints();
    ScaleBarUpdate();
    update();
}

void MdiChild::NewGamma(int channel, double gamma)
{
    if (fabs(m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->gamma - gamma) > 0.00001)
    {
        m_pointLocations->GetMultiSpectralImage()->RefreshPreview(channel, gamma, m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->percentileClip);
        if (m_graphicsPixmapItem)
        {
            m_graphicsScene->removeItem(m_graphicsPixmapItem);
            delete m_graphicsPixmapItem;
        }
        m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel)));
        m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
        isModified = true;
        update();
    }
}

void MdiChild::NewPercentile(int channel, double percentile)
{
    if (fabs(m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->percentileClip - percentile) > 0.000001)
    {
        m_pointLocations->GetMultiSpectralImage()->RefreshPreview(channel, m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->gamma, percentile);
        if (m_graphicsPixmapItem)
        {
            m_graphicsScene->removeItem(m_graphicsPixmapItem);
            delete m_graphicsPixmapItem;
        }
        m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel)));
        m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());
        isModified = true;
        update();
    }
}

void MdiChild::NewChannelAssigment(int channel, const QString &label)
{
    if (m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->label != label)
    {
        m_pointLocations->GetMultiSpectralImage()->SetImageLabel(channel, label);
        isModified = true;
        update();
    }
}

void MdiChild::ScaleBarUpdate()
{
    if (m_graphicsItemScaleBar)
    {
        m_graphicsScene->removeItem(m_graphicsItemScaleBar);
        delete m_graphicsItemScaleBar;
    }
    QPointF topLeft(ui->doubleSpinBoxXMin->value(), ui->doubleSpinBoxYMin->value());
    QPointF bottomRight(ui->doubleSpinBoxXMax->value(), ui->doubleSpinBoxYMax->value());
    QPoint scaleBarLocation(ui->spinBoxScaleBarPosX->value(), ui->spinBoxScaleBarPosY->value());
    double scaleBarLength = ui->doubleSpinBoxScaleBarLength->value();
    QString scaleBarUnits = ui->lineEditScaleBarUnits->text();
    int scaleBarFontSize = ui->spinBoxScaleBarFontSize->value();
    int markerFontSize = ui->spinBoxMarkerFontSize->value();

    ImageData *imageData = m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel);
    double pixelSize = (scaleBarLength / (bottomRight.x() - topLeft.x())) * imageData->width;
    QString scaleBarLabel = QString("%1 %2").arg(scaleBarLength).arg(scaleBarUnits);
    m_graphicsItemScaleBar = new GraphicsItemScaleBar(pixelSize, scaleBarLabel, ScaleBarRectangle, scaleBarFontSize, m_pointLocations->GetScaleBarColor());
    m_graphicsItemScaleBar->setPos(QPointF(scaleBarLocation.x(), scaleBarLocation.y()));
    m_graphicsItemScaleBar->setZValue(1);
    m_graphicsScene->addItem(m_graphicsItemScaleBar);

    m_pointLocations->SetOrnaments(scaleBarLength, scaleBarLocation, scaleBarUnits, scaleBarFontSize, markerFontSize);
    isModified = true;
}

void MdiChild::NewChannelSelected(int channel)
{
    m_currentChannel = channel;
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel)));
    CalculateClicked();
    update();
}

void MdiChild::SetScaleBar(double scaleBarLength, const QPoint &scaleBarLocation, const QString &scaleBarUnits, int scaleBarFontSize, int markerFontSize)
{
    BlockAllSignals(true);

    ui->doubleSpinBoxScaleBarLength->setValue(scaleBarLength);
    ui->spinBoxScaleBarPosX->setValue(scaleBarLocation.x());
    ui->spinBoxScaleBarPosY->setValue(scaleBarLocation.y());
    ui->lineEditScaleBarUnits->setText(scaleBarUnits);
    ui->spinBoxScaleBarFontSize->setValue(scaleBarFontSize);
    ui->spinBoxMarkerFontSize->setValue(markerFontSize);

    BlockAllSignals(false);
}

void MdiChild::ClearPoints()
{
    BlockAllSignals(true);

    ui->tableWidgetPointCountLocations->clear();
    ui->tableWidgetPointCountLocations->setRowCount(0);
    ui->tableWidgetPointCountLocations->setColumnCount(5);
    QStringList labels;
    labels << "Name" << "XPix" << "YPix" << "Xmm" << "Ymm";
    ui->tableWidgetPointCountLocations->setHorizontalHeaderLabels(labels);
    ui->tableWidgetPointCountLocations->resizeColumnsToContents();
    ui->tableWidgetPointCountLocations->resizeRowsToContents();
    for (int i = 0; i < m_graphicsItemAnalysisLocations.size(); i++)
    {
        m_graphicsScene->removeItem(m_graphicsItemAnalysisLocations[i]);
        delete m_graphicsItemAnalysisLocations[i];
    }
    m_graphicsItemAnalysisLocations.clear();

    BlockAllSignals(false);
}

void MdiChild::RecalculatePoints()
{
    ClearPoints();
    QList<NamedPoint> *pointList = m_pointLocations->GetPointList();
    QList<NamedPoint>::const_iterator it = pointList->constBegin();
    m_pointLocations->SetMarkerFontSize(ui->spinBoxMarkerFontSize->value());
    while (it != pointList->constEnd())
    {
        AddPoint(it->name, it->point);
        GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(it->name, m_pointLocations->GetMarkerFontSize(), m_pointLocations->GetMarkerColor());
        m_graphicsItemAnalysisLocations.append(newLocation);
        newLocation->setPos(QPointF(it->point.x() + 0.5f, it->point.y() + 0.5f));
        newLocation->setZValue(1);
        m_graphicsScene->addItem(newLocation);
        ++it;
    }
    isModified = true;
}

void MdiChild::AddPoint(const QString name, const QPoint &point)
{
    BlockAllSignals(true);

    int row = ui->tableWidgetPointCountLocations->rowCount();
    int i;
    QPointF scanPos = m_pointLocations->CalculateScanPosition(point);
    QTableWidgetItem *item;
    for (i = 0; i < row; i++)
    {
        item = ui->tableWidgetPointCountLocations->item(i, 0);
        if (item->text() == name)
        {
            ui->tableWidgetPointCountLocations->item(i, 1)->setText(QString("%1").arg(point.x()));
            ui->tableWidgetPointCountLocations->item(i, 2)->setText(QString("%1").arg(point.y()));
            ui->tableWidgetPointCountLocations->item(i, 3)->setText(QString("%1").arg(scanPos.x(), 0, 'f', 5));
            ui->tableWidgetPointCountLocations->item(i, 4)->setText(QString("%1").arg(scanPos.y(), 0, 'f', 5));
            return;
        }
    }

    ui->tableWidgetPointCountLocations->insertRow(row);

    QTableWidgetItem *xPos = new QTableWidgetItem(QString("%1").arg(point.x()));
    QTableWidgetItem *yPos = new QTableWidgetItem(QString("%1").arg(point.y()));
    QTableWidgetItem *xPosCal = new QTableWidgetItem(QString("%1").arg(scanPos.x(), 0, 'f', 5));
    QTableWidgetItem *yPosCal = new QTableWidgetItem(QString("%1").arg(scanPos.y(), 0, 'f', 5));
    QTableWidgetItem *nameItem = new QTableWidgetItem(name);
    //xPos->setFlags(xPos->flags() & ~Qt::ItemIsEditable);
    //yPos->setFlags(yPos->flags() & ~Qt::ItemIsEditable);
    //xPosCal->setFlags(xPosCal->flags() & ~Qt::ItemIsEditable);
    //yPosCal->setFlags(yPosCal->flags() & ~Qt::ItemIsEditable);
    //nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    ui->tableWidgetPointCountLocations->setItem(row, 0, nameItem);
    ui->tableWidgetPointCountLocations->setItem(row, 1, xPos);
    ui->tableWidgetPointCountLocations->setItem(row, 2, yPos);
    ui->tableWidgetPointCountLocations->setItem(row, 3, xPosCal);
    ui->tableWidgetPointCountLocations->setItem(row, 4, yPosCal);

    ui->tableWidgetPointCountLocations->selectRow(row);
    ui->tableWidgetPointCountLocations->scrollToItem(nameItem);


    BlockAllSignals(false);
    ui->tableWidgetPointCountLocations->resizeColumnsToContents();
    ui->tableWidgetPointCountLocations->resizeRowsToContents();
}

void MdiChild::SetImageCorners(const QPointF &topLeft, const QPointF &bottomRight, int width, int height)
{
    BlockAllSignals(true);

    ui->doubleSpinBoxXMin->setValue(topLeft.x());
    ui->doubleSpinBoxYMin->setValue(topLeft.y());
    ui->doubleSpinBoxXMax->setValue(bottomRight.x());
    ui->doubleSpinBoxYMax->setValue(bottomRight.y());

    ImageData *imageData = m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel);
    double widthResolution = (bottomRight.x() - topLeft.x()) / (imageData->width - 1);
    double heightResolution = (bottomRight.y() - topLeft.y()) / (imageData->height - 1);
    ui->labelImageSize->setText(QString("%1 x %2").arg(width).arg(height));
    ui->labelImageResolution->setText(QString("%1 x %2").arg(widthResolution, 0, 'f', 5).arg(heightResolution, 0, 'f', 5));

    BlockAllSignals(false);
}

void MdiChild::SetChannelAssignments(const QStringList &channelLabels, const QList<double> &gammas, const QList<double> &percentiles, const QList<uint32_t> &mins, const QList<uint32_t> &maxs)
{
    BlockAllSignals(true);

    ui->tableWidgetChannelAssignments->clear();
    ui->tableWidgetChannelAssignments->setRowCount(channelLabels.size());
    ui->tableWidgetChannelAssignments->setColumnCount(6);
    QStringList labels;
    labels << "Channel" << "Label" << "Percentiles" << "Gamma" << "Min" << "Max";
    ui->tableWidgetChannelAssignments->setHorizontalHeaderLabels(labels);

    for (int i = 0; i < channelLabels.size(); i++)
    {
        QTableWidgetItem *channel = new QTableWidgetItem(QString("%1").arg(i));
        QTableWidgetItem *label = new QTableWidgetItem(channelLabels[i]);
        QTableWidgetItem *percentile = new QTableWidgetItem(QString("%1").arg(percentiles[i], 0, 'f', 6));
        QTableWidgetItem *gamma = new QTableWidgetItem(QString("%1").arg(gammas[i], 0, 'f', 5));
        QTableWidgetItem *minVal = new QTableWidgetItem(QString("%1").arg(mins[i]));
        QTableWidgetItem *maxVal = new QTableWidgetItem(QString("%1").arg(maxs[i]));
        channel->setFlags(channel->flags() & ~Qt::ItemIsEditable);
        channel->setFlags(minVal->flags() & ~Qt::ItemIsEditable);
        channel->setFlags(maxVal->flags() & ~Qt::ItemIsEditable);
        ui->tableWidgetChannelAssignments->setItem(i, 0, channel);
        ui->tableWidgetChannelAssignments->setItem(i, 1, label);
        ui->tableWidgetChannelAssignments->setItem(i, 2, percentile);
        ui->tableWidgetChannelAssignments->setItem(i, 3, gamma);
        ui->tableWidgetChannelAssignments->setItem(i, 4, minVal);
        ui->tableWidgetChannelAssignments->setItem(i, 5, maxVal);
    }
    ui->tableWidgetChannelAssignments->resizeColumnsToContents();
    ui->tableWidgetChannelAssignments->resizeRowsToContents();

    BlockAllSignals(false);
}

void MdiChild::ChannelAssignmentsCellChanged(int row, int col)
{
    QTableWidgetItem *item;
    double v;
    if (col == 1)
    {
        item = ui->tableWidgetChannelAssignments->item(row, col);
        QString label(item->text());
        NewChannelAssigment(row, label);
    }
    if (col == 2)
    {
        item = ui->tableWidgetChannelAssignments->item(row, col);
        v = item->text().toDouble();
        if (v < 0) v = 0;
        if (v > 100) v = 100;
        NewPercentile(row, v);
    }
    if (col == 3)
    {
        item = ui->tableWidgetChannelAssignments->item(row, col);
        v = item->text().toDouble();
        if (v < 0) v = 0;
        if (v > 99999) v = 99999;
        NewGamma(row, v);
    }
    ui->tableWidgetChannelAssignments->resizeColumnsToContents();
    ui->tableWidgetChannelAssignments->resizeRowsToContents();
}

// this is a very blunt tool
// it causes difficulties becasue there are hidden signals that
// handle scrolling and other features and they can get broken by this
void MdiChild::BlockAllSignals(bool flag)
{
    QList<QWidget *> widgets = findChildren<QWidget *>();
    for (int i = 0; i < widgets.size(); i++)
    {
        widgets[i]->blockSignals(flag);
    }
}

void MdiChild::CalculateClicked()
{
    int i, j, n;
    ImageData *imageData = m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel);
    if (imageData->min == imageData->max) return; // nothing to display

    uint32_t *x, *y;
    int numOffsets = ui->spinBoxNumberOffsets->value();
    int listSize = numOffsets * 2 + 1;
    double *rList = new double[listSize];
    double rSum;
    int index;
    bool meanRegression = ui->checkBoxMean->checkState();

    QRect targetRect(m_graphicsViewLineCorrelationChart->rect());
#ifdef Q_OS_WIN
    m_graphicsSceneLineCorrelationChart->setSceneRect(0, 0, targetRect.width() - 16, targetRect.height() - 16); // set to -16 to prevent scroll bars on windows
#else
    m_graphicsSceneLineCorrelationChart->setSceneRect(0, 0, targetRect.width() - 2, targetRect.height() - 2); // set to -2 to prevent scroll bars
#endif
    m_graphicsSceneLineCorrelationChart->clear();
    QRectF limits = m_graphicsSceneLineCorrelationChart->sceneRect();
    QLineF line;
    QRectF srcCoords(QPointF(0, 1), QPointF(listSize, 0));
    QRectF dstCoords = limits;

    ui->spinBoxRowPairs->setMaximum(imageData->height / 2);
    int numRowPairs = ui->spinBoxRowPairs->value();
    ui->spinBoxLineOffset->setMinimum(-numOffsets);
    ui->spinBoxLineOffset->setMaximum(numOffsets);
    int offset = ui->spinBoxLineOffset->value();

    n = imageData->width;
    if (meanRegression == false)
    {
        for (i = 0; i < numRowPairs; i++)
        {
            index = 0;
            for (j = -numOffsets; j <= numOffsets; j++)
            {
                x = imageData->data + i * 2 * n + numOffsets;
                y = x + n + j;
                rList[index] = PearsonsProductMomentCorrelationCoefficient(x, y, n - listSize);
                ++index;
            }
            for (j = 0; j < listSize - 1; j++)
            {
                line.setP1(TransformCoordinateSystem(QPointF(j, rList[j]), srcCoords, dstCoords));
                line.setP2(TransformCoordinateSystem(QPointF(j + 1, rList[j + 1]), srcCoords, dstCoords));
                m_graphicsSceneLineCorrelationChart->addLine(line);
            }
        }
    }
    else
    {
        index = 0;
        for (j = -numOffsets; j <= numOffsets; j++)
        {
            rSum = 0;
            for (i = 0; i < numRowPairs; i++)
            {
                x = imageData->data + i * 2 * n + numOffsets;
                y = x + n + j;
                rSum += PearsonsProductMomentCorrelationCoefficient(x, y, n - listSize);
            }
            rList[index] = rSum / numRowPairs;
            ++index;
        }
        for (j = 0; j < listSize - 1; j++)
        {
            line.setP1(TransformCoordinateSystem(QPointF(j, rList[j]), srcCoords, dstCoords));
            line.setP2(TransformCoordinateSystem(QPointF(j + 1, rList[j + 1]), srcCoords, dstCoords));
            m_graphicsSceneLineCorrelationChart->addLine(line);
        }
    }
    // zero line
    line.setP1(TransformCoordinateSystem(QPointF(numOffsets, 0), srcCoords, dstCoords));
    line.setP2(TransformCoordinateSystem(QPointF(numOffsets, 1), srcCoords, dstCoords));
    m_graphicsSceneLineCorrelationChart->addLine(line, QPen(QColor(255, 0, 0)));
    // offset line
    if (offset != 0)
    {
        line.setP1(TransformCoordinateSystem(QPointF(numOffsets + offset, 0), srcCoords, dstCoords));
        line.setP2(TransformCoordinateSystem(QPointF(numOffsets + offset, 1), srcCoords, dstCoords));
        m_graphicsSceneLineCorrelationChart->addLine(line, QPen(QColor(0, 255, 0)));
    }
    update();
}

QPointF MdiChild::TransformCoordinateSystem(const QPointF &srcPoint, const QRectF &srcCoords, const QRectF &dstCoords)
{
    QPointF dstPoint;
    dstPoint.setX( ( (srcPoint.x() - srcCoords.left()) / (srcCoords.right() - srcCoords.left()) ) * (dstCoords.right() - dstCoords.left()) + dstCoords.left() );
    dstPoint.setY( ( (srcPoint.y() - srcCoords.top()) / (srcCoords.bottom() - srcCoords.top()) ) * (dstCoords.bottom() - dstCoords.top()) + dstCoords.top() );
    return dstPoint;
}

double MdiChild::PearsonsProductMomentCorrelationCoefficient(const uint32_t *x, const uint32_t *y, int n)
{
    double xy_sum = 0;
    double x_sum = 0;
    double y_sum = 0;
    double xx_sum = 0;
    double yy_sum = 0;
    for (int i = 0; i < n; i++)
    {
        x_sum += *x;
        y_sum += *y;
        xy_sum += double(*x) * double(*y);
        xx_sum += double(*x) * double(*x);
        yy_sum += double(*y) * double(*y);
        ++x;
        ++y;
    }
    double sxy = xy_sum - (x_sum * y_sum / n);
    double sxx = xx_sum - (x_sum * x_sum / n);
    double syy = yy_sum - (y_sum * y_sum / n);
    if (fabs(sxx) < 1e-10 || fabs(syy) < 1e-10) return 0;
    double r = sxy / sqrt(sxx * syy);
    return r;
}

void MdiChild::markerColorClicked()
{
    const QString Color_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");

#ifdef Q_OS_MAC
    QColor Color = QColorDialog::getColor(m_pointLocations->GetMarkerColor(), this, "Select Color", QColorDialog::ShowAlphaChannel);
#else
    QColor Color = QColorDialog::getColor(m_pointLocations->GetMarkerColor(), this, "Select Color", QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
#endif
    if (Color.isValid())
    {
        ui->pushButtonMarkerColor->setStyleSheet(Color_STYLE.arg(Color.name()).arg(PreferencesDialog::getIdealTextColor(Color).name()).arg(PreferencesDialog::getAlphaColorHint(Color).name()));
        m_pointLocations->SetMarkerColor(Color);
        RecalculatePoints();
    }
}

void MdiChild::scaleBarColorClicked()
{
    const QString Color_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");

#ifdef Q_OS_MAC
    QColor Color = QColorDialog::getColor(m_pointLocations->GetScaleBarColor(), this, "Select Color", QColorDialog::ShowAlphaChannel);
#else
    QColor Color = QColorDialog::getColor(m_pointLocations->GetScaleBarColor(), this, "Select Color", QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
#endif
    if (Color.isValid())
    {
        ui->pushButtonScaleBarColor->setStyleSheet(Color_STYLE.arg(Color.name()).arg(PreferencesDialog::getIdealTextColor(Color).name()).arg(PreferencesDialog::getAlphaColorHint(Color).name()));
        m_pointLocations->SetScaleBarColor(Color);
        ScaleBarUpdate();
    }
}

void MdiChild::lineOffsetValueClicked()
{
    CalculateClicked();
    if (ui->checkBoxApplyOffset->isChecked()) applyOffsetClicked();
}

void MdiChild::maxPairsButtonClicked()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->spinBoxRowPairs->setValue(int(m_pointLocations->GetMultiSpectralImage()->GetImageData(m_currentChannel)->height / 2));
    CalculateClicked();
    QApplication::restoreOverrideCursor();
}

void MdiChild::applyOffsetClicked()
{
    m_pointLocations->GetMultiSpectralImage()->SetApplyLineOffset(ui->checkBoxApplyOffset->isChecked());
    m_pointLocations->GetMultiSpectralImage()->SetLineOffset(ui->spinBoxLineOffset->value());
    QProgressDialog progressDialog("Processing Channels...", QString(), 0, m_pointLocations->GetMultiSpectralImage()->GetChannels(), this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();
    QApplication::processEvents();

    for (int channel = 0 ; channel < m_pointLocations->GetMultiSpectralImage()->GetChannels(); channel++)
    {
        m_pointLocations->GetMultiSpectralImage()->RefreshPreview(channel, m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->gamma, m_pointLocations->GetMultiSpectralImage()->GetImageData(channel)->percentileClip);
        progressDialog.setValue(channel + 1);
        QApplication::processEvents();
    }
    if (m_graphicsPixmapItem)
    {
        m_graphicsScene->removeItem(m_graphicsPixmapItem);
        delete m_graphicsPixmapItem;
    }
    m_graphicsPixmapItem = m_graphicsScene->addPixmap(QPixmap::fromImage(*m_pointLocations->GetMultiSpectralImage()->GetPreview(m_currentChannel)));
    m_graphicsScene->setSceneRect(m_graphicsPixmapItem->boundingRect());

    progressDialog.hide();
    isModified = true;
    update();
}

void MdiChild::menuRequestPointCountLocations(const QPoint &p)
{
    QMenu menu(this);
    menu.addAction(tr("Delete"));

    QPoint gp = ui->tableWidgetPointCountLocations->mapToGlobal(p);
    QAction *action = menu.exec(gp);

    if (action)
    {
        if (action->text() == tr("Delete"))
        {
            int row = ui->tableWidgetPointCountLocations->currentRow();
            QTableWidgetItem *item = ui->tableWidgetPointCountLocations->item(row, 0);
            QString name = item->text();
            m_pointLocations->DeletePoint(name);
            RecalculatePoints();
            isModified = true;
            update();
        }
    }
}

void MdiChild::PointLocationsCellChanged(int row, int col)
{
    // "Name" << "XPix" << "YPix" << "Xmm" << "Ymm";
    QList<NamedPoint> *points = m_pointLocations->GetPointList();
    QTableWidgetItem *item = ui->tableWidgetPointCountLocations->item(row, col);
    QPoint p;
    switch (col)
    {
    case 0:
        if (item->text().size() != 0) (*points)[row].name = item->text();
        break;
    case 1:
        (*points)[row].point.setX(item->text().toInt());
        break;
    case 2:
        (*points)[row].point.setY(item->text().toInt());
        break;
    case 3:
        p = m_pointLocations->CalculateImagePosition(QPointF(item->text().toDouble(), 0));
        (*points)[row].point.setX(p.x());
        break;
    case 4:
        p = m_pointLocations->CalculateImagePosition(QPointF(0, item->text().toDouble()));
        (*points)[row].point.setY(p.y());
        break;
    }
    RecalculatePoints();
}

