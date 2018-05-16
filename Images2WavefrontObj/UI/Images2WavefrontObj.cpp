#include "Images2WavefrontObj.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

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
    m_addImagesButton = new QPushButton(m_imageWidget);
    m_addImagesButton->setObjectName(QString::fromUtf8("addImagesButton"));
    m_addImagesButton->setGeometry(QRect(50, 30, 75, 23));
    m_listImagesWidget = new QListWidget(m_imageWidget);
    m_listImagesWidget->setObjectName(QString::fromUtf8("listImagesWidget"));
    m_listImagesWidget->setGeometry(QRect(140, 30, 271, 131));
    m_listImagesWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_deleteImageButton = new QPushButton(m_imageWidget);
    m_deleteImageButton->setObjectName(QStringLiteral("deleteImageButton"));
    m_deleteImageButton->setGeometry(QRect(50, 60, 75, 23));
    m_maxScaledImageHeight = screenGeometry.height() - 320;
    m_imageLabel = new QLabel(m_imageWidget);
    m_imageLabel->setObjectName(QStringLiteral("imageLabel"));
    m_imageLabel->setGeometry(QRect(50, 220, screenGeometry.width() - 160, m_maxScaledImageHeight));
    m_imageLabel->setFrameShape(QFrame::Box);
    m_imageLabel->setScaledContents(false);
    m_imageLabelScrollArea = new QScrollArea(m_imageWidget);
    m_imageLabelScrollArea->setGeometry(QRect(50, 220, screenGeometry.width() - 160, m_maxScaledImageHeight));
    m_imageLabelScrollArea->setWidget(m_imageLabel);
    m_imageLabelScrollArea->setBackgroundRole(QPalette::Dark);
    m_checkBoxScaleImages = new QCheckBox(m_imageWidget);
    m_checkBoxScaleImages->setObjectName(QStringLiteral("checkBoxScaleImages"));
    m_checkBoxScaleImages->setGeometry(QRect(280, 180, 131, 21));
    m_checkBoxScaleImages->setChecked(true);
    m_loadSelectedImagesButton = new QPushButton(m_imageWidget);
    m_loadSelectedImagesButton->setObjectName(QStringLiteral("loadSelectedImagesButton"));
    m_loadSelectedImagesButton->setGeometry(QRect(140, 180, 121, 23));
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

    QObject::connect(m_addImagesButton, SIGNAL(pressed()), this, SLOT(addImagesButton_clicked()));
    QObject::connect(m_deleteImageButton, SIGNAL(pressed()), this, SLOT(deleteImageButton_clicked()));
    QObject::connect(m_checkBoxScaleImages, SIGNAL(clicked(bool)), this, SLOT(scaleImagesCheckBox_clicked(bool)));
    QObject::connect(m_loadSelectedImagesButton, SIGNAL(pressed()), this, SLOT(loadSelectedImagesButton_clicked()));
    QObject::connect(m_quitButton, SIGNAL(pressed()), this, SLOT(close()));
}

void Images2WavefrontObj::retranslateUi()
{
    this->setWindowTitle(QApplication::translate("MainWindow", "Images2WavefrontObj", 0));
    m_addImagesButton->setText(QApplication::translate("MainWindow", "Add Images", 0));
    m_deleteImageButton->setText(QApplication::translate("MainWindow", "Delete Image", 0));
    m_imageLabel->setText(QString());
    m_checkBoxScaleImages->setText(QApplication::translate("MainWindow", "Scale Displayed Images", nullptr));
    m_loadSelectedImagesButton->setText(QApplication::translate("MainWindow", "Load Selected Images", nullptr));
    m_quitButton->setText(QApplication::translate("MainWindow", "Quit", 0));
}

void Images2WavefrontObj::addImagesButton_clicked()
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

void Images2WavefrontObj::scaleImagesCheckBox_clicked(bool p_isChecked)
{
    m_isScaleImagesChecked = p_isChecked;
    LoadImage();
}

void Images2WavefrontObj::loadSelectedImagesButton_clicked()
{
    LoadImage();
}

void Images2WavefrontObj::LoadImage()
{
    QList<QListWidgetItem*> items = m_listImagesWidget->selectedItems();
    QStringList itemsStringList;
    for (QListWidgetItem* item : items)
    {
        itemsStringList.push_back(item->text());
    }

    QImage image = m_imageProcessingFacade.CombineImages(itemsStringList);

    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
            tr("Cannot load or combine the selected images"));
    }
    else
    {
        if (m_isScaleImagesChecked) {
            double ratio = static_cast<double>(image.width()) / static_cast<double>(image.height());
            m_imageLabel->setGeometry(QRect(50, 220, ratio * m_maxScaledImageHeight, m_maxScaledImageHeight));
            m_imageLabelScrollArea->setGeometry(QRect(50, 220, ratio * m_maxScaledImageHeight, m_maxScaledImageHeight));
        }
        else {
            QRect screenGeometry = QApplication::desktop()->availableGeometry();
            m_imageLabel->setGeometry(QRect(50, 220, image.width(), image.height()));
            m_imageLabelScrollArea->setGeometry(QRect(50, 220, screenGeometry.width() - 160, m_maxScaledImageHeight));
        }
        m_imageLabel->setScaledContents(m_isScaleImagesChecked);
        m_imageLabel->setPixmap(QPixmap::fromImage(image));
    }
}