#include "mainwindow.h"

#include <QAction>
#include <QMenuBar>
#include <QFileDialog>
#include <QCoreApplication>
#include <QTimer>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),_fileNames("130529_ok_Trace Conditioning_part1_shot2.avi"),_videoProcessing( new VideoProcessing(_fileNames[0],this)),_videoFinished(false),_fileIndex(0)
{
    createActions();
    createMenus();
    connect(_videoProcessing,SIGNAL(videoFinished(bool)),this,SLOT(videoFinished(bool)));

   //connect(_videoProcessing, SIGNAL(cancel()), this, SLOT(CoreApplication::quit()));
    setCentralWidget(_videoProcessing);
    this->setWindowTitle(_fileNames[0]+" LocoTracking");
}

MainWindow::~MainWindow(){}

void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
   // export2csv = new QAction(tr("Export &data to CSV"), this);
   // connect(export2csv,SIGNAL(triggered()),this,SLOT(exportdata()));
   // write2Video = new QAction(tr("Write final result to video"), this);
   // connect(write2Video,SIGNAL(triggered()),_videoProcessing,SLOT(write2video()));
    aboutDefaultSignalAct =new QAction(tr("&About Default Signal"), this);
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(exitAct);
   // fileMenu->addAction(export2csv);
   // fileMenu->addAction(write2Video);
    menuBar()->addSeparator();
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutDefaultSignalAct);
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::exportdata()
{
    //_videoProcessing->exportData2CSV();
}

void MainWindow::videoFinished(bool f)
{
    _videoFinished = f;
    if (!_fileNames.isEmpty() && (_fileIndex < _fileNames.length()) &&!_fileNames[_fileIndex].isEmpty() && _fileNames.size() != 1)
    {
        loadFile(_fileNames[_fileIndex]);
        _fileIndex++;
        _videoProcessing->play();
    }
    /*else if(!_fileNames.isEmpty())
    {
        _fileIndex = 0;
        if( !_fileNames[_fileIndex].isEmpty())
        {
            loadFile(_fileNames[_fileIndex]);
            _fileIndex++;
        }
    }*/
}

void MainWindow::open()
{
    _fileIndex = 0;
    _fileNames.clear();
    _fileNames = QFileDialog::getOpenFileNames(
                             this,
                             "Select one or more files to open",
                             ".",
                             "Videos (*.avi *.rmvb *.mkv *.mp4)");

    if (!_fileNames.isEmpty() && !_fileNames[_fileIndex].isEmpty())
    {
        loadFile(_fileNames[_fileIndex]);
        _fileIndex++;
    }
}

void MainWindow::loadFile(const QString &fileName)
{
    this->setWindowTitle(fileName+" LocoTracking");
    _videoProcessing->setFileName(fileName);
    _videoProcessing->stop();
}

void MainWindow::closeEvent(QCloseEvent*)
{
    QTimer::singleShot(0, this, SLOT(close()));
}
