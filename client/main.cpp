#include "tcpclient.h"
#include "protoclo.h"
#include <QApplication>
#include "loginwidget.h"
#include "mainwidget.h"


int main(int argc, char *argv[])
{



    QApplication a(argc, argv);

    TCPClient::getInstance();

    return a.exec();
}
