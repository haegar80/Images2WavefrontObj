#include "UI/Images2WavefrontObj.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    const int cSize = 1000;

    Images2WavefrontObj mainWindow;

    mainWindow.show();
    return a.exec();
}
