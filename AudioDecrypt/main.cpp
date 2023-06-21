#include "stdafx.h"
#include "AudioDecrypt.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AudioDecrypt w;
    w.show();
    return a.exec();
}
