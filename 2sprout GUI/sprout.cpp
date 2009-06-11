#include "sprout.h"
#include "ui_sprout.h"
#include <qfile.h>
#include <iostream>
#include <qtextstream.h>

sprout::sprout(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::sproutClass)
{
    ui->setupUi(this);
}

sprout::~sprout()
{
    delete ui;
}

void sprout::on_pushButton_3_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

void sprout::on_pushButton_6_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page);
}


/*
 This is for the saved button on the advanced menu for the database
 */
void sprout::on_pushButton_4_clicked()
{
    QFile f("2sprout.conf");
    if(f.open(QFile::WriteOnly | QFile::Append))
    {
        QTextStream out(&f);

    }
    else
    {
        ui->textEdit->setText("Could Not Open File");
     }


}
