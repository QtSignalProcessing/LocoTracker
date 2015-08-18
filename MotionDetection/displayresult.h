#ifndef DISPLAYRESULT_H
#define DISPLAYRESULT_H

#include "initialprob.h"

class DisplayResult
{
public:
    DisplayResult(const QVector< QVector<int> >& colorIndex,QVector<std::vector<cv::Rect> >&legalBoundRectVec,
                  const cv::Point& centroid,cv::VideoCapture& cap,const cv::Point& mandible);

    QVector<cv::Mat>& getTrackList();
    bool getFinished();

private:
    void setTrackList(cv::Mat& trackList, int row,cv::Rect& localRect);
    void computeTipFromBB(QVector<cv::Mat>& trackList,QVector<std::vector<cv::Rect> >&legalBoundRectVec,
                          cv::VideoCapture& cap,const cv::Point& centroid);
    void fillGapOfTracklets(QVector<cv::Mat>& trackList,QVector<cv::Mat>& trackListNew,
                            cv::VideoCapture& cap,const cv::Point& mandible,
                            QVector<std::vector<cv::Rect> >&legalBoundRectVec);
    cv::Mat findCorner(cv::Mat& im,int x, int y);
    void setRect(cv::Mat& trackList, int row,cv::Rect& localRect);
    cv::Point getTip(const cv::Rect &rect,const cv::Mat &fgmask,bool tongue,const cv::Point& centroid);
    QVector<cv::Mat> _trackListNew;
    bool _finished;
};

#endif // DISPLAY_H
