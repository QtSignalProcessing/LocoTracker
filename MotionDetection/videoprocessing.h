#ifndef VIDEOPROCESSING_H
#define VIDEOPROCESSING_H

#include <QWidget>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <qthread.h>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/opencv.hpp>

#include "featureextraction.h"
#include "preprocess.h"
#include "trainingwidget.h"
#include "classification.h"

class QLabel;
class DisplayWidget;
class TrainingWidget;

class VideoProcessing : public QWidget
{
    Q_OBJECT
public:
    VideoProcessing(QString fileName,QWidget *parent = 0);
    ~VideoProcessing();
    const cv::Point& getMandible();
    const cv::Mat& getFrame();
    const cv::Mat& getPreFrame();
    bool isFirstTime();
    DisplayWidget* getForeground();
    cv::Point getCentroid();
    struct RectificationSt
    {
        int frameNo;
        bool frameModified;
        QVector<bool> bbModified;
    };

signals:
    void videoFinished(bool finished);
	//void processedImage(QImage& img);

public slots:
    void play();
    void pause();
    void stop();
    void showLight();
   // void showGray();
    void setFileName(const QString &fileName);
    void setFramePosion(int frame);
    void changeSelectMode();
    void changeMode(bool);
    void trainData();
    void loadData();
    void trainingFinish();
    void rectifyMode(bool i);
    void readyToModify();
    void modification(int seq);
    void currentFrameReady();
    void removeBBox();
    void addNewBBox();
    void reay2Update();
    void storeUpdated();
    void drawTip();
    void exportData2CSV();
    void write2video();

protected:

    
private:
    QImage Mat2QImage(const cv::Mat3b &src);
    void playVideo( cv::Mat frame);
    std::vector<cv::Rect> videoPreprocessing(cv::Mat& frame,cv::Mat& back,cv::Mat& fore);
    void ColortoBinary(QImage& img);
    float averageIntensity(const QImage& img);
    void startup();
    float inline computeDist(cv::Point p1, cv::Point p2);
    void initializeData();//Load/compute BBox and features
    void closeEvent(QCloseEvent *);
    void probComputation();
    void clear();
	int computeKFNum();
    void drawing(cv::Mat& frame);

    DisplayWidget* _foreGround;
  //  DisplayWidget* _backGround;
    bool _play;
    QString _fileName;
    cv::VideoCapture _cap;
    QLabel* _lightLabel;
    QLabel* _grayLabel;
    QPushButton* _playButton;
    QPushButton* _pauseButton;
    QPushButton* _stopButton;
    bool _lighting;
    cv::Point _startPt;
    cv::Point _endPt;
    bool _firstTime;
    cv::BackgroundSubtractorMOG2* _bg;
    cv::Point _mandible;
    QVector< QVector<FeatureExtraction::feature> > _features;
    cv::Mat _preFrame;
    Preprocess* _preprocess;
    int _numFrames;
    int _currentFrame;
    QSlider* _slider;
    QSpinBox* _spinBox;
    bool _selectBBMode;
    QCheckBox* _selectLightMode;
    QCheckBox* _selectBoundingBox;
    bool _trainingMode;
    TrainingWidget* _trainingWidget;
    cv::Mat _frame;
    QCheckBox* _training;
    bool _alreadyTrained;
    Classification* _classifier;
    QVector<std::vector<cv::Rect> > _legalBoundRectVec;
    bool _processed;
    QVector< QVector<int> > _labels;
    cv::Point _centroid;
    QVector< QVector<int> > _colorIndex;
    bool _rectifyMode;
    int _numOfKeyFrames;
    QVector<float> _probList;
    QLabel* _KFLabel;
    QLabel* _KFNumLabel;
    QVector<RectificationSt> _KFList;
    int _rCF;
    int _localIndex;
    QPushButton* _rightFrame;
    QCheckBox* _rectifyBox;
    QVector<cv::Mat> _trackListNew;
	QVector<int> _lightFrameNoVec;
    QPushButton* _storeUpdated;
    QPushButton* _showFinalResult;
    bool _renderTip;
    QVector<int> _sugarFrames;
};

#endif // VIDEOPROCESSING_H
