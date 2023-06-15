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

#ifndef MDICHILD_H
#define MDICHILD_H

#include <QDialog>
#include <QList>
#include <QPointF>
#include <QStringList>
#include <QTextCursor>
#include <QMainWindow>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPointF>

#include "MultiSpectralImage.h"
#include "Preferences.h"
#include "GraphicsView.h"
#include "PointLocations.h"
#include "GraphicsItemScaleBar.h"
#include "GraphicsViewLineCorrelationChart.h"
#include "GraphicsItemAnalysisLocation.h"

namespace Ui {
class MdiChild;
}

class MdiChild : public QDialog
{
    Q_OBJECT

public:
    explicit MdiChild(QWidget *parent = 0);
    ~MdiChild();

    void newFile();
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString &fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }
    bool importChannels(const QString &fileName);

    void cut() {}
    void copy() {}
    void paste() {}

    QTextCursor textCursor() { return QTextCursor(); }

    void AddPoint(const QString name, const QPoint &point);
    void ClearPoints();
    void BlockAllSignals(bool flag);

    void SetRASFileName(const QString &RASFileName) { m_RASFileName = RASFileName; }
    void SetImageCorners(const QPointF &topLeft, const QPointF &bottomRight, int width, int height);
    void SetChannelAssignments(const QStringList &channelLabels, const QList<double> &gammas, const QList<double> &percentiles, const QList<uint32_t> &mins, const QList<uint32_t> &maxs);
    void SetScaleBar(double scaleBarLength, const QPoint &scaleBarLocation, const QString &scaleBarUnits, int scaleBarFontSize, int markerFontSize);
    void SetPreferences(Preferences *preferences) { m_preferences = preferences; }
    Preferences *GetPreferences() { return m_preferences; }

    bool OpenRASFile();
    bool OpenPointLocationsFile();
    void SavePointLocationsFile();

    void UpdateCalculation(const ImageData &imageData);
    static double PearsonsProductMomentCorrelationCoefficient(const uint32_t *x, const uint32_t *y, int n);
    static QPointF TransformCoordinateSystem(const QPointF &srcPoint, const QRectF &srcCoords, const QRectF &dstCoords);

public slots:
    void zoomIn();
    void zoomOut();
    void nextChannel();
    void prevChannel();
    void autoGamma();
    void exportImage();

    void CalibrationUpdate();
    void NewMappedLocation(const QPointF &mappedLocation);
    void NewGamma(int channel, double gamma);
    void NewChannelAssigment(int channel, const QString &label);
    void NewPercentile(int channel, double percentile);
    void ScaleBarUpdate();
    void NewChannelSelected(int channel);
    void ChannelAssignmentsCellChanged(int row, int col);
    void PointLocationsCellChanged(int row, int col);
    void CalculateClicked();
    void RecalculatePoints();
    void markerColorClicked();
    void scaleBarColorClicked();
    void applyOffsetClicked();
    void lineOffsetValueClicked();
    void maxPairsButtonClicked();
    void menuRequestPointCountLocations(const QPoint &p);



signals:
    void copyAvailable(bool);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void documentWasModified();

private:
    Ui::MdiChild *ui;

    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    QString curFile;
    bool isUntitled;

    GraphicsView *m_graphicsView;
    QGraphicsScene *m_graphicsScene;
    PointLocations *m_pointLocations;

    QGraphicsPixmapItem *m_graphicsPixmapItem;
    GraphicsItemScaleBar *m_graphicsItemScaleBar;
    QList <GraphicsItemAnalysisLocation *> m_graphicsItemAnalysisLocations;

    QGraphicsScene *m_graphicsSceneLineCorrelationChart;
    GraphicsViewLineCorrelationChart *m_graphicsViewLineCorrelationChart;

    QString m_RASFileName;

    int m_currentZoomLevel;
    int m_currentChannel;
    bool isModified;

    Preferences *m_preferences;
};

#endif
