#ifndef UPDATEPROB_H
#define UPDATEPROB_H

#include <QVector>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/opencv.hpp>

#include "initialprob.h"

class UpdateProb
{
public:
    UpdateProb(QVector<std::vector<cv::Rect> >&legalBoundRectVec,
               InitialProb& initProb,QVector< QVector<FeatureExtraction::feature> >& feature);
    QVector<int> HungarianLinker(const cv::Mat& source,const cv::Mat& target);
    UpdateProb();

private:
    void firstForLoop();
    void notAntenna(InitialProb& initProb,int n, int k, int j,QVector<float>& pL,cv::Mat& counter);
    void notMandible(InitialProb& initProb,int n, int k, int j,QVector<float>& pL,cv::Mat& counter);
    void notProb(InitialProb& initProb,int n, int k, int j,QVector<float>& pL,cv::Mat& counter);
    inline void updateSortProbList(InitialProb &initProb, int n, int k, int j, int col1, int col2,int col3,double sort1,double sort2);
    inline void updateLtableList(InitialProb& initProb,int indexOfList,int row,int indOf1);
};

#endif // UPDATEPROB_H
