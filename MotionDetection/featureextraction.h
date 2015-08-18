#ifndef FEATUREEXTRACTION_H
#define FEATUREEXTRACTION_H

#include <QVector>
#include <opencv2/opencv.hpp>

class FeatureExtraction
{
public:
    FeatureExtraction(cv::Point mandible,cv::Point centroid, const std::vector<cv::Rect> &legalBoundRect, const cv::Mat& frame1,const cv::Mat& preFrame,bool first, const std::vector<cv::Point>& headVec =std::vector<cv::Point>() );
    FeatureExtraction();
    ~FeatureExtraction();
    struct feature{
        float distNO;
        float xFarest;
        float darkRegionArea;
        bool mandibleInBb;
        float xComponentOfMV;
        float yComponentOfMV;
        QVector<float> edge_counter;
        int numOfFields;
        feature():distNO(0),xFarest(0),darkRegionArea(0),mandibleInBb(false),xComponentOfMV(0),yComponentOfMV(0),edge_counter(QVector<float>(5)),numOfFields(11){}
        feature operator/(const feature& a)
        {
            feature tmp;
            tmp.darkRegionArea = darkRegionArea/a.darkRegionArea;
            tmp.distNO = distNO/a.distNO;
            tmp.xComponentOfMV = xComponentOfMV/a.xComponentOfMV;
            tmp.xFarest = xFarest/a.xFarest;
            tmp.yComponentOfMV = yComponentOfMV/a.yComponentOfMV;
            for(int i = 0; i < tmp.edge_counter.size(); i++)
                tmp.edge_counter[i] = edge_counter[i]/a.edge_counter[i];
            return tmp;
        }
    };
    QVector<feature> getFeatureVec();
    void edgeHistDisc(const cv::Mat& I, feature& tmp);
    void normailizeFeatureVec(QVector< QVector<feature> >& featureVecs);
    feature findMaxFeature(QVector< QVector<feature> >& featureVecs);
    void computeNearestPt(const cv::Point mandible,const cv::Rect &legalBoundRect,cv::Point &far,cv::Point& near);
    cv::Point changeCoordinate(cv::Point centroid,cv::Point mandible, cv::Point carcoordinate);

private:
    inline float dist(float x1,float y1,float x2,float y2);
    QVector<feature> _featureVec;
    inline bool isequal(const cv::Mat& mat1,const cv::Mat& mat2);
};

#endif // FEATUREEXTRACTION_H
