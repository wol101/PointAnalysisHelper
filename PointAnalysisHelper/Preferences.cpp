#include <QSettings>

#include "Preferences.h"

Preferences::Preferences()
{
    Organisation = "AnimalSimulationLaboratory";
    Application = "PointAnalysisHelper";
}

void Preferences::Read()
{
    QSettings settings(Organisation, Application);

    MainWindowSize = settings.value("MainWindowSize", QSize(0, 0)).toSize();
    MainWindowPosition = settings.value("MainWindowPosition", QPoint(0, 0)).toPoint();

    LastOpenedRASFile = settings.value("LastOpenedRASFile").toString();
    LastOpenedPointLocationsFile = settings.value("LastOpenedPointLocationsFile").toString();

    ScaleBarUnits = settings.value("ScaleBarUnits", "mm").toString();
    ScaleBarLength = settings.value("ScaleBarLength", 0.1).toDouble();
    ScaleBarLocation = settings.value("ScaleBarLocation", QPoint(50, 50)).toPoint();
    ScaleBarFontSize = settings.value("ScaleBarFontSize", 30).toInt();
    MarkerFontSize = settings.value("MarkerFontSize", 20).toInt();

    DefaultGamma = settings.value("DefaultGamma", 0.5).toDouble();
    DefaultPercentileClip = settings.value("DefaultPercentileClip", 99.9).toDouble();

    NumberOfOffsets = settings.value("NumberOfOffsets", 50).toInt();
    NumberOfRowPairs = settings.value("NumberOfRowPairs", 5).toInt();

    // handle default Colors
    if (settings.contains("MarkerColor") == false) settings.setValue("MarkerColor", QColor(255, 255, 255, 255).rgba());
    if (settings.contains("ScaleBarColor") == false) settings.setValue("ScaleBarColor", QColor(255, 255, 255, 255).rgba());

    MarkerColor.setRgba(settings.value("MarkerColor").toUInt());
    ScaleBarColor.setRgba(settings.value("ScaleBarColor").toUInt());

    Export100thPercentile = settings.value("Export100thPercentile", false).toBool();
    Export95thPercentile = settings.value("Export95thPercentile", false).toBool();
    Export99p9thPercentile = settings.value("Export99p9thPercentile", false).toBool();
    Export99p99thPercentile = settings.value("Export99p99thPercentile", false).toBool();
    Export99thPercentile = settings.value("Export99thPercentile", false).toBool();
    ExportDisplayedPercentile = settings.value("ExportDisplayedPercentile", true).toBool();
    ExportGamma0p5 = settings.value("ExportGamma0p5", false).toBool();
    ExportGamma1 = settings.value("ExportGamma1", false).toBool();
    ExportGamma1p5 = settings.value("ExportGamma1p5", false).toBool();
    ExportGammaDisplayed = settings.value("ExportGammaDisplayed", true).toBool();
    ExportJPG = settings.value("ExportJPG", false).toBool();
    ExportTIFF = settings.value("ExportTIFF", false).toBool();
    ExportPDF = settings.value("ExportPDF", false).toBool();
    ExportPNG = settings.value("ExportPNG", true).toBool();
    ExportSVG = settings.value("ExportSVG", false).toBool();
    ExportFolderPath = settings.value("ExportFolderPath").toString();
    ExportChannelNumberOffset = settings.value("ExportChannelNumberOffset", 0).toInt();
}

void Preferences::Write()
{
    QSettings settings(Organisation, Application);

    settings.setValue("MainWindowSize", MainWindowSize);
    settings.setValue("MainWindowPosition", MainWindowPosition);

    settings.setValue("LastOpenedRASFile", LastOpenedRASFile);
    settings.setValue("LastOpenedPointLocationsFile", LastOpenedPointLocationsFile);

    settings.setValue("ScaleBarUnits", ScaleBarUnits);
    settings.setValue("ScaleBarLength", ScaleBarLength);
    settings.setValue("ScaleBarLocation", ScaleBarLocation);
    settings.setValue("ScaleBarFontSize", ScaleBarFontSize);
    settings.setValue("MarkerFontSize", MarkerFontSize);

    settings.setValue("DefaultGamma", DefaultGamma);
    settings.setValue("DefaultPercentileClip", DefaultPercentileClip);

    settings.setValue("NumberOfOffsets", NumberOfOffsets);
    settings.setValue("NumberOfRowPairs", NumberOfRowPairs);

    settings.setValue("MarkerColor", MarkerColor.rgba());
    settings.setValue("ScaleBarColor", ScaleBarColor.rgba());

    settings.setValue("Export95thPercentile", Export95thPercentile);
    settings.setValue("Export99p9thPercentile", Export99p9thPercentile);
    settings.setValue("Export99p99thPercentile", Export99p99thPercentile);
    settings.setValue("Export99thPercentile", Export99thPercentile);
    settings.setValue("ExportDisplayedPercentile", ExportDisplayedPercentile);
    settings.setValue("ExportGamma0p5", ExportGamma0p5);
    settings.setValue("ExportGamma1", ExportGamma1);
    settings.setValue("ExportGamma1p5", ExportGamma1p5);
    settings.setValue("ExportGammaDisplayed", ExportGammaDisplayed);
    settings.setValue("ExportJPG", ExportJPG);
    settings.setValue("ExportTIFF", ExportTIFF);
    settings.setValue("ExportPDF", ExportPDF);
    settings.setValue("ExportPNG", ExportPNG);
    settings.setValue("ExportSVG", ExportSVG);
    settings.setValue("ExportFolderPath", ExportFolderPath);
    settings.setValue("ExportChannelNumberOffset", ExportChannelNumberOffset);

    settings.sync();
}
