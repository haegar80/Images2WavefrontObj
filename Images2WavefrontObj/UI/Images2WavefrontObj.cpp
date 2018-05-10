#include "Images2WavefrontObj.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QFileDialog>

Images2WavefrontObj::Images2WavefrontObj()
{
    setupUi();
}

void Images2WavefrontObj::setupUi()
{
    if (this->objectName().isEmpty()) {
        this->setObjectName(QString::fromUtf8("MainWindow"));
    }
    
    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    this->resize(screenGeometry.width(), screenGeometry.height());
    this->setMinimumSize(QSize(200, 200));

    m_centralwidget = new QWidget(this);
    m_centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    m_imageWidget = new QWidget(m_centralwidget);
    m_imageWidget->setObjectName(QString::fromUtf8("imageWidget"));
    m_imageWidget->setGeometry(QRect(60, 50, 581, 411));
    m_loadImagesButton = new QPushButton(m_imageWidget);
    m_loadImagesButton->setObjectName(QString::fromUtf8("loadImagesButton"));
    m_loadImagesButton->setGeometry(QRect(40, 30, 75, 23));
    m_listImagesWidget = new QListWidget(m_imageWidget);
    m_listImagesWidget->setObjectName(QString::fromUtf8("listImagesWidget"));
    m_listImagesWidget->setGeometry(QRect(160, 30, 256, 192));
    m_quitButton = new QPushButton(m_centralwidget);
    m_quitButton->setObjectName(QString::fromUtf8("quitButton"));
    m_quitButton->setGeometry(QRect(20, 620, 75, 23));
    this->setCentralWidget(m_centralwidget);
    m_menubar = new QMenuBar(this);
    m_menubar->setObjectName(QString::fromUtf8("menubar"));
    m_menubar->setGeometry(QRect(0, 0, 800, 21));
    this->setMenuBar(m_menubar);
    m_statusbar = new QStatusBar(this);
    m_statusbar->setObjectName(QString::fromUtf8("statusbar"));
    this->setStatusBar(m_statusbar);

    retranslateUi();

    QObject::connect(m_loadImagesButton, SIGNAL(pressed()), this, SLOT(loadImageButton_clicked()));
    QObject::connect(m_quitButton, SIGNAL(pressed()), this, SLOT(close()));
}

void Images2WavefrontObj::retranslateUi()
{
    this->setWindowTitle(QApplication::translate("MainWindow", "Images2WavefrontObj", 0));
    m_loadImagesButton->setText(QApplication::translate("MainWindow", "Load Images", 0));
    m_quitButton->setText(QApplication::translate("MainWindow", "Quit", 0));
}

void Images2WavefrontObj::loadImageButton_clicked()
{
    QStringList openFileNames = QFileDialog::getOpenFileNames(this, "Images", "", "Images Files (*.png *.bmp *.jpg *.raw)");
}
