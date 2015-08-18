#include "videoprocessing.h"

#include "displaywidget.h"
#include "initialprob.h"
#include "updateprob.h"
#include "displayresult.h"
#include "updatenew.h"
#include "iohandle.h"
#include "rectifywidget.h"

#include <QtGui>

#include <vector>
#include <string>
#include <iostream>

#include <QDebug>
int k = 0;
using namespace std;

const int MINIMUM_HEIGHT = 40;
const int MINIMUM_WIDTH = 30;
const int MINIMUM_RHEIGHT = 0;
const int MINIMUM_RWIDTH = 10;
const int MAX_HEIGHT = 100;
const int MAX_WIDTH = 100;
const float FACTOR = 0.3;
const float THRESHOLD = 0.5;
const int BACKWARDPIX = 10;
int numFrames;


VideoProcessing::VideoProcessing(QString fileName,QWidget *parent) :
    QWidget(parent),_foreGround(new DisplayWidget)/*,_backGround(new DisplayWidget)*/,_play(false),_fileName(fileName),_cap(),_lightLabel(new QLabel(this)),_grayLabel(new QLabel(this)),
    _playButton(new QPushButton("Play",this)),_pauseButton(new QPushButton("Pause",this)),_stopButton(new QPushButton("Stop",this)),
    _lighting(false),_startPt(),_endPt(),_firstTime(true),_bg(new cv::BackgroundSubtractorMOG2()),_mandible(),_features(),_preFrame(),_preprocess(new Preprocess()),
    _numFrames(0),_currentFrame(0),_slider(new QSlider(this)),_spinBox(new QSpinBox(this)),_selectBBMode(false),_selectLightMode(new QCheckBox("Select Light Mode",this)),
    _selectBoundingBox(new QCheckBox("Select Boundingbox Mode",this)),_trainingMode(false),_trainingWidget(new TrainingWidget(this)),_frame(),_training(new QCheckBox("Traing",this)),
    _alreadyTrained(false),_classifier(NULL),_legalBoundRectVec(),_processed(false),_centroid(),_colorIndex(),_rectifyMode(false),_numOfKeyFrames(0),_probList(),
    _KFLabel(new QLabel("Number of Key Frames:",this)),_KFNumLabel(new QLabel("",this)),_KFList(),_rCF(0),_localIndex(-1),_rightFrame(new QPushButton("Go to the next KF",this))
  ,_rectifyBox(new QCheckBox("Correction Mode",this)),_trackListNew(),_lightFrameNoVec(),_storeUpdated(new QPushButton("Store updated data",this)),
  _showFinalResult(new QPushButton("Show result",this)),_renderTip(false)
{
    //Initilization
    setMinimumSize(500,500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _preprocess->computeMeanAndStd(_fileName,20);
    _cap.open(_fileName.toAscii().data());
    _numFrames = _cap.get(CV_CAP_PROP_FRAME_COUNT);
    _cap >> _frame;
    playVideo(_frame);
    connect(_storeUpdated,SIGNAL(clicked()),this,SLOT(storeUpdated()));
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
   /* QHBoxLayout* fileLayout = new QHBoxLayout;
    QLabel* fName = new QLabel("FIle name:",this);
    fName->setFixedSize(50,20);
    _fileLabel->setFixedSize(100,20);
    fileLayout->addWidget(fName);
    fileLayout->addWidget(_fileLabel);
    mainLayout->addLayout(fileLayout);*/
    QHBoxLayout* trainRecLayout = new QHBoxLayout;
    trainRecLayout->addWidget(_training);
    trainRecLayout->addWidget(_rightFrame);
    _rightFrame->setVisible(false);
    connect(_rightFrame,SIGNAL(clicked()),this,SLOT(currentFrameReady()));
    connect(_training,SIGNAL(toggled(bool)),this,SLOT(changeMode(bool)));
    connect(_training,SIGNAL(toggled(bool)),_foreGround,SLOT(setTrainingMode(bool)));
    connect(_rectifyBox,SIGNAL(toggled(bool)),this,SLOT(rectifyMode(bool)));
    connect(_rectifyBox,SIGNAL(toggled(bool)),_foreGround,SLOT(setRectifyMode(bool)));
    connect(_foreGround,SIGNAL(pointSelected()),this,SLOT(readyToModify()));
    connect(_foreGround,SIGNAL(newBBAdded()),this,SLOT(addNewBBox()));
    trainRecLayout->addWidget(_rectifyBox);
    mainLayout->addLayout(trainRecLayout);
    _KFLabel->setVisible(false);
    _KFNumLabel->setVisible(false);
    QHBoxLayout* keyFrameLayout = new QHBoxLayout;
    _KFLabel->setFixedHeight(20);
    _KFNumLabel->setFixedHeight(20);
    keyFrameLayout->addWidget(_KFLabel);
    keyFrameLayout->addWidget(_KFNumLabel);
    keyFrameLayout->addWidget(_storeUpdated);
    keyFrameLayout->addWidget(_showFinalResult);
    connect(_showFinalResult,SIGNAL(clicked()),this,SLOT(drawTip()));
    mainLayout->addLayout(keyFrameLayout);
    QHBoxLayout* checkBoxLayout = new QHBoxLayout;
    _selectLightMode->setChecked(true);
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(_selectLightMode);
    buttonGroup->addButton(_selectBoundingBox);
    buttonGroup->setExclusive(true);
    checkBoxLayout->addWidget(_selectLightMode);
    checkBoxLayout->addWidget(_selectBoundingBox);
    connect(_selectBoundingBox,SIGNAL(clicked(bool)),this,SLOT(changeSelectMode()));
    connect(_selectLightMode,SIGNAL(clicked(bool)),this,SLOT(changeSelectMode()));
    _slider->setOrientation(Qt::Horizontal);
    _slider->setRange(0,_numFrames);
    _spinBox->setRange(0,_numFrames);
    QHBoxLayout* sliderLayout = new QHBoxLayout;
    sliderLayout->addWidget(_slider);
    sliderLayout->addWidget(_spinBox);
    connect(_slider,SIGNAL(valueChanged(int)),_spinBox,SLOT(setValue(int)));
    connect(_spinBox,SIGNAL(valueChanged(int)),_slider,SLOT(setValue(int)));
    connect(_slider,SIGNAL(valueChanged(int)),this,SLOT(setFramePosion(int)));
    connect(_spinBox,SIGNAL(valueChanged(int)),this,SLOT(setFramePosion(int)));
    mainLayout->addLayout(checkBoxLayout);
    mainLayout->addLayout(sliderLayout);
    QHBoxLayout* videoLayout = new QHBoxLayout();
    videoLayout->addWidget(_foreGround);
   // videoLayout->addWidget(_backGround);
    _trainingWidget->setVisible(false);
    mainLayout->addLayout(videoLayout);
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(_playButton);
    connect(_playButton,SIGNAL(clicked()),this,SLOT(play()));
    buttonLayout->addWidget(_pauseButton);
    _pauseButton->setVisible(false);
    connect(_pauseButton,SIGNAL(clicked()),this,SLOT(pause()));
    buttonLayout->addWidget(_stopButton);
    connect(_stopButton,SIGNAL(clicked()),this,SLOT(stop()));
    _stopButton->setVisible(false);
    mainLayout->addLayout(buttonLayout);
    connect(_foreGround,SIGNAL(lightRegionSelected()),this,SLOT(showLight()));
    _lightLabel->setText("light");
    QHBoxLayout* labelLayout = new QHBoxLayout;
    _grayLabel->setText("gray img of bee head");
    //connect(_backGround,SIGNAL(lightRegionSelected()),this,SLOT(showGray()));
    labelLayout->addWidget(_lightLabel);
    labelLayout->addWidget(_grayLabel);
    mainLayout->addLayout(labelLayout);
    connect(_trainingWidget,SIGNAL(closed(bool)),this,SLOT(changeMode(bool)));
    connect(_trainingWidget,SIGNAL(traingFinish()),this,SLOT(trainingFinish()));
    this->setLayout(mainLayout);
    initializeData();

    startup();
}

VideoProcessing::~VideoProcessing()
{
    delete _bg;
    delete _preprocess;
    delete _trainingWidget;
}

void VideoProcessing::storeUpdated()
{
    IOHandle io;
    QString dirName = _fileName;
    int ind = _fileName.indexOf(".");
    dirName.remove(ind,_fileName.length()-1);
    io.writeLegalRect2File(_legalBoundRectVec,dirName);
    io.writeFeature2File(_features,dirName);
    io.writeManAndcen2File(_mandible,_centroid,dirName);
    io.writeProb2file(_probList,dirName);
    io.writeSeq2File(_colorIndex,dirName);
    QMessageBox msgBox;
    msgBox.setText("Writing finished!");

    QString s("OK");
    QPushButton* yesButton = msgBox.addButton(s,QMessageBox::AcceptRole);
     msgBox.setDefaultButton(yesButton);
    msgBox.exec();
}

void VideoProcessing::write2video()
{

    cv::VideoCapture cap;
    cap.open(_fileName.toAscii().data());
    cap.set(CV_CAP_PROP_POS_FRAMES,0);
    cv::Mat frame1;
    cv::Size S = cv::Size((int) _cap.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                   (int) _cap.get(CV_CAP_PROP_FRAME_HEIGHT));

    cv::VideoWriter outputVideo;
    int ind = _fileName.indexOf(".");
    QString fname = _fileName;
    fname.remove(ind,_fileName.length()-1);
    fname += "_final.mkv";
    outputVideo.open(fname.toAscii().data(), CV_FOURCC('P','I','M','1'), _cap.get(CV_CAP_PROP_FPS), S, true);
    QProgressDialog p("Writing...","cancel",0,_numFrames,this);;
    p.setRange(1,_numFrames);
    for( _currentFrame = 0; _currentFrame < _numFrames; _currentFrame++ )
    {
        cap >> frame1;
        for( uint i = 0; i < _legalBoundRectVec[_currentFrame].size(); i++ )
        {
            if( _legalBoundRectVec[_currentFrame][i].width > 0 && _legalBoundRectVec[_currentFrame][i].height > 0 )
            {
                cv::Scalar color(0,0,255);
                if( !_trainingMode )
                {
                    switch (_colorIndex[_currentFrame][i])
                    {
                    case 1:
                        color = cv::Scalar(0,165,255);// upper antena
                        break;
                    case 2:
                        color = cv::Scalar(240,32,160);// mandible
                        break;
                    case 3:
                        color = cv::Scalar(0,0,255);//r tounge
                        break;
                    case 4:
                        color = cv::Scalar(0,255,0);// bottom mandible
                        break;
                    case 5:
                        color = cv::Scalar(255,0,0);//bottom antenna
                        break;
                    }
                    int index = _colorIndex[_currentFrame][i]-1;
                    if( index < 0 )
                        index = 5;
                    if( _renderTip && _trackListNew.size() > 0 )
                    {
                        cv::Point center( _trackListNew[index].at<double>(_currentFrame,9), _trackListNew[index].at<double>(_currentFrame,10) );
                        cv::circle(frame1,center,5,color,-1);
                    }
                    QString label = QString::number(_colorIndex[_currentFrame][i]);
                    cv::Point org =  (_legalBoundRectVec[_currentFrame][i].tl());
                    org.x -= 10;
                    cv::putText(frame1,label.toStdString(),org,cv::FONT_HERSHEY_PLAIN,1.5,color);
                }
                rectangle( frame1, _legalBoundRectVec[_currentFrame][i].tl(), _legalBoundRectVec[_currentFrame][i].br(), color, 2, 8, 0 );
            }
        }
        p.setValue(_currentFrame);
        outputVideo << frame1;
    }
    _currentFrame = 0;
    cap.release();
}

void VideoProcessing::closeEvent(QCloseEvent *e)
{
    e->accept();
    qApp->quit();
}

void VideoProcessing::play()
{
    _play = true;
    int i =0;
    _cap.set(CV_CAP_PROP_POS_FRAMES,_currentFrame);
    while(_play)
    {
        if(!_cap.read(_frame))
        {
            break;
        }
        if(i==0)
        {
            _playButton->setVisible(false);
            _pauseButton->setVisible(true);
            _stopButton->setVisible(true);
            _firstTime = true;
            i++;
        }
         playVideo(_frame);
         if(cv::waitKey(300) >= 0)
         {
             break;
         }
    }
    if(!_cap.grab())
    {
        emit videoFinished(true);
        _cap.open(_fileName.toAscii().data());
        _numFrames = _cap.get(CV_CAP_PROP_FRAME_COUNT);
        _currentFrame = 0;
        _slider->setRange(0,_numFrames);
        _spinBox->setRange(0,_numFrames);
        _slider->setValue(_currentFrame);
        _playButton->setVisible(true);
        _pauseButton->setVisible(false);
        _stopButton->setVisible(false);
    }
}

void VideoProcessing::pause()
{
    _play = false;
   // _currentFrame--;
    _playButton->setVisible(true);
    _pauseButton->setVisible(false);
    _stopButton->setVisible(true);
}

void VideoProcessing::stop()
{
    _play = false;
    _cap.release();
    _cap.open(_fileName.toAscii().data());
    _numFrames = _cap.get(CV_CAP_PROP_FRAME_COUNT);
    _currentFrame = 0;
    _slider->setRange(0,_numFrames);
    _spinBox->setRange(0,_numFrames);
    _slider->setValue(_currentFrame);
    if(_bg!=NULL)
        delete _bg;
    _bg = new cv::BackgroundSubtractorMOG2();
   _playButton->setVisible(true);
   _pauseButton->setVisible(false);
   _stopButton->setVisible(false);
}

QImage VideoProcessing::Mat2QImage(const cv::Mat3b &src)
{
   cv::Mat tmp1;
   cv::cvtColor(src, tmp1, CV_BGR2RGB);
   QImage tmp((uchar*) tmp1.data, tmp1.cols, tmp1.rows, tmp1.step, QImage::Format_RGB888);
   return  tmp.scaled(tmp1.cols-1,tmp1.rows-1);
}

void VideoProcessing::showLight()
{
    QPoint tmp = _foreGround->getStartPt();
    _startPt.x = tmp.x();
    _startPt.y = tmp.y();
    tmp = _foreGround->getEndpt();
    _endPt.x = tmp.x();
    _endPt.y = tmp.y();
    cv::Mat local;
    _frame.colRange(_startPt.x,_endPt.x).rowRange(_startPt.y,_endPt.y).copyTo(local);
    if(!_trainingMode)
    {
        _lightLabel->setVisible(true);
        QImage img = _foreGround->getLight();
        float intensity = averageIntensity(img);
        if(intensity > 0.4)
        {
            _lighting = true;
            _lightFrameNoVec.push_back(_currentFrame);
        }
        else
        {
            _lighting = false;
        }
        _lightLabel->setPixmap(QPixmap::fromImage(_foreGround->getLight()));
        _lightLabel->update();
    }
    else
    {
        _trainingWidget->setImg(local);
    }
}

/*void VideoProcessing::showGray()
{
    _grayLabel->setVisible(true);
    QImage localimage = _backGround->getLight();
    ColortoBinary(localimage);
    _grayLabel->setPixmap(QPixmap::fromImage(localimage));
}*/

float VideoProcessing::averageIntensity(const QImage &img)
{
    QRgb color;
    float sum = 0;
    for (int f1=0; f1<img.width(); f1++)
    {
        for (int f2=0; f2<img.height(); f2++)
        {
            color = img.pixel(f1, f2);
            int gray = qGray(color);
            sum += (float)gray/255.0;
        }
    }
    return sum/(img.width()*img.height());
}

void VideoProcessing::ColortoBinary(QImage &img)
{
    QRgb color;
    for (int f1=0; f1<img.width(); f1++)
    {
        for (int f2=0; f2<img.height(); f2++)
        {
            color = img.pixel(f1, f2);
            int gray = qGray(color);
            float threshold = (float)gray / 255.0;
            if(threshold < THRESHOLD)
            {
                img.setPixel(f1, f2, qRgb(255,255,255));
            }
            else
            {
                img.setPixel(f1, f2, qRgb(0,0,0));
            }
        }
    }
}

void VideoProcessing::playVideo(cv::Mat frame)
{
    if(_trainingWidget->getTrained()&&!_alreadyTrained)
    {
        _alreadyTrained = true;
        if(_classifier == NULL)
            _classifier = new Classification(_trainingWidget->getTrainingFeatures());
    }
    cv::Mat frame1;
    frame.copyTo(frame1);
    cv::Mat back;
    cv::Mat fore;
    _bg->operator ()(frame1,fore);
    _bg->getBackgroundImage(back);
    std::vector<cv::Rect> legalRect;
    if(!_processed)
    {
       legalRect = videoPreprocessing(frame1,back,fore);
    }
    if(_lighting && !_trainingMode )
    {
        rectangle( frame1, _startPt, _endPt, cv::Scalar(0,0,255), 2, 8, 0 );
    }



    for( uint i = 0; i < _legalBoundRectVec[_currentFrame].size(); i++ )
    {
        if( _legalBoundRectVec[_currentFrame][i].width > 0 && _legalBoundRectVec[_currentFrame][i].height > 0 )
        {
            cv::Scalar color(0,0,255);
            if( !_trainingMode )
            {
                switch (_colorIndex[_currentFrame][i])
                {
                case 1:
                    color = cv::Scalar(0,165,255);// upper antena
                    break;
                case 2:
                    color = cv::Scalar(240,32,160);// mandible
                    break;
                case 3:
                    color = cv::Scalar(0,0,255);//r tounge
                    break;
                case 4:
                    color = cv::Scalar(0,255,0);// bottom mandible
                    break;
                case 5:
                    color = cv::Scalar(255,0,0);//bottom antenna
                    break;
                }
                int index = _colorIndex[_currentFrame][i]-1;
                if( index < 0 )
                    index = 5;
                if( _renderTip && _trackListNew.size() > 0 )
                {
                    cv::Point center( _trackListNew[index].at<double>(_currentFrame,9), _trackListNew[index].at<double>(_currentFrame,10) );
                    cv::circle(frame1,center,5,color,-1);
                }
                QString label = QString::number(_colorIndex[_currentFrame][i]);
                cv::Point org =  (_legalBoundRectVec[_currentFrame][i].tl());
                org.x -= 10;
                cv::putText(frame1,label.toStdString(),org,cv::FONT_HERSHEY_PLAIN,1.5,color);
            }
            rectangle( frame1, _legalBoundRectVec[_currentFrame][i].tl(), _legalBoundRectVec[_currentFrame][i].br(), color, 2, 8, 0 );
        }
    }
  /*  if( !_firstTime )
    {
       // for( int i = 0; i < _numFrames; i++ )
        {
            cv::Point2f org = upAnt[_currentFrame];
            cv::Point2f org1 = boAnt[_currentFrame];
            cv::Point2f org2 = tougne[_currentFrame];
            cv::circle(frame1,org,5,cv::Scalar(0,165,255),-1);
            cv::circle(frame1,org1,5,cv::Scalar(255,0,0),-1);
            cv::circle(frame1,org2,5,cv::Scalar(0,0,255),-1);
        }
    }*/

   // imshow("fe",frame1);
    if(_play)
        _slider->setValue(_currentFrame++);
    if( _currentFrame >= _numFrames )
        _currentFrame = _numFrames - 1;
    QImage forGroundImg = Mat2QImage(frame1);
    _foreGround->setImage(forGroundImg);
    _foreGround->setVisible(true);
   // QImage tmp = Mat2QImage(back);
    //_backGround->setImage(tmp);
   // _backGround->setVisible(true);
    _firstTime = false;
}

std::vector<cv::Rect> VideoProcessing::videoPreprocessing(cv::Mat &frame1, cv::Mat &back, cv::Mat &fore)
{
    cv::erode(fore,fore,cv::Mat());
    cv::dilate(fore,fore,cv::Mat());
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(fore,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    std::vector<cv::Rect> legalBoundRect;
    vector<vector<cv::Point> > contours_poly( contours.size() );
    vector<cv::Rect> boundRect( contours.size() );
    std::vector<cv::Point> headVec = _preprocess->findBeeHead(back);
    if(!_firstTime)
        _preprocess->brightnessDistortion(frame1);
     int xmax =0;
     int xmin = 10000;
     int ymax =0;
     int ymin = 10000;
     for(uint i =0; i < headVec.size(); i++)
     {
         if(xmax < headVec[i].x)
             xmax = headVec[i].x;
         if(xmin > headVec[i].x)
             xmin = headVec[i].x;
         if(ymax < headVec[i].y)
             ymax = headVec[i].y;
         if(ymin > headVec[i].y)
             ymin = headVec[i].y;
     }
    QImage backGroundImg = Mat2QImage(back);
    for( uint i = 0; i < contours.size(); i++ )
    {
        approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 5, true );
        boundRect[i] = boundingRect( cv::Mat(contours_poly[i]) );
        if((boundRect[i].x+boundRect[i].width) > FACTOR*backGroundImg.width() && boundRect[i].height > 0
                && boundRect[i].width > 0 && boundRect[i].width < frame1.rows && boundRect[i].height < frame1.cols
                && boundRect[i].tl().x != boundRect[i].br().x && boundRect[i].tl().y != boundRect[i].br().y)
        {
            if( boundRect[i].width + boundRect[i].height <= 250 // detect sugar stick
                &&_preprocess->shadowRemoval(boundRect[i].x,boundRect[i].y,boundRect[i].width,boundRect[i].height) //shadow removal
                && boundRect[i].width > MINIMUM_RWIDTH && boundRect[i].height > MINIMUM_RHEIGHT
                &&_preprocess->legalRect(boundRect[i].x,boundRect[i].y,boundRect[i].width,boundRect[i].height,headVec)
               )
            {
               legalBoundRect.push_back(boundRect[i]);
            }
        }
    }
    contours.push_back(headVec);
    cv::Mat tp=cv::Mat::zeros(frame1.size(),frame1.type());
    cv::drawContours(tp,contours,contours.size()-1,cv::Scalar(0,0,255));
    if(_firstTime)
    {
       _mandible = _preprocess->getMandible(headVec,xmax,ymax,tp);
       _centroid = _preprocess->getCentroid();
    }
    cv::Mat temp;
    cv::cvtColor(frame1, temp, CV_BGR2GRAY);
    if( legalBoundRect.size() > 0 && !_trainingMode)
    {
        FeatureExtraction f(_mandible,_centroid,legalBoundRect,temp,_preFrame,_firstTime,headVec);
       //_labels.push_back(_classifier->getClassifiedLabels(f.getFeatureVec()));
        _features.push_back(f.getFeatureVec());

    }
    else if( legalBoundRect.size() == 0 && !_trainingMode )
    {
       // QVector<int>  tmp;
        //_labels.push_back(tmp);
        QVector<FeatureExtraction::feature> t;
        _features.push_back(t);
    }
    if(!_trainingMode)
        _legalBoundRectVec.push_back(legalBoundRect);
     _preFrame = temp;
     return legalBoundRect;
}

void VideoProcessing::setFramePosion(int frame)
{
    if(!_play)
    {
        if(!_rectifyMode)
        {
           // if( frame == 0 )
                _currentFrame = frame;

          //  else
              //  _currentFrame = frame -1;
        }
        else
        {
            _currentFrame = _KFList[frame-1].frameNo;
            _rCF = frame-1;
        }
        _cap.set(CV_CAP_PROP_POS_FRAMES,_currentFrame);
        if( frame > 100 )
            _currentFrame += 1;
        cv::Mat frames;
        _cap >> _frame;
        _frame.copyTo(frames);
        playVideo(frames);
        qDebug()<<_currentFrame<<_cap.get(CV_CAP_PROP_POS_FRAMES)<<frame;
    }
}

void VideoProcessing::setFileName(const QString &fileName)
{
    _fileName = fileName;
    _preprocess->computeMeanAndStd(_fileName,20);
    _cap.release();
    _cap.open(_fileName.toAscii().data());
    _numFrames = _cap.get(CV_CAP_PROP_FRAME_COUNT);
    _currentFrame = 0;
   // _cap >> _frame;
   // playVideo(_frame);
    clear();
    initializeData();
    trainingFinish();
}

void VideoProcessing::changeSelectMode()
{
    _selectBBMode = _selectBoundingBox->isChecked();
}

void VideoProcessing::changeMode(bool mode)
{
    _trainingMode = mode;
    _training->setChecked(_trainingMode);
    _trainingWidget->setVisible(_trainingMode);
    _foreGround->clearPt();
}

const cv::Point& VideoProcessing::getMandible()
{
    return _mandible;
}

const cv::Mat& VideoProcessing::getFrame()
{
    return _frame;
}

const cv::Mat& VideoProcessing::getPreFrame()
{
    return _preFrame;
}

bool VideoProcessing::isFirstTime()
{
    return _firstTime;
}

cv::Point VideoProcessing::getCentroid()
{
    return _centroid;
}

DisplayWidget* VideoProcessing::getForeground()
{
    return _foreGround;
}

void VideoProcessing::startup()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("LocoTracker");
    msgBox.setText("Please load existing training data or train samples");
   // QPushButton* loadButton = new QPushButton("Load Training Data",&msgBox);
    //QPushButton* trainButton = new QPushButton("Train Data",&msgBox);
   // msgBox.setStandardButtons(QMessageBox::Close);

   QPushButton* trainButton = msgBox.addButton("Train Data",QMessageBox::AcceptRole);
   QPushButton* loadButton  =  msgBox.addButton("Load Training Data",QMessageBox::AcceptRole);
   msgBox.addButton(QMessageBox::Close);

   // msgBox.addButton(trainButton,QMessageBox::AcceptRole);
  // QPushButton* cancel = msgBox.addButton("Cancel",QMessageBox::AcceptRole);
   //
    connect(trainButton,SIGNAL(clicked()),this,SLOT(trainData()));
    connect(loadButton,SIGNAL(clicked()),this,SLOT(loadData()));
    int ret = msgBox.exec();
    if(ret == QMessageBox::Close)
    {
        exit(EXIT_FAILURE);
    }
}

