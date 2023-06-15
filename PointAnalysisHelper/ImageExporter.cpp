#include <QFileInfo>
#include <QProgressDialog>
#include <QMap>
#include <Qstring>
#include <QPointF>
#include <QSvgGenerator>
#include <QPrinter>
#include <QPainter>
#include <QImage>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDir>

#include <math.h>

#include "ImageExporter.h"
#include "PointLocations.h"
#include "MultiSpectralImage.h"
#include "GraphicsItemAnalysisLocation.h"
#include "GraphicsItemScaleBar.h"
#include "MdiChild.h"

ImageExporter::ImageExporter(PointLocations *pointLocations, QWidget *parentWidget)
{
    m_parentWidget = parentWidget;
    m_pointLocations = pointLocations;
}

void ImageExporter::ExportPDF(const QString &fileName, double inputPercentileClip, double inputGamma)
{
    double percentileClip, gamma, storedPercentileClip, storedGamma;
    MultiSpectralImage *multiSpectralImage = m_pointLocations->GetMultiSpectralImage();
    QFileInfo fileInfo(fileName);
    QString dirPath = fileInfo.path();
    QString rootName = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();

    QProgressDialog progressDialog("Generating PDF files...", "Abort Generation", 0, multiSpectralImage->GetChannels(), m_parentWidget);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    int channel;
    for (channel = 0; channel < multiSpectralImage->GetChannels(); channel++)
    {
        progressDialog.setValue(channel);
        QApplication::processEvents();
        if (progressDialog.wasCanceled()) break;

        storedPercentileClip = multiSpectralImage->GetImageData(channel)->percentileClip;
        storedGamma =  multiSpectralImage->GetImageData(channel)->gamma;
        if (inputPercentileClip == 0) percentileClip = multiSpectralImage->GetImageData(channel)->percentileClip;
        else percentileClip = inputPercentileClip;
        if (inputGamma == 0) gamma = multiSpectralImage->GetImageData(channel)->gamma;
        else gamma = inputGamma;
        if (gamma != storedGamma || percentileClip != storedPercentileClip) multiSpectralImage->RefreshPreview(channel, gamma, percentileClip);

        QGraphicsScene scene;
        QGraphicsPixmapItem *graphicsPixmapItem = scene.addPixmap(QPixmap::fromImage(*multiSpectralImage->GetPreview(channel)));
        scene.setSceneRect(graphicsPixmapItem->boundingRect());

        QList<NamedPoint> *pointList = m_pointLocations->GetPointList();
        QList<NamedPoint>::const_iterator it = pointList->constBegin();
        while (it != pointList->constEnd())
        {
            GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(it->name, m_pointLocations->GetMarkerFontSize(), m_pointLocations->GetMarkerColor());
            newLocation->setPos(QPointF(it->point.x() + 0.5, it->point.y() + 0.5));
            newLocation->setZValue(1);
            scene.addItem(newLocation);
            ++it;
        }

        QPointF topLeft, bottomRight;
        double scaleBarLength, pixelSize;
        QPoint scaleBarLocation;
        QString scaleBarUnits, scaleBarLabel;
        int scaleBarFontSize, markerFontSize;;
        m_pointLocations->GetCalibrations(&topLeft, &bottomRight);
        m_pointLocations->GetOrnaments(&scaleBarLength, &scaleBarLocation, &scaleBarUnits, &scaleBarFontSize, &markerFontSize);
        scaleBarLabel = QString("%1 %2").arg(scaleBarLength).arg(scaleBarUnits);
        pixelSize = (scaleBarLength / (bottomRight.x() - topLeft.x())) * multiSpectralImage->GetImageData(channel)->width;
        GraphicsItemScaleBar *scaleBar = new GraphicsItemScaleBar(pixelSize, scaleBarLabel, ScaleBarRectangle, scaleBarFontSize, m_pointLocations->GetScaleBarColor());
        scaleBar->setPos(QPointF(scaleBarLocation.x(), scaleBarLocation.y()));
        scaleBar->setZValue(1);
        scene.addItem(scaleBar);

        QString printFileName = QString("%1_ch%2_%3_%4%_%5g.%6").arg(rootName).arg(channel + dynamic_cast<MdiChild *>(m_parentWidget)->GetPreferences()->ExportChannelNumberOffset,
                                                                                   int(log10(multiSpectralImage->GetChannels())) + 1, 10, QChar('0')
                                                                                  ).arg( multiSpectralImage->GetImageData(channel)->label).arg(percentileClip).arg(gamma).arg(suffix);
        QPrinter printer(QPrinter::ScreenResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setPaperSize(QSizeF(scene.sceneRect().width(), scene.sceneRect().height()), QPrinter::Millimeter);
        printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
        printer.setOutputFileName(QDir(dirPath).filePath(printFileName));
        QPainter painter(&printer);
        scene.render(&painter);

        multiSpectralImage->GetImageData(channel)->percentileClip = storedPercentileClip; // note that this does NOT refresh the image
        multiSpectralImage->GetImageData(channel)->gamma = storedGamma; // note that this does NOT refresh the image
    }
    progressDialog.hide();
}

void ImageExporter::ExportSVG(const QString &fileName, double inputPercentileClip, double inputGamma)
{
    double percentileClip, gamma, storedPercentileClip, storedGamma;
    MultiSpectralImage *multiSpectralImage = m_pointLocations->GetMultiSpectralImage();
    QFileInfo fileInfo(fileName);
    QString dirPath = fileInfo.path();
    QString rootName = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();

    QProgressDialog progressDialog("Generating SVG files...", "Abort Generation", 0, multiSpectralImage->GetChannels(), m_parentWidget);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    int channel;
    for (channel = 0; channel < multiSpectralImage->GetChannels(); channel++)
    {
        progressDialog.setValue(channel);
        QApplication::processEvents();
        if (progressDialog.wasCanceled()) break;

        storedPercentileClip = multiSpectralImage->GetImageData(channel)->percentileClip;
        storedGamma =  multiSpectralImage->GetImageData(channel)->gamma;
        if (inputPercentileClip == 0) percentileClip = multiSpectralImage->GetImageData(channel)->percentileClip;
        else percentileClip = inputPercentileClip;
        if (inputGamma == 0) gamma = multiSpectralImage->GetImageData(channel)->gamma;
        else gamma = inputGamma;
        if (gamma != storedGamma || percentileClip != storedPercentileClip) multiSpectralImage->RefreshPreview(channel, gamma, percentileClip);

        QGraphicsScene scene;
        QGraphicsPixmapItem *graphicsPixmapItem = scene.addPixmap(QPixmap::fromImage(*multiSpectralImage->GetPreview(channel)));
        scene.setSceneRect(graphicsPixmapItem->boundingRect());

        QList<NamedPoint> *pointList = m_pointLocations->GetPointList();
        QList<NamedPoint>::const_iterator it = pointList->constBegin();
        while (it != pointList->constEnd())
        {
            GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(it->name, m_pointLocations->GetMarkerFontSize(), m_pointLocations->GetMarkerColor());
            newLocation->setPos(QPointF(it->point.x() + 0.5, it->point.y() + 0.5));
            newLocation->setZValue(1);
            scene.addItem(newLocation);
            ++it;
        }

        QPointF topLeft, bottomRight;
        double scaleBarLength, pixelSize;
        QPoint scaleBarLocation;
        QString scaleBarUnits, scaleBarLabel;
        int scaleBarFontSize, markerFontSize;;
        m_pointLocations->GetCalibrations(&topLeft, &bottomRight);
        m_pointLocations->GetOrnaments(&scaleBarLength, &scaleBarLocation, &scaleBarUnits, &scaleBarFontSize, &markerFontSize);
        scaleBarLabel = QString("%1 %2").arg(scaleBarLength).arg(scaleBarUnits);
        pixelSize = (scaleBarLength / (bottomRight.x() - topLeft.x())) * multiSpectralImage->GetImageData(channel)->width;
        GraphicsItemScaleBar *scaleBar = new GraphicsItemScaleBar(pixelSize, scaleBarLabel, ScaleBarRectangle, scaleBarFontSize, m_pointLocations->GetScaleBarColor());
        scaleBar->setPos(QPointF(scaleBarLocation.x(), scaleBarLocation.y()));
        scaleBar->setZValue(1);
        scene.addItem(scaleBar);

        QString printFileName = QString("%1_ch%2_%3_%4%_%5g.%6").arg(rootName).arg(channel + dynamic_cast<MdiChild *>(m_parentWidget)->GetPreferences()->ExportChannelNumberOffset,
                                                                                   int(log10(multiSpectralImage->GetChannels())) + 1, 10, QChar('0')
                                                                                  ).arg( multiSpectralImage->GetImageData(channel)->label).arg(percentileClip).arg(gamma).arg(suffix);

        QSvgGenerator svgGen;
        svgGen.setFileName(QDir(dirPath).filePath(printFileName));
        QRect box(graphicsPixmapItem->boundingRect().toRect());
        svgGen.setSize(QSize(box.width(), box.height()));
        svgGen.setViewBox(box);
        svgGen.setResolution(72);
        QPainter painter(&svgGen);
        scene.render(&painter);

        multiSpectralImage->GetImageData(channel)->percentileClip = storedPercentileClip; // note that this does NOT refresh the image
        multiSpectralImage->GetImageData(channel)->gamma = storedGamma; // note that this does NOT refresh the image
    }
    progressDialog.hide();
}

void ImageExporter::ExportPNG(const QString &fileName, double inputPercentileClip, double inputGamma)
{
    double percentileClip, gamma, storedPercentileClip, storedGamma;
    bool writeOverlay = true;
    MultiSpectralImage *multiSpectralImage = m_pointLocations->GetMultiSpectralImage();
    QFileInfo fileInfo(fileName);
    QString dirPath = fileInfo.path();
    QString rootName = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();

    QProgressDialog progressDialog("Generating image files...", "Abort Generation", 0, multiSpectralImage->GetChannels(), m_parentWidget);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    int channel;
    QRectF bounds;
    for (channel = 0; channel < multiSpectralImage->GetChannels(); channel++)
    {
        progressDialog.setValue(channel);
        QApplication::processEvents();
        if (progressDialog.wasCanceled()) break;

        storedPercentileClip = multiSpectralImage->GetImageData(channel)->percentileClip;
        storedGamma =  multiSpectralImage->GetImageData(channel)->gamma;
        if (inputPercentileClip == 0) percentileClip = multiSpectralImage->GetImageData(channel)->percentileClip;
        else percentileClip = inputPercentileClip;
        if (inputGamma == 0) gamma = multiSpectralImage->GetImageData(channel)->gamma;
        else gamma = inputGamma;
        if (gamma != storedGamma || percentileClip != storedPercentileClip) multiSpectralImage->RefreshPreview(channel, gamma, percentileClip);

        QImage *preview = multiSpectralImage->GetPreview(channel);
        QGraphicsScene scene;
        QGraphicsPixmapItem *graphicsPixmapItem;
        graphicsPixmapItem = scene.addPixmap(QPixmap::fromImage(*preview));
        bounds = graphicsPixmapItem->boundingRect();
        scene.setSceneRect(bounds);

        QString printFileName = QString("%1_ch%2_%3_%4%_%5g.%6").arg(rootName).arg(channel + dynamic_cast<MdiChild *>(m_parentWidget)->GetPreferences()->ExportChannelNumberOffset,
                                                                                   int(log10(multiSpectralImage->GetChannels())) + 1, 10, QChar('0')
                                                                                  ).arg( multiSpectralImage->GetImageData(channel)->label).arg(percentileClip).arg(gamma).arg(suffix);

        QImage image(preview->width(), preview->height(), QImage::Format_RGB32);
        QPainter painter(&image);
        scene.render(&painter);
        image.save(QDir(dirPath).filePath(printFileName));

        multiSpectralImage->GetImageData(channel)->percentileClip = storedPercentileClip; // note that this does NOT refresh the image
        multiSpectralImage->GetImageData(channel)->gamma = storedGamma; // note that this does NOT refresh the image
    }

    if (writeOverlay)
    {
        channel = 0;
        QImage *preview = multiSpectralImage->GetPreview(channel);
        QGraphicsScene scene;
        scene.setSceneRect(bounds);

        QList<NamedPoint> *pointList = m_pointLocations->GetPointList();
        QList<NamedPoint>::const_iterator it = pointList->constBegin();
        while (it != pointList->constEnd())
        {
            GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(it->name, m_pointLocations->GetMarkerFontSize(), m_pointLocations->GetMarkerColor());
            newLocation->setPos(QPointF(it->point.x() + 0.5, it->point.y() + 0.5));
            newLocation->setZValue(1);
            scene.addItem(newLocation);
            ++it;
        }
        QPointF topLeft, bottomRight;
        double scaleBarLength, pixelSize;
        QPoint scaleBarLocation;
        QString scaleBarUnits, scaleBarLabel;
        int scaleBarFontSize, markerFontSize;;
        m_pointLocations->GetCalibrations(&topLeft, &bottomRight);
        m_pointLocations->GetOrnaments(&scaleBarLength, &scaleBarLocation, &scaleBarUnits, &scaleBarFontSize, &markerFontSize);
        scaleBarLabel = QString("%1 %2").arg(scaleBarLength).arg(scaleBarUnits);
        pixelSize = (scaleBarLength / (bottomRight.x() - topLeft.x())) * multiSpectralImage->GetImageData(channel)->width;
        GraphicsItemScaleBar *scaleBar = new GraphicsItemScaleBar(pixelSize, scaleBarLabel, ScaleBarRectangle, scaleBarFontSize, m_pointLocations->GetScaleBarColor());
        scaleBar->setPos(QPointF(scaleBarLocation.x(), scaleBarLocation.y()));
        scaleBar->setZValue(1);
        scene.addItem(scaleBar);
        QString printFileName = QString("%1_overlay.%2").arg(rootName).arg(suffix);
        QImage image(preview->width(), preview->height(), QImage::Format_ARGB32);
        for(int iy = 0; iy < preview->height(); iy++) // this is necessary because the empty image is opaque
            for(int ix = 0; ix < preview->width(); ix++)
                image.setPixel(ix, iy, qRgba(0, 0, 0, 0));
        QPainter painter(&image);
        scene.render(&painter);
        image.save(QDir(dirPath).filePath(printFileName));
    }
    progressDialog.hide();
}

void ImageExporter::ExportImage(const QString &fileName, double inputPercentileClip, double inputGamma)
{
    double percentileClip, gamma, storedPercentileClip, storedGamma;
    MultiSpectralImage *multiSpectralImage = m_pointLocations->GetMultiSpectralImage();
    QFileInfo fileInfo(fileName);
    QString dirPath = fileInfo.path();
    QString rootName = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();

    QProgressDialog progressDialog("Generating image files...", "Abort Generation", 0, multiSpectralImage->GetChannels(), m_parentWidget);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    int channel;
    for (channel = 0; channel < multiSpectralImage->GetChannels(); channel++)
    {
        progressDialog.setValue(channel);
        QApplication::processEvents();
        if (progressDialog.wasCanceled()) break;

        storedPercentileClip = multiSpectralImage->GetImageData(channel)->percentileClip;
        storedGamma =  multiSpectralImage->GetImageData(channel)->gamma;
        if (inputPercentileClip == 0) percentileClip = multiSpectralImage->GetImageData(channel)->percentileClip;
        else percentileClip = inputPercentileClip;
        if (inputGamma == 0) gamma = multiSpectralImage->GetImageData(channel)->gamma;
        else gamma = inputGamma;
        if (gamma != storedGamma || percentileClip != storedPercentileClip) multiSpectralImage->RefreshPreview(channel, gamma, percentileClip);

        QImage *preview = multiSpectralImage->GetPreview(channel);
        QGraphicsScene scene;
        QGraphicsPixmapItem *graphicsPixmapItem = scene.addPixmap(QPixmap::fromImage(*preview));
        scene.setSceneRect(graphicsPixmapItem->boundingRect());

        QList<NamedPoint> *pointList = m_pointLocations->GetPointList();
        QList<NamedPoint>::const_iterator it = pointList->constBegin();
        while (it != pointList->constEnd())
        {
            GraphicsItemAnalysisLocation *newLocation = new GraphicsItemAnalysisLocation(it->name, m_pointLocations->GetMarkerFontSize(), m_pointLocations->GetMarkerColor());
            newLocation->setPos(QPointF(it->point.x() + 0.5, it->point.y() + 0.5));
            newLocation->setZValue(1);
            scene.addItem(newLocation);
            ++it;
        }

        QPointF topLeft, bottomRight;
        double scaleBarLength, pixelSize;
        QPoint scaleBarLocation;
        QString scaleBarUnits, scaleBarLabel;
        int scaleBarFontSize, markerFontSize;;
        m_pointLocations->GetCalibrations(&topLeft, &bottomRight);
        m_pointLocations->GetOrnaments(&scaleBarLength, &scaleBarLocation, &scaleBarUnits, &scaleBarFontSize, &markerFontSize);
        scaleBarLabel = QString("%1 %2").arg(scaleBarLength).arg(scaleBarUnits);
        pixelSize = (scaleBarLength / (bottomRight.x() - topLeft.x())) * multiSpectralImage->GetImageData(channel)->width;
        GraphicsItemScaleBar *scaleBar = new GraphicsItemScaleBar(pixelSize, scaleBarLabel, ScaleBarRectangle, scaleBarFontSize, m_pointLocations->GetScaleBarColor());
        scaleBar->setPos(QPointF(scaleBarLocation.x(), scaleBarLocation.y()));
        scaleBar->setZValue(1);
        scene.addItem(scaleBar);

        QString printFileName = QString("%1_ch%2_%3_%4%_%5g.%6").arg(rootName).arg(channel + dynamic_cast<MdiChild *>(m_parentWidget)->GetPreferences()->ExportChannelNumberOffset,
                                                                                   int(log10(multiSpectralImage->GetChannels())) + 1, 10, QChar('0')
                                                                                  ).arg( multiSpectralImage->GetImageData(channel)->label).arg(percentileClip).arg(gamma).arg(suffix);

        QImage image(preview->width(), preview->height(), QImage::Format_RGB32);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        scene.render(&painter);
        image.save(QDir(dirPath).filePath(printFileName));

        multiSpectralImage->GetImageData(channel)->percentileClip = storedPercentileClip; // note that this does NOT refresh the image
        multiSpectralImage->GetImageData(channel)->gamma = storedGamma; // note that this does NOT refresh the image
    }
    progressDialog.hide();
}
