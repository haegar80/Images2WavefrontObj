#include "Images2WavefrontObj.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
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
    m_imageWidget->setGeometry(QRect(60, 50, screenGeometry.width() - 80, screenGeometry.height() - 100));
    m_loadImagesButton = new QPushButton(m_imageWidget);
    m_loadImagesButton->setObjectName(QString::fromUtf8("loadImagesButton"));
    m_loadImagesButton->setGeometry(QRect(50, 30, 75, 23));
    m_listImagesWidget = new QListWidget(m_imageWidget);
    m_listImagesWidget->setObjectName(QString::fromUtf8("listImagesWidget"));
    m_listImagesWidget->setGeometry(QRect(140, 30, 256, 121));
    m_deleteImageButton = new QPushButton(m_imageWidget);
    m_deleteImageButton->setObjectName(QStringLiteral("deleteImageButton"));
    m_deleteImageButton->setGeometry(QRect(410, 30, 75, 23));
    m_imageLabel = new QLabel(m_imageWidget);
    m_imageLabel->setObjectName(QStringLiteral("imageLabel"));
    m_imageLabel->setGeometry(QRect(50, 180, screenGeometry.width() - 160, screenGeometry.height() - 280));
    m_imageLabel->setFrameShape(QFrame::Box);
    m_imageLabel->setScaledContents(true);
    m_quitButton = new QPushButton(m_centralwidget);
    m_quitButton->setObjectName(QString::fromUtf8("quitButton"));
    m_quitButton->setGeometry(QRect(20, screenGeometry.height() - 73, 75, 23));
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
    QObject::connect(m_deleteImageButton, SIGNAL(pressed()), this, SLOT(deleteImageButton_clicked()));
    QObject::connect(m_quitButton, SIGNAL(pressed()), this, SLOT(close()));
}

void Images2WavefrontObj::retranslateUi()
{
    this->setWindowTitle(QApplication::translate("MainWindow", "Images2WavefrontObj", 0));
    m_loadImagesButton->setText(QApplication::translate("MainWindow", "Load Images", 0));
    m_deleteImageButton->setText(QApplication::translate("MainWindow", "Delete Image", 0));
    m_imageLabel->setText(QString());
    m_quitButton->setText(QApplication::translate("MainWindow", "Quit", 0));
}

void Images2WavefrontObj::loadImageButton_clicked()
{
    QStringList openFileNames = QFileDialog::getOpenFileNames(this, "Images", "", "Images Files (*.png *.bmp *.jpg *.raw)");
    m_listImagesWidget->addItems(openFileNames);
}

void Images2WavefrontObj::deleteImageButton_clicked()
{
    QList<QListWidgetItem*> items = m_listImagesWidget->selectedItems();
    for(QListWidgetItem* item : items)
    {
        delete m_listImagesWidget->takeItem(m_listImagesWidget->row(item));
    }
}