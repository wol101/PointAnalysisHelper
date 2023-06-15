#include <QFile>
#include <QFileInfo>
#include <QDomDocument>
#include <QDir>
#include <QFileDialog>

#include "PointLocations.h"
#include "MultiSpectralImage.h"
#include "math.h"

PointLocations::PointLocations()
{
    m_multiSpectralImage = 0;
    m_scaleBarLength = 1;
    m_topLeft = QPointF(0, 0);
    m_bottomRight = QPointF(1, 1);
}

PointLocations::~PointLocations()
{
    if (m_multiSpectralImage) delete m_multiSpectralImage;
}

void PointLocations::SetMultiSpectralImage(MultiSpectralImage *multiSpectralImage)
{
    m_pointList.clear();
    m_multiSpectralImage = multiSpectralImage;
}

void PointLocations::AddPoint(const QString &name, const QPoint &point)
{
    for (int i = 0; i < m_pointList.size(); i++)
    {
        if (m_pointList[i].name == name)
        {
            m_pointList[i].point = point;
            return;
        }
    }
    m_pointList.append(NamedPoint(name, point));
}

void PointLocations::DeletePoint(const QString &name)
{
    for (int i = 0; i < m_pointList.size(); i++)
    {
        if (m_pointList[i].name == name)
        {
            m_pointList.removeAt(i);
            return;
        }
    }
}

bool PointLocations::PointLocationExists(const QString &name)
{
    for (int i = 0; i < m_pointList.size(); i++)
    {
        if (m_pointList[i].name == name)
        {
            return true;
        }
    }
    return false;
}

QPoint PointLocations::GetPointLocation(const QString &name)
{
    for (int i = 0; i < m_pointList.size(); i++)
    {
        if (m_pointList[i].name == name)
        {
            return m_pointList[i].point;
        }
    }
    return QPoint(0, 0);
}

QPointF PointLocations::CalculateScanPosition(const QPoint &point)
{
    Q_ASSERT(m_multiSpectralImage != 0);
    Q_ASSERT(m_multiSpectralImage->GetChannels() != 0);

    double width = m_multiSpectralImage->GetPreview(0)->width();
    double height = m_multiSpectralImage->GetPreview(0)->height();
    QPointF calibratedPoint(m_topLeft.x() + (m_bottomRight.x() - m_topLeft.x()) * (double(point.x()) / (width - 1.f)), // (width - 1) because we are measuring centred locations not intervals
                            m_topLeft.y() + (m_bottomRight.y() - m_topLeft.y()) * (double(point.y()) / (height - 1.f))); // (height - 1) because we are measuring centred locations not intervals
    return calibratedPoint;
}

QPoint PointLocations::CalculateImagePosition(const QPointF &point)
{
    Q_ASSERT(m_multiSpectralImage != 0);
    Q_ASSERT(m_multiSpectralImage->GetChannels() != 0);

    double width = m_multiSpectralImage->GetPreview(0)->width();
    double height = m_multiSpectralImage->GetPreview(0)->height();
    QPoint imagePos(int((point.x() - m_topLeft.x()) * (width - 1) / (m_bottomRight.x() - m_topLeft.x())),
                    int((point.y() - m_topLeft.y()) * (height - 1) / (m_bottomRight.y() - m_topLeft.y())));
    return imagePos;
}

