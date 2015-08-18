#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "videoprocessing.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();


public slots:
    void open();
   // void about();
    void exportdata();
    void videoFinished(bool);


private:
    void createActions();
    void createMenus();
    void loadFile(const QString &fileName);
    void closeEvent(QCloseEvent *e);

    QMenu *fileMenu;
    QMenu *helpMenu;
    QAction *openAct;
    QAction *exitAct;
    QAction* export2csv;
    QAction* write2Video;
    QAction *aboutAct;
    QAction *aboutDefaultSignalAct;
    QAction *aboutQtAct;

    QStringList _fileNames;
    VideoProcessing* _videoProcessing;
    bool _videoFinished;
    int _fileIndex;
};

#endif // MAINWINDOW_H
