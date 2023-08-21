#include <QApplication>
#include "Client.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Client client("localhost", 2323);
//26.7.166.31
    //PlusMinus pl;
//pl.show();
//pl.grid->addWidget(&client, 1, 0);
    client.show();

    return a.exec();
}
