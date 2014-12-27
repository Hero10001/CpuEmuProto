#include <QCoreApplication>
#include <ram.h>
#include <prom.h>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    uchar u=0xE4;
    uchar b=u+0x64;
    qDebug()<<b;
    return 0;
}
