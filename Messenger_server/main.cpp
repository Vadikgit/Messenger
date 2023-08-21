#include <QApplication>
#include <QtGui>
#include "Server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Server     srvr(2323);
    //PlusMinus pl;
    //pl.show();
    //pl.grid->addWidget(&srvr, 1, 0);
    srvr.show();


    return a.exec();
}
