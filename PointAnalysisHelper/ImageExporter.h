#ifndef IMAGEEXPORTER_H
#define IMAGEEXPORTER_H

#include <QString>
#include <QWidget>

class PointLocations;

class ImageExporter
{
public:
    ImageExporter(PointLocations *pointLocations, QWidget *parentWidget = 0);

    void ExportPDF(const QString &fileName, double inputPercentileClip, double inputGamma);
    void ExportSVG(const QString &fileName, double inputPercentileClip, double inputGamma);
    void ExportPNG(const QString &fileName, double inputPercentileClip, double inputGamma);
    void ExportImage(const QString &fileName, double inputPercentileClip, double inputGamma);

private:
    PointLocations *m_pointLocations;
    QWidget *m_parentWidget;
};

#endif // IMAGEEXPORTER_H
