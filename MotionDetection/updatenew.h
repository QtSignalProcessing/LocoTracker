#ifndef UPDATENEW_H
#define UPDATENEW_H

#include"featureextraction.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/opencv.hpp>

class UpdateNew
{
public:
    UpdateNew();
    void intoF(int* a, FeatureExtraction::feature& f);
    void update(QVector<float>& probList,const QVector< QVector<FeatureExtraction::feature> >& features,
                QVector< QVector<int> >& seqList);
    QVector<int> HungarianLinker(const cv::Mat& source,const cv::Mat& target);
    bool isUniqueSorted(const QVector<int>& seq );
};

#endif // UPDATENEW_H
