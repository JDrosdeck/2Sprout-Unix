#ifndef SPROUT_H
#define SPROUT_H

#include <QtGui/QMainWindow>

namespace Ui
{
    class sproutClass;
}

class sprout : public QMainWindow
{
    Q_OBJECT

public:
    sprout(QWidget *parent = 0);
    ~sprout();

private:
    Ui::sproutClass *ui;

private slots:
    void on_pushButton_4_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_3_clicked();
};

#endif // SPROUT_H