void VideoProcessing::exportData2CSV()
{
    for( int n = 0; n < _lightFrameNoVec.size(); n++ )
    {
        if( _legalBoundRectVec[_lightFrameNoVec[n]].size() > 0 )
        {
            for( int i = 0; i< 6; i++ )
                _trackListNew[i].at<double>(_lightFrameNoVec[n],14) = 1;
        }
    }
    IOHandle io;
    io.exportTrackList2CSV(_fileName,_trackListNew);
}

void VideoProcessing::initializeData()
{
    QString dirName="";
    int ind = _fileName.indexOf(".");
    dirName = _fileName;
    dirName.remove(ind,_fileName.length()-1);
    QDir d(dirName);
    IOHandle io;
    if( d.exists() )
    {
        QFileInfoList list = d.entryInfoList();
        QFileInfo fileInfo = list.at(3);
        QString f = fileInfo.filePath();
        io.readLegalRectFromFile(_legalBoundRectVec,f);
        fileInfo = list.at(2);
        f = dirName+"/"+fileInfo.fileName();
        io.readFeatureFromFile(_features,f);
        fileInfo = list.at(4);
        f = dirName+"/"+fileInfo.fileName();
        io.readManCenFromFile(_mandible,_centroid,f);
        fileInfo = list.at(5);
        f = dirName+"/"+fileInfo.fileName();
        io.readProbFromFile(_probList,f);
        fileInfo = list.at(6);
        f = dirName+"/"+fileInfo.fileName();
        io.readSeqFromFile(_colorIndex,f);
        _labels = _colorIndex;
    }
    else
    {
        QProgressDialog p("Preprocessing...","cancel",0,_numFrames,this);
        int i = 0;
        while(_cap.read(_frame))
        {
            cv::Mat frame1;
            _frame.copyTo(frame1);
            cv::Mat back;
            cv::Mat fore;
            _bg->operator ()(frame1,fore);
            _bg->getBackgroundImage(back);
            videoPreprocessing(frame1,back,fore);
            p.setValue(i);
            qApp->processEvents();
            if( p.wasCanceled() )
            {
                exit(EXIT_FAILURE);
            }
            i++;
        }
    }
}