int PointLocations::Write(QString fileName)
{
    if (m_multiSpectralImage == 0)
    {
        qWarning("No multispectral image data to save");
        return __LINE__;
    }

    if (m_multiSpectralImage->GetChannels() == 0)
    {
        qWarning("Multispectral image has no channels");
        return __LINE__;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Unable to open to PointLocations file: %s", qPrintable(fileName));
        return __LINE__;
    }

    QDomDocument doc("POINTANALYSISHELPERXML");
    QDomElement root = doc.createElement("POINTANALYSISHELPERXML");
    doc.appendChild(root);

    QDomElement imageElement = doc.createElement("MULTISPECTRALIMAGE");
    root.appendChild(imageElement);

    // this works out the relative path of the multispectral image
    QFileInfo filenameInfo(fileName);
    QDir parentFolder = QDir(filenameInfo.path());
    QString relativeFilename = parentFolder.relativeFilePath(m_multiSpectralImage->GetRASFileName());

    imageElement.setAttribute("Filename", relativeFilename);
    imageElement.setAttribute("Channels", m_multiSpectralImage->GetChannels());
    imageElement.setAttribute("Width", m_multiSpectralImage->GetImageData(0)->width);
    imageElement.setAttribute("Height", m_multiSpectralImage->GetImageData(0)->height);
    imageElement.setAttribute("TopLeftX", m_topLeft.x());
    imageElement.setAttribute("TopLeftY", m_topLeft.y());
    imageElement.setAttribute("BottomRightX", m_bottomRight.x());
    imageElement.setAttribute("BottomRightY", m_bottomRight.y());
    imageElement.setAttribute("ApplyLineOffset", int(m_multiSpectralImage->GetApplyLineOffset()));
    imageElement.setAttribute("LineOffset", m_multiSpectralImage->GetLineOffset());


    QDomElement scaleBarElement = doc.createElement("ORNAMENTS");
    root.appendChild(scaleBarElement);
    scaleBarElement.setAttribute("ScaleBarLength", m_scaleBarLength);
    scaleBarElement.setAttribute("ScaleBarLocationX", m_scaleBarLocation.x());
    scaleBarElement.setAttribute("ScaleBarLocationY", m_scaleBarLocation.y());
    scaleBarElement.setAttribute("ScaleBarUnits", m_scaleBarUnits);
    scaleBarElement.setAttribute("ScaleBarFontSize", m_scaleBarFontSize);
    scaleBarElement.setAttribute("ScaleBarRed", m_scaleBarColor.red());
    scaleBarElement.setAttribute("ScaleBarGreen", m_scaleBarColor.green());
    scaleBarElement.setAttribute("ScaleBarBlue", m_scaleBarColor.blue());
    scaleBarElement.setAttribute("ScaleBarAlpha", m_scaleBarColor.alpha());
    scaleBarElement.setAttribute("MarkerFontSize", m_pointMapFontSize);
    scaleBarElement.setAttribute("MarkerRed", m_pointMapColor.red());
    scaleBarElement.setAttribute("MarkerGreen", m_pointMapColor.green());
    scaleBarElement.setAttribute("MarkerBlue", m_pointMapColor.blue());
    scaleBarElement.setAttribute("MarkerAlpha", m_pointMapColor.alpha());

    int i;
    for (i = 0; i < m_multiSpectralImage->GetChannels(); i++)
    {
        QDomElement channelInfo = doc.createElement("CHANNELINFORMATION");
        root.appendChild(channelInfo);
        channelInfo.setAttribute("Channel", QString("%1").arg(i));
        channelInfo.setAttribute("Min", QString("%1").arg(m_multiSpectralImage->GetImageData(i)->min));
        channelInfo.setAttribute("Max", QString("%1").arg(m_multiSpectralImage->GetImageData(i)->max));
        channelInfo.setAttribute("Gamma", QString("%1").arg(m_multiSpectralImage->GetImageData(i)->gamma));
        channelInfo.setAttribute("PercentileClip", QString("%1").arg(m_multiSpectralImage->GetImageData(i)->percentileClip));
        channelInfo.setAttribute("Label", m_multiSpectralImage->GetImageData(i)->label);
    }

    QList<NamedPoint>::const_iterator it = m_pointList.constBegin();
    while (it != m_pointList.constEnd())
    {
        QDomElement location = doc.createElement("POINTANALYSISLOCATION");
        root.appendChild(location);
        location.setAttribute("ImageX", it->point.x());
        location.setAttribute("ImageY", it->point.y());
        QPointF scanPos = CalculateScanPosition(it->point);
        location.setAttribute("ScanX", scanPos.x());
        location.setAttribute("ScanY", scanPos.y());
        location.setAttribute("Name", it->name);
        ++it;
    }

    // we need to add the processing instruction at the beginning of the document
    QString encoding("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    file.write(encoding.toUtf8());

    // and now the actual xml doc
    QString xmlString = doc.toString();
    QByteArray xmlData = xmlString.toUtf8();
    qint64 bytesWritten = file.write(xmlData);
    if (bytesWritten != xmlData.size()) qWarning("Unable to write to PointLocations file: %s", qPrintable(fileName));
    file.close();

    return 0;
}

int PointLocations::Read(QString fileName)
{
    m_pointList.clear();
    m_multiSpectralImage = 0;

    QFileInfo filenameInfo(fileName);
    QDir parentFolder = QDir(filenameInfo.path());

    QDomDocument doc;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return __LINE__;
    if (!doc.setContent(&file)) return __LINE__;
    file.close();

    QPoint pointAnalysisLocation;
    QString key;
    QDomElement docElem = doc.documentElement();
    // qWarning("Element name: %s", qPrintable(docElem.tagName()));
    if (docElem.tagName() != "POINTANALYSISHELPERXML") return __LINE__;
    QDomNode n = docElem.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull())
        {
            if (e.tagName() == "MULTISPECTRALIMAGE")
            {
                QString filenameAttribute = e.attribute("Filename");
                QString RASFileName = parentFolder.absoluteFilePath(filenameAttribute);
                if (QFile::exists(RASFileName) == false)
                {
                    QString fileName = QFileDialog::getOpenFileName(0, QString("%1 missing. Select a new RAS File").arg(RASFileName), QFileInfo(RASFileName).path(), "RAS Files (*.RAS *.ras)");
                    if (fileName.isNull() == true) return __LINE__;
                    RASFileName = fileName;
                }
                m_multiSpectralImage = new MultiSpectralImage();
                if (m_multiSpectralImage->ReadRASFile(RASFileName)) return __LINE__;
                int channels = e.attribute("Channels").toInt();
                int width = e.attribute("Width").toInt();
                int height = e.attribute("Height").toInt();
                if (channels != m_multiSpectralImage->GetChannels()) return __LINE__;
                if (width != m_multiSpectralImage->GetImageData(0)->width) return __LINE__;
                if (height != m_multiSpectralImage->GetImageData(0)->height) return __LINE__;
                m_topLeft.setX(e.attribute("TopLeftX").toDouble());
                m_topLeft.setY(e.attribute("TopLeftY").toDouble());
                m_bottomRight.setX(e.attribute("BottomRightX").toDouble());
                m_bottomRight.setY(e.attribute("BottomRightY").toDouble());
                m_multiSpectralImage->SetApplyLineOffset(bool(e.attribute("ApplyLineOffset").toInt()));
                m_multiSpectralImage->SetLineOffset(e.attribute("LineOffset").toInt());
            }
            if (e.tagName() == "ORNAMENTS")
            {
                m_scaleBarLength = e.attribute("ScaleBarLength").toDouble();
                m_scaleBarLocation.setX(e.attribute("ScaleBarLocationX").toInt());
                m_scaleBarLocation.setY(e.attribute("ScaleBarLocationY").toInt());
                m_scaleBarUnits = e.attribute("ScaleBarUnits");
                m_scaleBarFontSize = e.attribute("ScaleBarFontSize").toInt();
                m_scaleBarColor.setRed(e.attribute("ScaleBarRed").toInt());
                m_scaleBarColor.setGreen(e.attribute("ScaleBarGreen").toInt());
                m_scaleBarColor.setBlue(e.attribute("ScaleBarBlue").toInt());
                m_scaleBarColor.setAlpha(e.attribute("ScaleBarAlpha").toInt());
                m_pointMapFontSize = e.attribute("MarkerFontSize").toInt();
                m_pointMapColor.setRed(e.attribute("MarkerRed").toInt());
                m_pointMapColor.setGreen(e.attribute("MarkerGreen").toInt());
                m_pointMapColor.setBlue(e.attribute("MarkerBlue").toInt());
                m_pointMapColor.setAlpha(e.attribute("MarkerAlpha").toInt());
            }
            if (e.tagName() == "CHANNELINFORMATION")
            {
                int channel = e.attribute("Channel").toInt();
                uint32_t channelMin = e.attribute("Min").toUInt();
                uint32_t channelMax = e.attribute("Max").toUInt();
                double channelGamma = e.attribute("Gamma").toDouble();
                double channelPercentileClip = e.attribute("PercentileClip").toDouble();
                QString channelLabel = e.attribute("Label");
                if (m_multiSpectralImage->GetImageData(channel)->min != channelMin) return __LINE__;
                if (m_multiSpectralImage->GetImageData(channel)->max != channelMax) return __LINE__;
                m_multiSpectralImage->GetImageData(channel)->gamma = channelGamma;
                m_multiSpectralImage->GetImageData(channel)->percentileClip = channelPercentileClip;
                m_multiSpectralImage->GetImageData(channel)->label = channelLabel;
            }
            if (e.tagName() == "POINTANALYSISLOCATION")
            {
                pointAnalysisLocation.setX(e.attribute("ImageX").toInt());
                pointAnalysisLocation.setY(e.attribute("ImageY").toInt());
                QPointF scanPos;
                scanPos.setX(e.attribute("ScanX").toDouble()); // the calibrated values are currently ignored although we could check they were right
                scanPos.setY(e.attribute("ScanY").toDouble());
                key = e.attribute("Name");
                AddPoint(key, pointAnalysisLocation);
            }
        }
        n = n.nextSibling();
    }
    return 0;
}

void PointLocations::SetOrnaments(double scaleBarLength, const QPoint &scaleBarLocation, const QString &scaleBarUnits, int scaleBarFontSize, int markerFontSize)
{
    m_scaleBarLength = scaleBarLength;
    m_scaleBarLocation = scaleBarLocation;
    m_scaleBarUnits = scaleBarUnits;
    m_scaleBarFontSize = scaleBarFontSize;
    m_pointMapFontSize = markerFontSize;
}

void PointLocations::GetOrnaments(double *scaleBarLength, QPoint *scaleBarLocation, QString *scaleBarUnits, int *scaleBarFontSize, int *markerFontSize)
{
    *scaleBarLength = m_scaleBarLength;
    *scaleBarLocation = m_scaleBarLocation;
    *scaleBarUnits = m_scaleBarUnits;
    *scaleBarFontSize = m_scaleBarFontSize;
    *markerFontSize = m_pointMapFontSize;
}
