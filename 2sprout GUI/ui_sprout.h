/********************************************************************************
** Form generated from reading ui file 'sprout.ui'
**
** Created: Wed May 27 21:03:22 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SPROUT_H
#define UI_SPROUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_sproutClass
{
public:
    QWidget *centralWidget;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QGroupBox *groupBox;
    QTextEdit *textEdit;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QWidget *page_2;
    QGroupBox *groupBox_2;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLabel *label_4;
    QComboBox *comboBox;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QLineEdit *lineEdit_3;
    QLabel *label_5;
    QLabel *label_6;
    QLineEdit *lineEdit_4;
    QLineEdit *lineEdit_5;
    QCheckBox *checkBox;
    QPushButton *pushButton_4;
    QPushButton *pushButton_5;
    QPushButton *pushButton_6;

    void setupUi(QMainWindow *sproutClass)
    {
        if (sproutClass->objectName().isEmpty())
            sproutClass->setObjectName(QString::fromUtf8("sproutClass"));
        sproutClass->setEnabled(true);
        sproutClass->resize(464, 369);
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/icons/2sprout.png")), QIcon::Normal, QIcon::Off);
        sproutClass->setWindowIcon(icon);
        centralWidget = new QWidget(sproutClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        stackedWidget = new QStackedWidget(centralWidget);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        stackedWidget->setGeometry(QRect(0, -1, 471, 371));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        groupBox = new QGroupBox(page);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(9, 9, 451, 311));
        textEdit = new QTextEdit(groupBox);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setGeometry(QRect(13, 39, 421, 251));
        pushButton = new QPushButton(page);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(230, 330, 113, 32));
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(":/icons/started.png")), QIcon::Normal, QIcon::Off);
        pushButton->setIcon(icon1);
        pushButton_2 = new QPushButton(page);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(340, 330, 113, 32));
        QIcon icon2;
        icon2.addPixmap(QPixmap(QString::fromUtf8(":/icons/stop.png")), QIcon::Normal, QIcon::Off);
        pushButton_2->setIcon(icon2);
        pushButton_3 = new QPushButton(page);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        pushButton_3->setGeometry(QRect(10, 330, 113, 32));
        QIcon icon3;
        icon3.addPixmap(QPixmap(QString::fromUtf8(":/icons/sprites_09.png")), QIcon::Normal, QIcon::Off);
        pushButton_3->setIcon(icon3);
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        groupBox_2 = new QGroupBox(page_2);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(19, 9, 431, 311));
        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 40, 111, 17));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 70, 111, 17));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 100, 101, 17));
        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(310, 70, 41, 17));
        comboBox = new QComboBox(groupBox_2);
        QIcon icon4;
        icon4.addPixmap(QPixmap(QString::fromUtf8(":/icons/db_add.png")), QIcon::Normal, QIcon::Off);
        comboBox->addItem(icon4, QString());
        comboBox->addItem(icon4, QString());
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        comboBox->setGeometry(QRect(130, 30, 161, 26));
        comboBox->setMinimumContentsLength(0);
        comboBox->setFrame(true);
        lineEdit = new QLineEdit(groupBox_2);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(130, 70, 161, 22));
        lineEdit_2 = new QLineEdit(groupBox_2);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(130, 100, 161, 22));
        lineEdit_3 = new QLineEdit(groupBox_2);
        lineEdit_3->setObjectName(QString::fromUtf8("lineEdit_3"));
        lineEdit_3->setGeometry(QRect(350, 70, 61, 22));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 180, 81, 17));
        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 210, 61, 17));
        lineEdit_4 = new QLineEdit(groupBox_2);
        lineEdit_4->setObjectName(QString::fromUtf8("lineEdit_4"));
        lineEdit_4->setGeometry(QRect(130, 180, 161, 22));
        lineEdit_5 = new QLineEdit(groupBox_2);
        lineEdit_5->setObjectName(QString::fromUtf8("lineEdit_5"));
        lineEdit_5->setGeometry(QRect(130, 210, 161, 22));
        checkBox = new QCheckBox(groupBox_2);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(10, 250, 111, 21));
        pushButton_4 = new QPushButton(page_2);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));
        pushButton_4->setGeometry(QRect(340, 330, 113, 32));
        pushButton_4->setIcon(icon1);
        pushButton_5 = new QPushButton(page_2);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));
        pushButton_5->setGeometry(QRect(220, 330, 113, 32));
        QIcon icon5;
        icon5.addPixmap(QPixmap(QString::fromUtf8(":/icons/db_host.png")), QIcon::Normal, QIcon::Off);
        pushButton_5->setIcon(icon5);
        pushButton_6 = new QPushButton(page_2);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));
        pushButton_6->setGeometry(QRect(20, 330, 113, 32));
        pushButton_6->setIcon(icon3);
        stackedWidget->addWidget(page_2);
        sproutClass->setCentralWidget(centralWidget);

        retranslateUi(sproutClass);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(sproutClass);
    } // setupUi

    void retranslateUi(QMainWindow *sproutClass)
    {
        sproutClass->setWindowTitle(QApplication::translate("sproutClass", "2Sprout", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("sproutClass", "2sprout Status", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("sproutClass", "Start", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("sproutClass", "Stop", 0, QApplication::UnicodeUTF8));
        pushButton_3->setText(QApplication::translate("sproutClass", "Advanced", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("sproutClass", "Database", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("sproutClass", "Select Database:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("sproutClass", "Server Host:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("sproutClass", "Default Table:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("sproutClass", "Port:", 0, QApplication::UnicodeUTF8));
        comboBox->setItemText(0, QApplication::translate("sproutClass", "PostgreSQL", 0, QApplication::UnicodeUTF8));
        comboBox->setItemText(1, QApplication::translate("sproutClass", "MySQL", 0, QApplication::UnicodeUTF8));

        lineEdit->setText(QApplication::translate("sproutClass", "localhost", 0, QApplication::UnicodeUTF8));
        lineEdit_3->setText(QApplication::translate("sproutClass", "5432", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("sproutClass", "Username:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("sproutClass", "Password:", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("sproutClass", "Use Database", 0, QApplication::UnicodeUTF8));
        pushButton_4->setText(QApplication::translate("sproutClass", "Save", 0, QApplication::UnicodeUTF8));
        pushButton_5->setText(QApplication::translate("sproutClass", "Test", 0, QApplication::UnicodeUTF8));
        pushButton_6->setText(QApplication::translate("sproutClass", "Home", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(sproutClass);
    } // retranslateUi

};

namespace Ui {
    class sproutClass: public Ui_sproutClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SPROUT_H
