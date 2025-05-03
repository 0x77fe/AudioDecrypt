#include "MusicDecryptor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MusicDecryptor w;
    w.show();
    return a.exec();
}
