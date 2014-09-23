/********************************************************************************
** Form generated from reading UI file 'peerclientui.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PEERCLIENTUI_H
#define UI_PEERCLIENTUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PeerClientUIClass
{
public:
    QWidget *centralWidget;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QLineEdit *server_lineEdit;
    QLineEdit *port_lineEdit;
    QLabel *server_label;
    QLabel *port_label;
    QPushButton *ConnectButton;
    QWidget *page_2;
    QListView *peerlistView;
    QPushButton *talkButton;
    QPushButton *disconnectButton;
    QPushButton *clearlogButton;
    QTextEdit *logtextEdit;

    void setupUi(QMainWindow *PeerClientUIClass)
    {
        if (PeerClientUIClass->objectName().isEmpty())
            PeerClientUIClass->setObjectName(QStringLiteral("PeerClientUIClass"));
        PeerClientUIClass->resize(600, 400);
        PeerClientUIClass->setMinimumSize(QSize(600, 400));
        PeerClientUIClass->setMaximumSize(QSize(600, 400));
        QIcon icon;
        icon.addFile(QStringLiteral(":/PeerClientUI/Resources/wlm.ico"), QSize(), QIcon::Normal, QIcon::Off);
        PeerClientUIClass->setWindowIcon(icon);
        PeerClientUIClass->setIconSize(QSize(36, 36));
        centralWidget = new QWidget(PeerClientUIClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        stackedWidget = new QStackedWidget(centralWidget);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        stackedWidget->setGeometry(QRect(0, -20, 600, 420));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy);
        page = new QWidget();
        page->setObjectName(QStringLiteral("page"));
        server_lineEdit = new QLineEdit(page);
        server_lineEdit->setObjectName(QStringLiteral("server_lineEdit"));
        server_lineEdit->setGeometry(QRect(200, 190, 251, 20));
        port_lineEdit = new QLineEdit(page);
        port_lineEdit->setObjectName(QStringLiteral("port_lineEdit"));
        port_lineEdit->setGeometry(QRect(200, 240, 113, 20));
        server_label = new QLabel(page);
        server_label->setObjectName(QStringLiteral("server_label"));
        server_label->setGeometry(QRect(140, 190, 54, 12));
        port_label = new QLabel(page);
        port_label->setObjectName(QStringLiteral("port_label"));
        port_label->setGeometry(QRect(140, 240, 54, 20));
        ConnectButton = new QPushButton(page);
        ConnectButton->setObjectName(QStringLiteral("ConnectButton"));
        ConnectButton->setGeometry(QRect(200, 280, 75, 23));
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QStringLiteral("page_2"));
        peerlistView = new QListView(page_2);
        peerlistView->setObjectName(QStringLiteral("peerlistView"));
        peerlistView->setGeometry(QRect(0, 20, 511, 191));
		peerlistView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        talkButton = new QPushButton(page_2);
        talkButton->setObjectName(QStringLiteral("talkButton"));
        talkButton->setGeometry(QRect(520, 30, 75, 23));
        disconnectButton = new QPushButton(page_2);
        disconnectButton->setObjectName(QStringLiteral("disconnectButton"));
        disconnectButton->setGeometry(QRect(520, 70, 75, 23));
        clearlogButton = new QPushButton(page_2);
        clearlogButton->setObjectName(QStringLiteral("clearlogButton"));
        clearlogButton->setGeometry(QRect(520, 110, 75, 23));
        logtextEdit = new QTextEdit(page_2);
        logtextEdit->setObjectName(QStringLiteral("logtextEdit"));
        logtextEdit->setGeometry(QRect(0, 216, 511, 201));
        logtextEdit->setReadOnly(true);
		logtextEdit->setFontPointSize(10);
        stackedWidget->addWidget(page_2);
        PeerClientUIClass->setCentralWidget(centralWidget);

        retranslateUi(PeerClientUIClass);

        stackedWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(PeerClientUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *PeerClientUIClass)
    {
        PeerClientUIClass->setWindowTitle(QApplication::translate("PeerClientUIClass", "PeerClientUI", 0));
        server_label->setText(QApplication::translate("PeerClientUIClass", "server:", 0));
        port_label->setText(QApplication::translate("PeerClientUIClass", "port:", 0));
        ConnectButton->setText(QApplication::translate("PeerClientUIClass", "Connect", 0));
        talkButton->setText(QApplication::translate("PeerClientUIClass", "Talk", 0));
        disconnectButton->setText(QApplication::translate("PeerClientUIClass", "Disconnect", 0));
        clearlogButton->setText(QApplication::translate("PeerClientUIClass", "Clearlog", 0));
    } // retranslateUi

};

namespace Ui {
    class PeerClientUIClass: public Ui_PeerClientUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PEERCLIENTUI_H