void VideoProcessing::drawTip()
{
    _cap.set(CV_CAP_PROP_POS_FRAMES,0);
    DisplayResult dis(_colorIndex,_legalBoundRectVec,_centroid,_cap,_mandible);
    _trackListNew = dis.getTrackList();
    _renderTip = true;
    _cap.set(CV_CAP_PROP_POS_FRAMES,0);
    _cap >> _frame;
    playVideo(_frame);
}

void VideoProcessing::probComputation()
{
    if( _alreadyTrained )
    {
        int ind = _fileName.indexOf(".");

        QString dirName = _fileName;
        dirName.remove(ind,_fileName.length()-1);
        QDir d(dirName);
        if(!d.exists())
        {
        _labels.clear();
        for( int i = 0; i < _features.size(); i++ )
        {
            if(_features[i].size() > 0 )
                _labels.push_back(_classifier->getClassifiedLabels(_features[i]));
            else
            {
                 QVector<int>  tmp;
                 _labels.push_back(tmp);
            }
        }
        if( _probList.size() != _numFrames )
        {
            InitialProb init(_mandible,_centroid);
            _cap.set(CV_CAP_PROP_POS_FRAMES,0);
            QProgressDialog p("Processing...","cancel",0,_numFrames,this);
            for( int i = 0; i < _legalBoundRectVec.size(); i++ )
            {
                cv::Mat frame;
                _cap >> frame;
               // if( i == 954 )
                  //  cv::imshow("sdf",frame);
                p.setValue(i);
                init.setFrameNum(_legalBoundRectVec[i],_features[i],_labels[i],frame);
                qApp->processEvents();
                if( p.wasCanceled() )
                {
                    exit(EXIT_FAILURE);
                }
            }
            _probList.clear();
            _colorIndex.clear();
            _probList = init.getProb();
            _colorIndex = init.getSeq();
            }
        }
        qDebug()<<"before update";
        UpdateNew* u = new UpdateNew();
        qDebug()<<_probList.size()<<_features.size()<<_colorIndex.size()<<_labels.size();
        u->update(_probList,_features,_colorIndex);
        delete u;
        qDebug()<<"after update";

        if( !d.exists() )
        {
            QDir d1;
            IOHandle io;
            d1.mkdir(dirName);
            io.writeLegalRect2File(_legalBoundRectVec,dirName);
            io.writeFeature2File(_features,dirName);
            io.writeManAndcen2File(_mandible,_centroid,dirName);
            io.writeProb2file(_probList,dirName);
            io.writeSeq2File(_colorIndex,dirName);
        }

        _cap.release();
        _cap.open(_fileName.toAscii().data());
        _numFrames = _cap.get(CV_CAP_PROP_FRAME_COUNT);
        _currentFrame = 0;
        _slider->setRange(0,_numFrames);
        _spinBox->setRange(0,_numFrames);
        _slider->setValue(_currentFrame);
        _playButton->setVisible(true);
        _pauseButton->setVisible(false);
        _stopButton->setVisible(false);
        _processed = true;
    }
}

