#include <QColorDialog>
#include <QAbstractSpinBox>

#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"
#include "Preferences.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    // I want to fix the sizes of the input widgets and this seems hard to do in designer
    int targetSize = 142; // this value is the current lineEditScaleBarUnits width in QtDesigner
    ui->lineEditScaleBarUnits->setFixedWidth(targetSize);
    QList<QAbstractSpinBox *> widgets = findChildren<QAbstractSpinBox *>(); // this will find all the spin boxes
    for (int i = 0; i < widgets.size(); i++) widgets[i]->setFixedWidth(targetSize);


    connect(ui->pushButtonMarkerColor, SIGNAL(clicked()), this, SLOT(markerColorClicked()));
    connect(ui->pushButtonScaleBarColor, SIGNAL(clicked()), this, SLOT(scaleBarColorClicked()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::setValues(const Preferences &prefs)
{
    ui->lineEditScaleBarUnits->setText(prefs.ScaleBarUnits);
    ui->doubleSpinBoxScaleBarLength->setValue(prefs.ScaleBarLength);
    ui->spinBoxScaleBarX->setValue(prefs.ScaleBarLocation.x());
    ui->spinBoxScaleBarY->setValue(prefs.ScaleBarLocation.y());
    ui->spinBoxScaleBarFontSize->setValue(prefs.ScaleBarFontSize);
    ui->spinBoxMarkerFontSize->setValue(prefs.MarkerFontSize);
    ui->doubleSpinBoxDefaultGamma->setValue(prefs.DefaultGamma);
    ui->doubleSpinBoxDefaultPercentileClip->setValue(prefs.DefaultPercentileClip);
    ui->spinBoxNumOffsets->setValue(prefs.NumberOfOffsets);
    ui->spinBoxNumLinePairs->setValue(prefs.NumberOfRowPairs);

    const QString Color_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");
    m_markerColor = prefs.MarkerColor;
    ui->pushButtonMarkerColor->setStyleSheet(Color_STYLE.arg(m_markerColor.name()).arg(getIdealTextColor(m_markerColor).name()).arg(getAlphaColorHint(m_markerColor).name()));
    m_scaleBarColor = prefs.ScaleBarColor;
    ui->pushButtonScaleBarColor->setStyleSheet(Color_STYLE.arg(m_scaleBarColor.name()).arg(getIdealTextColor(m_scaleBarColor).name()).arg(getAlphaColorHint(m_scaleBarColor).name()));

}

void PreferencesDialog::getValues(Preferences *prefs)
{
    prefs->ScaleBarUnits = ui->lineEditScaleBarUnits->text();
    prefs->ScaleBarLength = ui->doubleSpinBoxScaleBarLength->value();
    prefs->ScaleBarLocation.setX(ui->spinBoxScaleBarX->value());
    prefs->ScaleBarLocation.setY(ui->spinBoxScaleBarY->value());
    prefs->ScaleBarFontSize = ui->spinBoxScaleBarFontSize->value();
    prefs->MarkerFontSize = ui->spinBoxMarkerFontSize->value();
    prefs->DefaultGamma = ui->doubleSpinBoxDefaultGamma->value();
    prefs->DefaultPercentileClip = ui->doubleSpinBoxDefaultPercentileClip->value();
    prefs->NumberOfOffsets = ui->spinBoxNumOffsets->value();
    prefs->NumberOfRowPairs = ui->spinBoxNumLinePairs->value();

    prefs->MarkerColor = m_markerColor;
    prefs->ScaleBarColor = m_scaleBarColor;
}

void PreferencesDialog::markerColorClicked()
{
    const QString Color_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");

#ifdef Q_OS_MAC
    QColor Color = QColorDialog::getColor(m_markerColor, this, "Select Color", QColorDialog::ShowAlphaChannel);
#else
    QColor Color = QColorDialog::getColor(m_markerColor, this, "Select Color", QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
#endif
    if (Color.isValid())
    {
        ui->pushButtonMarkerColor->setStyleSheet(Color_STYLE.arg(Color.name()).arg(getIdealTextColor(Color).name()).arg(getAlphaColorHint(Color).name()));
        m_markerColor = Color;
    }
}

void PreferencesDialog::scaleBarColorClicked()
{
    const QString Color_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");

#ifdef Q_OS_MAC
    QColor Color = QColorDialog::getColor(m_scaleBarColor, this, "Select Color", QColorDialog::ShowAlphaChannel);
#else
    QColor Color = QColorDialog::getColor(m_scaleBarColor, this, "Select Color", QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
#endif
    if (Color.isValid())
    {
        ui->pushButtonScaleBarColor->setStyleSheet(Color_STYLE.arg(Color.name()).arg(getIdealTextColor(Color).name()).arg(getAlphaColorHint(Color).name()));
        m_scaleBarColor = Color;
    }
}

// return an ideal label Color, based on the given background Color.
// Based on http://www.codeproject.com/cs/media/IdealTextColor.asp
QColor PreferencesDialog::getIdealTextColor(const QColor& rBackgroundColor)
{
    const int THRESHOLD = 105;
    int BackgroundDelta = (rBackgroundColor.red() * 0.299) + (rBackgroundColor.green() * 0.587) + (rBackgroundColor.blue() * 0.114);
    return QColor((255- BackgroundDelta < THRESHOLD) ? Qt::black : Qt::white);
}

QColor PreferencesDialog::getAlphaColorHint(const QColor& Color)
{
    // (source * Blend.SourceAlpha) + (background * Blend.InvSourceAlpha)
    QColor background;
    background.setRgbF(1.0, 1.0, 1.0);
    QColor hint;
    hint.setRedF((Color.redF() * Color.alphaF()) + (background.redF() * (1 - Color.alphaF())));
    hint.setGreenF((Color.greenF() * Color.alphaF()) + (background.greenF() * (1 - Color.alphaF())));
    hint.setBlueF((Color.blueF() * Color.alphaF()) + (background.blueF() * (1 - Color.alphaF())));
    return hint;
}




