#ifndef HUNGARIANLINKER_H
#define HUNGARIANLINKER_H

#include <opencv2/opencv.hpp>

#include <QVector>

class Hungarianlinker
{
public:
    Hungarianlinker(const cv::Mat& source,const cv::Mat& target);
    QVector<int> getTargetinice();

private:
    QVector<int> _opt;

};

#endif // HUNGARIANLINKER_H
