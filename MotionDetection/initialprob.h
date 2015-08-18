#ifndef INITIALPROB_H
#define INITIALPROB_H

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/opencv.hpp>
#include "featureextraction.h"

#include <QVector>

class InitialProb
{
public:
    InitialProb( cv::Point& mandible,cv::Point& centroid);
    InitialProb();
    QVector< QVector<int> > getSeq();
    QVector<float> getProb();
    QVector< QVector<int> > getColorIndex();
    QVector<cv::Mat>& getLtableList();
    QVector<cv::Mat>& getSortProbList();
    void setLtableList(int indexOfList,int rowNum,const cv::Mat& source);
    void setLtableList(int indexOfList,const cv::Mat& source);
    void setSortProbList(int indexOfList,int rowNum,const cv::Mat& source);
    void setSeq(int indexOfLsit,int rowNum,int value);
    void setSeq(int indexOfList,QVector<int> source);
    bool isUniqueSorted(const QVector<int>& seq );
    void setLtableList(int indexOfList,int rowNum,int colNum,double value);
    void setSortProbList(int indexOfList,int rowNum,int colNum,double value);
    QVector<cv::Mat>& getSortFeatureList();
    cv::Point getCentroid();
    cv::Point getMandible();
    cv::Point getTip(QVector<int>& rows,QVector<int>& cols,const cv::Rect &rect,const cv::Point& centroid);
    void setFrameNum(std::vector<cv::Rect>& legalBoundRectVec,QVector<FeatureExtraction::feature> & features,QVector<int>& labels,cv::Mat& frame);
    cv::Mat getFgmask(const cv::Mat& frame);

private:
    void findAntena(const cv::Rect& rect,const cv::Mat& fgmask);
    void findMandible(const cv::Rect& rect,const cv::Mat& fgmask);
    cv::Point changeCoord(const cv::Point& centroid, const cv::Point& tip);
    cv::Point getPointa(const cv::Rect &rect,const cv::Mat &fgmask);
    inline bool contains3(const QVector<int>& seq);
    void twoBoxesInFrame(int frameBoundRectsize,const cv::Mat& sortProb,const std::vector<cv::Rect> &legalBoundRectVec,const cv::Mat &fgmask);
    void threeBoxesInFrame(int frameBoundRectsize,cv::Mat& ltable,const std::vector<cv::Rect> &legalBoundRectVec,const cv::Mat &fgmask);
    void fourBoxesInFrame(int frameBoundRectsize,cv::Mat& ltable,const std::vector<cv::Rect> &legalBoundRectVec,const cv::Mat &fgmask);
    void fiveBoxesInFrame(int frameBoundRectsize,cv::Mat& ltable,const std::vector<cv::Rect> &legalBoundRectVec,const cv::Mat &fgmask);
    inline void sortProb021(cv::Mat& sortProb,int k);
    inline void sortProb120(cv::Mat& sortProb,int k);
    inline void setLtable(cv::Mat& ltable,int k, int index);

    QVector< QVector<int> > _seq;
    QVector<float> _prob;
    QVector< QVector<int> > _colorIndex;
    QVector<cv::Mat> _ltableList;
    QVector<cv::Mat> _sortProbList;
    QVector<cv::Mat> _sortFeatureList;
    cv::Point _centroid;
    cv::Point _mandible;
};

#endif // INITIALPROB_H