void VideoProcessing::trainData()
{
    _training->setChecked(true);
}

void VideoProcessing::loadData()
{
    _trainingWidget->loadAll();
    _training->setVisible(false);
}

void VideoProcessing::trainingFinish()
{
    _training->setChecked(false);
    _alreadyTrained = true;

    if( _classifier != NULL )
        delete _classifier;
    _classifier = new Classification(_trainingWidget->getTrainingFeatures());
   // processing();
    probComputation();
}

void VideoProcessing::rectifyMode(bool i)
{
    _selectLightMode->setVisible(!i);
    _selectBoundingBox->setVisible(!i);
    _training->setVisible(!i);
    _rightFrame->setVisible(i);
    _KFLabel->setVisible(i);
    _KFNumLabel->setVisible(i);
    _rectifyMode = i;
    _playButton->setVisible(!i);
    _rectifyBox->setChecked(i);
    stop();
    if( _rectifyMode )
    {
        if( _KFList.size() != 0 )
            _KFList.clear();
        QVector<int> KFList;
        for( int i = 0; i < _probList.size()-1; i++ )
        {
            if( _probList[i] < 1 )
                KFList.push_back(i);
        }
        if( KFList.size() == 0 )
        {
            QMessageBox msg;
            msg.setWindowTitle("LocoTracker");
            msg.setText("No more key frames");
            msg.setDefaultButton(QMessageBox::Cancel);
            msg.exec();
            rectifyMode(false);
            return;
        }
        RectificationSt tt;
        tt.frameNo = KFList[0];
        tt.frameModified = false;
        for( int j = 0; j < (int)_legalBoundRectVec[tt.frameNo].size(); j++ )
            tt.bbModified.push_back(false);
        _KFList.push_back(tt);
        for( int i = 1; i < KFList.size(); i++ )
        {
            if( KFList[i-1] != KFList[i]-1 )
            {
                RectificationSt tmp;
                tmp.frameNo = KFList[i];
                tmp.frameModified = false;
                for( int j = 0; j < (int)_legalBoundRectVec[tmp.frameNo].size(); j++ )
                    tmp.bbModified.push_back(false);
                _KFList.push_back(tmp);
            }
        }
        _numOfKeyFrames = _KFList.size();
      //  setFramePosion(_KFList[0].frameNo);
        _KFNumLabel->setText(QString::number(_numOfKeyFrames));
        _slider->setRange(1,_numOfKeyFrames);
        _slider->setValue(1);
        _spinBox->setRange(1,_numOfKeyFrames);
        _spinBox->setValue(1);
    }
    else
    {
        _slider->setRange(1,_numFrames);
        _spinBox->setRange(1,_numFrames);
    }
}

