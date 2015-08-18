#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include "featureextraction.h"

class Classification
{
public:
    Classification(const QVector< QVector<FeatureExtraction::feature> >& trainingData);
    void setTrainingData(const QVector< QVector<FeatureExtraction::feature> >& trainingData);
    QVector<int> getClassifiedLabels(const QVector< FeatureExtraction::feature >& samples);

private:
    cv::Mat _trainingDataMat;
    cv::Mat _labelsMat;
    CvSVM _SVM;
    cv::Mat _shift;
    cv::Mat _scalingF;

    void feature2Mat(const FeatureExtraction::feature& tmp, int row, cv::Mat& target);
};

#endif // CLASSIFICATION_H
/*
 *class number = 3; cn
 *
 *examples number = 10; en
 *gn = cn*en; ????
 *
 *
 *
 */
