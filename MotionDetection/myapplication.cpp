#include "myapplication.h"

#include <QDebug>

MyApplication::MyApplication(int &argc, char ** argv) :
    QApplication(argc,argv)
{
}

bool MyApplication::notify(QObject *receiver, QEvent *event)
{
    try {
        return QApplication::notify( receiver, event );
    } catch ( std::exception& e ) {
        qDebug()<<"what the fuck "<<e.what() ;
        return false;
    }
}