void VideoProcessing::readyToModify()
{
    QPoint p = _foreGround->getStartPt();
    cv::Point p1;
    p1.x = p.x();
    p1.y = p.y();
    float mindist = 1000;
    _localIndex = -1;
    for( int i = 0; i < (int)_legalBoundRectVec[_currentFrame].size(); i++ )
    {
        cv::Point center;
        center.x = (_legalBoundRectVec[_currentFrame][i].tl().x + _legalBoundRectVec[_currentFrame][i].br().x)/2;
        center.y = (_legalBoundRectVec[_currentFrame][i].tl().y + _legalBoundRectVec[_currentFrame][i].br().y)/2;
        float dist = computeDist(center,p1);
        if( dist < mindist )
        {
            mindist = dist;
            _localIndex = i;
        }
    }
    if( _localIndex == -1 )
        return;
    cv::Rect rr = _legalBoundRectVec[_currentFrame][_localIndex];
    cv::Mat local;// = _frame(rr).clone;
    _frame(rr).copyTo(local);
    QImage tmp = Mat2QImage(local);

    if( _rectifyMode )
    {
        RectifyWidget* r = new RectifyWidget(tmp,_labels[_currentFrame][_localIndex],_colorIndex[_currentFrame][_localIndex],_probList[_currentFrame]);
        connect(r,SIGNAL(seqModified(int)),this,SLOT(modification(int)));
        connect(r->getDoneButton(),SIGNAL(clicked()),this,SLOT(removeBBox()));
        r->show();
    }
    else if( _trainingMode )
    {
        _trainingWidget->setImg(local);
        _trainingWidget->setFeature(_features[_currentFrame][_localIndex]);
    }
}

