#ifndef DIALOGNEWPOINT_H
#define DIALOGNEWPOINT_H

#include <QDialog>

namespace Ui {
class DialogNewPoint;
}

class DialogNewPoint : public QDialog
{
    Q_OBJECT

public:
    explicit DialogNewPoint(QWidget *parent = 0);
    ~DialogNewPoint();

    void SetName(const QString name);
    QString GetName();

private:
    Ui::DialogNewPoint *ui;
};

#endif // DIALOGNEWPOINT_H
