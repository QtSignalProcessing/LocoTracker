#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <QString>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/opencv.hpp>

class Preprocess
{
public:
    Preprocess();
    void computeMeanAndStd(const QString& filename,int numOfFrames);
    enum Relation
    {
        EQUAL = 0,
        GREATER,
        LESS
    };
    cv::Mat getMean();
    cv::Mat getStd();
    std::vector<cv::Point> findBeeHead(cv::Mat frame);
    cv::Point getMandible(const std::vector<cv::Point>&headVec,int xmax,int ymax,cv::Mat& tp);
    cv::Point getMandible();
    cv::Point getCentroid();
    void brightnessDistortion(cv::Mat& frame);
    bool legalRect(int x,int y, int width,int height,std::vector<cv::Point>& beeHead);
    bool shadowRemoval(int x,int y, int width,int height);
    bool splitBBox(cv::Rect& bbox,cv::Mat fore,cv::Point mandible,std::vector<cv::Rect>& newRect);
    cv::Mat imFillHoles(cv::Mat input);
    void bwareaopen(cv::Mat& input, double size);
   // void sortRows(std::vector<cv::Rect>& boundRect);
    QVector<int> sortRows(std::vector<cv::Rect>& legalBoundRectVec);
    void sortRows(QVector<cv::Point2f>& centers2);
    void find(const cv::Mat& data,float number,Relation realtion,QVector<int>& rows, QVector<int>& cols);

private:
    cv::Mat _mean;
    cv::Mat _std;
    cv::Point _mandible;
    cv::Mat _shadowMask;
    cv::Point _centroid;
};

#endif // PREPROCESS_H
