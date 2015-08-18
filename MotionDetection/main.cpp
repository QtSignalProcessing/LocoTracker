#include "mainwindow.h"
#include "myapplication.h"


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>

#include <QFile>
#include <QTextStream>
#include"updatenew.h"
#include "featureextraction.h"
#include <QDebug>

#include <QMotifStyle>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
   //MyApplication a(argc,argv);
    MainWindow *w = new MainWindow();
    a.setApplicationName("LocoTracker");
    a.setStyle(new QMotifStyle);
    QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );
    w->show();
    return a.exec();
	/*cv::Mat f;
	f = cv::imread("C:\\Users\\Le\\Desktop\\beeProjectWin\\MotionDetection\\watershed.png");
	if(! f.data ) // Check for invalid input
    {
        qDebug() << "Could not open or find the image";
        return -1;
    }
	cv::imshow("sdf",f);
    cv::waitKey(0);*/
}

