#ifndef TRAININGWIDGET_H
#define TRAININGWIDGET_H

#include "videoprocessing.h"
#include "featureextraction.h"

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QRadioButton>
#include <QVector>
#include <QSpinBox>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/opencv.hpp>

class VideoProcessing;
class TrainingWidget : public QWidget
{
    Q_OBJECT
public:
    TrainingWidget(VideoProcessing* video,QWidget *parent = 0);
    void setImg(cv::Mat img);
    bool getTrained();
    void setFeature(FeatureExtraction::feature f);

    enum ObjectType
    {
        ANTENNA = 0,
        MANDIBLE,
        PROBOSCIS
    };
    const QVector< QVector<FeatureExtraction::feature> >& getTrainingFeatures();

signals:
    void closed(bool);
    void traingFinish();
    
public slots:
    void training();
    void clear();
    void finish();
    void store();
    void clearCurrent();
    void clearAll();
    void loadCurrent();
    void loadAll();
    void updatLabels();
    void frameNumChanged(int num);

private:
    VideoProcessing* _video;
    QLabel* _imgLabel;
    cv::Mat _object;
    QList<QRadioButton*> _radioButtonList;
    QLabel* _distNM;
    QLabel* _distFL;
    QLabel* _area;
    QLabel* _xComponentOfMV;
    QLabel* _yComponentOfMV;
    QLabel* _cm;
    QLabel* _edgeCounter1;
    QLabel* _edgeCounter2;
    QLabel* _edgeCounter3;
    QLabel* _edgeCounter4;
    QLabel* _edgeCounter5;
    QSpinBox* _frameNum;
    QVector< QVector<FeatureExtraction::feature> > _trainingFeatures;//0: RIGHT_ANTENNA 1: RIGHT_MANDIBLE 2: PROBOSCIS 3: LEFT_ANTENNA 4: LEFT_MANDIBLE
    bool _trained;
    FeatureExtraction::feature _tmpFeature;

    void init();
    void setLabels(const FeatureExtraction::feature &feature,int index, int framNum = 0);
    void closeEvent(QCloseEvent *event);
    inline int getActiveIndex();
    bool errorMessage();
    void messageBox(QString message);
    void writToFile(QString fileName,const QVector<FeatureExtraction::feature>& vec);
    bool readFromFile(QString fileName,int index);
};

#endif // TRAININGWIDGET_H
