#ifndef DIALOGEXPORTIMAGE_H
#define DIALOGEXPORTIMAGE_H

#include <QDialog>

namespace Ui {
class DialogExportImage;
}

class Preferences;

class DialogExportImage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogExportImage(QWidget *parent = 0);
    ~DialogExportImage();

    void setValues(const Preferences &prefs);
    void getValues(Preferences *prefs);

public slots:
    void folderPathBrowse();

private:
    Ui::DialogExportImage *ui;
};

#endif // DIALOGEXPORTIMAGE_H
