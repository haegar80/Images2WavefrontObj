#pragma once

#include "ImageProcessing/ImageProcessingFacade.h"
#include <QtWidgets/QMainWindow>

class QWidget;
class QPushButton;
class QListWidget;
class QPushButton;
class QLabel;
class QScrollArea;
class QCheckBox;
class QMenuBar;
class QStatusBar;

class Images2WavefrontObj : public QMainWindow
{
   Q_OBJECT

public:
    Images2WavefrontObj();
    virtual ~Images2WavefrontObj() = default;

    void setupUi();
    void retranslateUi();

public slots:
    void addImagesButton_clicked();
    void deleteImageButton_clicked();
    void scaleImagesCheckBox_clicked(bool p_isChecked);
    void loadSelectedImagesButton_clicked();

private:
    QWidget* m_centralwidget;
    QWidget* m_imageWidget;
    QPushButton* m_addImagesButton;
    QListWidget* m_listImagesWidget;
    QPushButton* m_deleteImageButton;
    QLabel* m_imageLabel;
    QScrollArea* m_imageLabelScrollArea;
    QCheckBox* m_checkBoxScaleImages;
    QPushButton* m_loadSelectedImagesButton;
    QPushButton* m_quitButton;
    QMenuBar* m_menubar;
    QStatusBar* m_statusbar;

    ImageProcessingFacade m_imageProcessingFacade;
    bool m_isScaleImagesChecked{ true };
    int m_maxScaledImageHeight{ 0 };

    void LoadImage();
};
