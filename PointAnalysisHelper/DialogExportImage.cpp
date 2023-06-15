#include <QFileDialog>

#include "Preferences.h"

#include "DialogExportImage.h"
#include "ui_DialogExportImage.h"

DialogExportImage::DialogExportImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogExportImage)
{
    ui->setupUi(this);

    connect(ui->pushButtonBrowse, SIGNAL(clicked()), this, SLOT(folderPathBrowse()));
}

DialogExportImage::~DialogExportImage()
{
    delete ui;
}

void DialogExportImage::folderPathBrowse()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Select Output Folder"), "");
    if (folder.isNull() == false)
    {
        ui->lineEditFolderPath->setText(folder);
    }
}

void DialogExportImage::setValues(const Preferences &prefs)
{
    ui->checkBox95thPercentile->setChecked(prefs.Export95thPercentile);
    ui->checkBox99p9thPercentile->setChecked(prefs.Export99p9thPercentile);
    ui->checkBox99p99thPercentile->setChecked(prefs.Export99p99thPercentile);
    ui->checkBox99thPercentile->setChecked(prefs.Export99thPercentile);
    ui->checkBoxDisplayedPercentile->setChecked(prefs.ExportDisplayedPercentile);
    ui->checkBoxGamma0p5->setChecked(prefs.ExportGamma0p5);
    ui->checkBoxGamma1->setChecked(prefs.ExportGamma1);
    ui->checkBoxGamma1p5->setChecked(prefs.ExportGamma1p5);
    ui->checkBoxGammaDisplayed->setChecked(prefs.ExportGammaDisplayed);
    ui->checkBoxJPG->setChecked(prefs.ExportJPG);
    ui->checkBoxTIFF->setChecked(prefs.ExportTIFF);
    ui->checkBoxPDF->setChecked(prefs.ExportPDF);
    ui->checkBoxPNG->setChecked(prefs.ExportPNG);
    ui->checkBoxSVG->setChecked(prefs.ExportSVG);

    ui->spinBoxChannelNumberOffset->setValue(prefs.ExportChannelNumberOffset);

    ui->lineEditFolderPath->setText(prefs.ExportFolderPath);
}

void DialogExportImage::getValues(Preferences *prefs)
{
    prefs->Export95thPercentile = ui->checkBox95thPercentile->isChecked();
    prefs->Export99p9thPercentile = ui->checkBox99p9thPercentile->isChecked();
    prefs->Export99p99thPercentile = ui->checkBox99p99thPercentile->isChecked();
    prefs->Export99thPercentile = ui->checkBox99thPercentile->isChecked();
    prefs->ExportDisplayedPercentile = ui->checkBoxDisplayedPercentile->isChecked();
    prefs->ExportGamma0p5 = ui->checkBoxGamma0p5->isChecked();
    prefs->ExportGamma1 = ui->checkBoxGamma1->isChecked();
    prefs->ExportGamma1p5 = ui->checkBoxGamma1p5->isChecked();
    prefs->ExportGammaDisplayed = ui->checkBoxGammaDisplayed->isChecked();
    prefs->ExportJPG = ui->checkBoxJPG->isChecked();
    prefs->ExportTIFF = ui->checkBoxTIFF->isChecked();
    prefs->ExportPDF = ui->checkBoxPDF->isChecked();
    prefs->ExportPNG = ui->checkBoxPNG->isChecked();
    prefs->ExportSVG = ui->checkBoxSVG->isChecked();

    prefs->ExportChannelNumberOffset = ui->spinBoxChannelNumberOffset->value();

    prefs->ExportFolderPath = ui->lineEditFolderPath->text();
}
