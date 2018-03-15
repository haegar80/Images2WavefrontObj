#pragma once

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMenuBar>

class Images2WavefrontObj : public QMainWindow
{
    Q_OBJECT

public:
    Images2WavefrontObj();
    virtual ~Images2WavefrontObj() = default;

    void setupUi();
    void retranslateUi();

private:
    QWidget* m_centralwidget;
    QWidget* m_widget;
    QPushButton* m_quitButton;
    QMenuBar* m_menubar;
    QStatusBar* m_statusbar;
};
