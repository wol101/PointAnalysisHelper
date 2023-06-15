#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QString>
#include <QByteArray>
#include <QSize>
#include <QPoint>
#include <QColor>

class Preferences
{
public:
    Preferences();

    void Read();
    void Write();

    QString Organisation;
    QString Application;
    QSize MainWindowSize;
    QPoint MainWindowPosition;
    QString LastOpenedRASFile;
    QString LastOpenedPointLocationsFile;
    QString ScaleBarUnits;
    double ScaleBarLength;
    QPoint ScaleBarLocation;
    int ScaleBarFontSize;
    int MarkerFontSize;
    double DefaultGamma;
    double DefaultPercentileClip;
    QColor MarkerColor;
    QColor ScaleBarColor;
    int NumberOfOffsets;
    int NumberOfRowPairs;
    bool Export100thPercentile;
    bool Export95thPercentile;
    bool Export99p9thPercentile;
    bool Export99p99thPercentile;
    bool Export99thPercentile;
    bool ExportDisplayedPercentile;
    bool ExportGamma0p5;
    bool ExportGamma1;
    bool ExportGamma1p5;
    bool ExportGammaDisplayed;
    bool ExportJPG;
    bool ExportTIFF;
    bool ExportPDF;
    bool ExportPNG;
    bool ExportSVG;
    QString ExportFolderPath;
    int ExportChannelNumberOffset;

};

#endif // PREFERENCES_H

