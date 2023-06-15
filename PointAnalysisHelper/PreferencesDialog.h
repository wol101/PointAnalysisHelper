#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QColor>

#include "Preferences.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

    void setValues(const Preferences &prefs);
    void getValues(Preferences *prefs);

    static QColor getIdealTextColor(const QColor& rBackgroundColor);
    static QColor getAlphaColorHint(const QColor& Color);

public slots:
    void markerColorClicked();
    void scaleBarColorClicked();

private:
    void FixSizes();

    Ui::PreferencesDialog *ui;

    QColor m_markerColor;
    QColor m_scaleBarColor;

};

#endif // PREFERENCESDIALOG_H