void VideoProcessing::clear()
{
    _legalBoundRectVec.clear();
    _labels.clear();
    _probList.clear();
    _features.clear();
    _colorIndex.clear();
    _currentFrame = 0;
    _lightFrameNoVec.clear();
    _renderTip = false;
    _trackListNew.clear();
}

void VideoProcessing::addNewBBox()
{
    QPoint st = _foreGround->getStartPt();
    QPoint et = _foreGround->getEndpt();
    int width = et.x() - st.x();
    int height = et.y() - st.y();
    _foreGround->clearPt();
    cv::Rect tmp(st.x(),st.y(),width,height);
    _legalBoundRectVec[_currentFrame].push_back(tmp);
    _colorIndex[_currentFrame].push_back(1);
    _labels[_currentFrame].push_back(1);
    _probList[_currentFrame] = 0;
    _KFList[_rCF].bbModified.push_back(false);
    _localIndex = (int)_legalBoundRectVec[_currentFrame].size()-1;
    FeatureExtraction::feature tmpf;
    cv::Point far;
    cv::Point near;
    FeatureExtraction f;
    f.computeNearestPt(_mandible,tmp,far,near);
    tmpf.distNO = qRound(std::sqrt(std::pow(near.x-_mandible.x,2) + std::pow(near.y-_mandible.y,2) ));
    cv::Point pb = f.changeCoordinate(_centroid,_mandible,far);
    tmpf.xFarest = pb.x;
    for( int i = 0; i < tmpf.edge_counter.size();i++ )
        tmpf.edge_counter[i] = 0;
    _features[_currentFrame].push_back(tmpf);
    cv::Mat local;// = _frame(rr).clone;
    _frame(tmp).copyTo(local);
    QImage tmp1 = Mat2QImage(local);
    RectifyWidget* r = new RectifyWidget(tmp1,_labels[_currentFrame][_localIndex],_colorIndex[_currentFrame][_localIndex],_probList[_currentFrame]);
    connect(r,SIGNAL(seqModified(int)),this,SLOT(modification(int)));
    connect(r->getDoneButton(),SIGNAL(clicked()),this,SLOT(removeBBox()));
    r->show();
}

