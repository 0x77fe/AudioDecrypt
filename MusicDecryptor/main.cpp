#include "MusicDecryptor.h"
#include <QtWidgets/QApplication>
#if _DEBUG
#include <qdebug.h>
#include ".\Private\KGG.h"
void udebug()
{
}
#endif
int main(int argc, char *argv[])
{
#if _DEBUG
    // 测试单元
    //udebug();
#endif
    QApplication a(argc, argv);
    MusicDecryptor w;
    w.show();
    return a.exec();
}
