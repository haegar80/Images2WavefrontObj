#pragma once

#include <QtWidgets/QMainWindow>

class QWidget;
class QPushButton;
class QListWidget;
class QPushButton;
class QLabel;
class QScrollArea;
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
    void loadImageButton_clicked();
    void deleteImageButton_clicked();

private:
    QWidget* m_centralwidget;
    QWidget* m_imageWidget;
    QPushButton* m_loadImagesButton;
    QListWidget* m_listImagesWidget;
    QPushButton* m_deleteImageButton;
    QLabel* m_imageLabel;
    QScrollArea* m_imageLabelScrollArea;
    QPushButton* m_quitButton;
    QMenuBar* m_menubar;
    QStatusBar* m_statusbar;
};
