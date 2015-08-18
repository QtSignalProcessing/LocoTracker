#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>

class MyApplication : public QApplication
{
    Q_OBJECT
public:
    explicit MyApplication(int &argc, char ** argv);
    
signals:
    
public slots:
private:
    bool notify(QObject *receiver_, QEvent *event_);
    
};

#endif // MYAPPLICATION_H
