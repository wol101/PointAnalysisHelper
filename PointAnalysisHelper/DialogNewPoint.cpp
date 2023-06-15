#include "DialogNewPoint.h"
#include "ui_DialogNewPoint.h"

DialogNewPoint::DialogNewPoint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogNewPoint)
{
    ui->setupUi(this);

    // start with all text selected
    ui->lineEditName->selectAll();
}

DialogNewPoint::~DialogNewPoint()
{
    delete ui;
}

void DialogNewPoint::SetName(const QString name)
{
    ui->lineEditName->setText(name);
}

QString DialogNewPoint::GetName()
{
    return ui->lineEditName->text();
}