void VideoProcessing::modification(int seq)
{
    bool modified = false;
    if( _KFList[_rCF].frameModified )
        modified = true;
    else
    {
        for( int i = 0; i < _KFList[_rCF].bbModified.size(); i++ )
        {
            if( !_KFList[_rCF].bbModified[i] )
            {
                modified = false;
                break;
            }
        }
    }
    if(modified)
    {
        QMessageBox msgBox;
        msgBox.setText("Already modified. Do you want to modify again?");
        QString s("Yes");
        QString s1("No");
        QPushButton* yesButton = msgBox.addButton(s,QMessageBox::AcceptRole);
        QPushButton* noButton = msgBox.addButton(s1,QMessageBox::RejectRole);
        msgBox.exec();
        if( msgBox.clickedButton() == yesButton )
        {

        }
        else if( msgBox.clickedButton() == noButton )
        {
            return;
        }
    }
    _KFList[_rCF].bbModified[_localIndex] = true;
    _colorIndex[_currentFrame][_localIndex] = seq;
    playVideo(_frame);
}

void VideoProcessing::currentFrameReady()
{
    bool finished = true;
    for( int i = 0; i < _KFList.size(); i++ )
    {
        if( !_KFList[i].frameModified )
        {
            finished = false;
            break;
        }
    }
    if( !finished )
    {
        _KFList[_rCF].frameModified = true;
        //if( _numOfKeyFrames > 1 )
        {
            _numOfKeyFrames--;
            _KFNumLabel->setText(QString::number(_numOfKeyFrames));
            _probList[_currentFrame] = 1;
            if( _rCF + 1 < _KFList.size() )
            _spinBox->setValue(_rCF+2);
        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("All frames modified. Do you want to update result?");
        QString s("Yes");
        QString s1("No");
        QPushButton* yesButton = msgBox.addButton(s,QMessageBox::AcceptRole);
        QPushButton* noButton = msgBox.addButton(s1,QMessageBox::RejectRole);
        msgBox.exec();
        connect(yesButton,SIGNAL(clicked()),this,SLOT(reay2Update()));
        if( msgBox.clickedButton() == yesButton )
            reay2Update();
        else if( msgBox.clickedButton() == noButton )
            return;
    }
}

void VideoProcessing::reay2Update()
{
   UpdateNew* u = new UpdateNew();
    u->update(_probList,_features,_colorIndex);
    delete u;
    _rectifyBox->setChecked(false);
    _rectifyMode = false;
    setFramePosion(0);
    _numFrames = _cap.get(CV_CAP_PROP_FRAME_COUNT);

    _slider->setRange(0,_numFrames);
    _spinBox->setRange(0,_numFrames);
    _slider->setValue(_currentFrame);
    _playButton->setVisible(true);
    _pauseButton->setVisible(false);
    _stopButton->setVisible(false);
    _processed = true;
}

void VideoProcessing::removeBBox()
{
    _legalBoundRectVec[_currentFrame].erase(_legalBoundRectVec[_currentFrame].begin()+_localIndex);
    _features[_currentFrame].erase(_features[_currentFrame].begin()+_localIndex);
    _colorIndex[_currentFrame].erase(_colorIndex[_currentFrame].begin()+_localIndex);
    playVideo(_frame);
}

float inline VideoProcessing::computeDist(cv::Point p1, cv::Point p2)
{
    return std::sqrt( (p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) );
}
