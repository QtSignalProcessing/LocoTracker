#include "featureextraction.h"

#include "math.h"
#include <QVector>

#include <QDebug>

#include<iostream>

FeatureExtraction::FeatureExtraction(cv::Point mandible,cv::Point centroid,const std::vector<cv::Rect> &legalBoundRect,const cv::Mat& frame1,const cv::Mat& preFrame,bool first,const std::vector<cv::Point>& beeHead):_featureVec()
{
    cv::Mat frame2;
    frame1.convertTo(frame2,CV_64F);
    cv::Scalar avgPixelIntensity = cv::mean( frame2 );
    cv::Mat yad = frame2 - avgPixelIntensity[0];
   // qDebug()<<"mean "<<avgPixelIntensity[0];
    for(uint i = 0; i < legalBoundRect.size(); i++)
    {
        feature tmp;
        cv::Point far;
        cv::Point near;
        computeNearestPt(centroid,legalBoundRect[i],far,near);// ( matlab: dist( mandible, n ))
      //  if( (int)i == 0 )
           // qDebug()<<avgPixelIntensity[0]<<" x:"<<legalBoundRect[i].x<<" y:"<<legalBoundRect[i].y<<" w:"<<legalBoundRect[i].width<<" h:"<<legalBoundRect[i].height;


        tmp.distNO = qRound(std::sqrt(std::pow((float)(near.x-mandible.x),2) + std::pow((float)(near.y-mandible.y),2)));
        cv::Point pb = changeCoordinate(centroid,mandible,far);
        tmp.xFarest = pb.x; //paper: x-cor of f; matlab : distance between the furthest vertex and the bee tougnue line
        //corresponds to matlab code 324-327 extract_feature11.m
        // yad.rowRange(170-1,170+46-1).colRange(344-1,344+88-1).copyTo(tmp1);
        cv::Mat temp;
        yad.rowRange(legalBoundRect[i].y-1,legalBoundRect[i].y-1+legalBoundRect[i].height).colRange(legalBoundRect[i].x-1,legalBoundRect[i].x-1+legalBoundRect[i].width).copyTo(temp);
     //   yad.colRange(legalBoundRect[i].x-1,legalBoundRect[i].x-1 + legalBoundRect[i].width).rowRange(legalBoundRect[i].y-1,legalBoundRect[i].y-1+legalBoundRect[i].height).copyTo(temp);
        int numOfzeros = 0;
        for(int k = 0; k < temp.rows; k++)
        {
            for(int j = 0; j < temp.cols; j ++)
            {
                if(temp.at<double>(k,j) < 0 )
                {
                    numOfzeros++;
                }
            }
        }
        tmp.darkRegionArea = numOfzeros;
        if( legalBoundRect[i].x <= mandible.x && (legalBoundRect[i].x + legalBoundRect[i].width) >= mandible.x &&
            legalBoundRect[i].y <= mandible.y && (legalBoundRect[i].y + legalBoundRect[i].height) >= mandible.y)
        {
            tmp.mandibleInBb = true;
        }
        else
        {
            tmp.mandibleInBb = false;
        }

        edgeHistDisc(temp,tmp);

        if(first)
        {
            tmp.xComponentOfMV = 0;
            tmp.yComponentOfMV = 0;
        }
        else
        {
            cv::Mat templ1;
            cv::Mat result;
            frame2.colRange(legalBoundRect[i].x-1,legalBoundRect[i].x + legalBoundRect[i].width-1).rowRange(legalBoundRect[i].y-1,legalBoundRect[i].y+legalBoundRect[i].height-1).copyTo(templ1);
            cv::Mat templ;
            templ1.convertTo(templ,CV_32F);
            cv::Mat pre;
            preFrame.convertTo(pre,CV_32F);
            matchTemplate( pre, templ, result, CV_TM_SQDIFF );
            double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
            minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
            if((int)beeHead.size() > 0)
            {
                cv::Point p0( legalBoundRect[i].x , legalBoundRect[i].y );
                cv::Point p1( legalBoundRect[i].x + legalBoundRect[i].width , legalBoundRect[i].y );
                cv::Point p2( legalBoundRect[i].x , legalBoundRect[i].y + legalBoundRect[i].height  );
                cv::Point p3( legalBoundRect[i].x + legalBoundRect[i].width , legalBoundRect[i].y + legalBoundRect[i].height );
                if( (pointPolygonTest(beeHead,p0,true) < 0) &&
                    (pointPolygonTest(beeHead,p1,true) < 0) &&
                    (pointPolygonTest(beeHead,p2,true) < 0) &&
                    (pointPolygonTest(beeHead,p3,true) < 0)) // not attach to beeHead
                {
                    tmp.xComponentOfMV = 0;
                    tmp.yComponentOfMV = 0;
                }
                else
                {
                    tmp.xComponentOfMV = legalBoundRect[i].x + legalBoundRect[i].width/2  - (minLoc.x + qRound((float)templ.cols/2.0)) - 1;
                    tmp.yComponentOfMV = legalBoundRect[i].y + legalBoundRect[i].height/2  - (minLoc.y + qRound((float)templ.rows/2.0)) - 1;
                }
            }
            else
            {
                tmp.xComponentOfMV = legalBoundRect[i].x + legalBoundRect[i].width/2  - (minLoc.x + qRound((float)templ.cols/2.0)) - 1;
                tmp.yComponentOfMV = legalBoundRect[i].y + legalBoundRect[i].height/2  - (minLoc.y + qRound((float)templ.rows/2.0)) - 1;
            }
            if( fabs(tmp.xComponentOfMV) + fabs(tmp.yComponentOfMV ) > 20  )
            {
                tmp.xComponentOfMV = 0;
                tmp.yComponentOfMV = 0;
            }
        }
       //imshow("dd",temp);

      //  _featureVec.push_back(tmp);
       _featureVec.push_back(tmp);
    }
}

FeatureExtraction::~FeatureExtraction(){}

FeatureExtraction::FeatureExtraction()
{}

void FeatureExtraction::computeNearestPt(const cv::Point centroid,const cv::Rect &BoundRect,cv::Point &far,cv::Point& near)//modified this function
{
    float distArray[4];
    distArray[0] = dist(centroid.x,centroid.y,BoundRect.x,BoundRect.y);
    distArray[1] = dist(centroid.x,centroid.y,BoundRect.x + BoundRect.width,BoundRect.y);
    distArray[2] = dist(centroid.x,centroid.y,BoundRect.x,BoundRect.y + BoundRect.height);
    distArray[3] = dist(centroid.x,centroid.y,BoundRect.x + BoundRect.width,BoundRect.y + BoundRect.height );
    float min = 10000;
    float max = 0;
    int indexMax = -1;
    int indexMin = -1;
    for(int i = 0; i < 4; i++ )
    {
        if( min > distArray[i] )
        {
            min = distArray[i];
            indexMin = i;
        }
        if( max < distArray[i] )
        {
            max = distArray[i];
            indexMax = i;
        }
    }
    switch (indexMax) {
    case 0:
        far.x = BoundRect.x;
        far.y = BoundRect.y;
        break;
    case 1:
        far.x = BoundRect.x+BoundRect.width;
        far.y = BoundRect.y;
        break;
    case 2:
        far.x = BoundRect.x;
        far.y = BoundRect.y+BoundRect.height;
        break;
    default:
        far.x = BoundRect.x+BoundRect.width;
        far.y = BoundRect.y+BoundRect.height;
        break;
    }
    switch (indexMin) {
    case 0:
        near.x = BoundRect.x;
        near.y = BoundRect.y;
        break;
    case 1:
        near.x = BoundRect.x+BoundRect.width;
        near.y = BoundRect.y;
        break;
    case 2:
        near.x = BoundRect.x;
        near.y = BoundRect.y+BoundRect.height;
        break;
    default:
        near.x = BoundRect.x+BoundRect.width;
        near.y = BoundRect.y+BoundRect.height;
        break;
    }
}

inline float FeatureExtraction::dist(float x1, float y1, float x2, float y2)
{
    return sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
}

QVector<FeatureExtraction::feature> FeatureExtraction::getFeatureVec()
{
    return _featureVec;
}

void FeatureExtraction::edgeHistDisc(const cv::Mat &I,feature &tmp)
{
    cv::Mat I1;
    I.convertTo(I1,CV_8U,255.0);
    cv::Mat edge;
    cv::Canny(I1,edge,100,255);
    cv::threshold(edge, edge, 100, 255, cv::THRESH_BINARY);
    edge.convertTo(edge,CV_64F,1.0/255.0);
    int x = I.size().width / 2.0;
    int y = I.size().height / 2.0;
    cv::Mat temp;
    edge.colRange(0,x*2).rowRange(0,y*2).copyTo(temp);

    cv::Mat filt_vertical_1 = (cv::Mat_<double>(2,2)<<1,0,1,0);
    cv::Mat filt_vertical_2 = (cv::Mat_<double>(2,2)<<0,1,0,1);
    cv::Mat filt_45_degree = (cv::Mat_<double>(2,2)<<0,1,1,0);
    cv::Mat filt_horizontal_1 = (cv::Mat_<double>(2,2)<<1,1,0,0);
    cv::Mat filt_horizontal_2 = (cv::Mat_<double>(2,2)<<0,0,1,1);
    cv::Mat filt_135_degree = (cv::Mat_<double>(2,2)<<1,0,0,1);
    cv::Mat filt_not_edge = (cv::Mat_<double>(2,2)<<0,0,0,0);

    for( int i2= 0; i2 < y; i2 ++ )
    {
        int temp2 = (2*i2);
        int temp3 = (2*i2+2);
        for(int i3 = 0; i3 < x; i3 ++)
        {
            cv::Mat local;

            temp.colRange(2*i3,2*i3+2).rowRange(temp2,temp3).copyTo(local);
           // qDebug()<<local.rows<<" "<<local.cols;
            if( isequal(filt_vertical_1,local) )
            {
                tmp.edge_counter[0]++;
            }
            else if( isequal(filt_vertical_2,local) )
            {
                tmp.edge_counter[0]++;
            }
            else if(isequal(filt_45_degree, local))
            {
                tmp.edge_counter[1]++;
            }
            else if(isequal(filt_horizontal_1,local))
            {
                tmp.edge_counter[2]++;
            }
            else if(isequal(filt_horizontal_2,local))
            {
                tmp.edge_counter[2]++;
            }
            else if(isequal(filt_135_degree, local))
            {
                tmp.edge_counter[3]++;
            }
            else if(isequal(filt_not_edge,local))
            {
                tmp.edge_counter[4]++;
            }
            else
                tmp.edge_counter[4]++;
        }
    }
}

inline bool FeatureExtraction::isequal(const cv::Mat &mat1,const cv::Mat &mat2)
{
    if (mat1.empty() && mat2.empty())
    {
        return true;
    }
    // if dimensionality of two mat is not identical, these two mat is not identical
    if (mat1.cols != mat2.cols || mat1.rows != mat2.rows || mat1.dims != mat2.dims) {
        return false;
    }
    cv::Mat diff;
    cv::compare(mat1, mat2, diff, cv::CMP_NE);
    int nz = cv::countNonZero(diff);
    return nz==0;
}

void FeatureExtraction::normailizeFeatureVec(QVector< QVector<feature> >& featureVecs)
{
    feature max = findMaxFeature(featureVecs);
    for( int i = 0; i < featureVecs.size(); i++)
    {
        for(int j = 0; j < featureVecs[i].size(); j++)
        {
            if(max.darkRegionArea != 0)
                featureVecs[i][j].darkRegionArea /= max.darkRegionArea;
            if(max.distNO != 0)
                featureVecs[i][j].distNO /= max.distNO;
            if(max.xComponentOfMV != 0)
                featureVecs[i][j].xComponentOfMV /= max.xComponentOfMV;
            if(max.xFarest != 0)
                featureVecs[i][j].xFarest /= max.xFarest;
            if(max.yComponentOfMV != 0)
                featureVecs[i][j].yComponentOfMV /= max.yComponentOfMV;
            for(int k = 0; k < featureVecs[i][j].edge_counter.size(); k++)
            {
                if(max.edge_counter[k] != 0)
                    featureVecs[i][j].edge_counter[k] /= max.edge_counter[k];
            }
        }
    }
}

FeatureExtraction::feature FeatureExtraction::findMaxFeature(QVector<QVector<feature> > &featureVecs)
{
    feature max;
    for( int i = 0; i < featureVecs.size(); i++ )
    {
        for( int j= 0; j < featureVecs[i].size(); j++ )
        {
            if( fabs(featureVecs[i][j].darkRegionArea) > max.darkRegionArea )
                max.darkRegionArea = fabs(featureVecs[i][j].darkRegionArea);
            if( fabs(featureVecs[i][j].distNO) > max.distNO )
                max.distNO = fabs(featureVecs[i][j].distNO);
            if( fabs(featureVecs[i][j].xComponentOfMV) > max.xComponentOfMV )
                max.xComponentOfMV = fabs(featureVecs[i][j].xComponentOfMV) ;
            if( fabs(featureVecs[i][j].xFarest) > max.xFarest)
                max.xFarest = fabs(featureVecs[i][j].xFarest);
            if( fabs(featureVecs[i][j].yComponentOfMV) > max.yComponentOfMV )
                max.yComponentOfMV = fabs(featureVecs[i][j].yComponentOfMV);
            for(int k = 0; k < featureVecs[i][j].edge_counter.size(); k++)
            {
                if(featureVecs[i][j].edge_counter[k] > max.edge_counter[k])
                    max.edge_counter[k] = featureVecs[i][j].edge_counter[k];
            }
        }
    }
    return max;
}

cv::Point FeatureExtraction::changeCoordinate(cv::Point centroid, cv::Point mandible, cv::Point carcoordinate)
{
    cv::Point cord;
    float xo = mandible.x;
    float yo = mandible.y;
    cord.x = carcoordinate.x - xo;
    cord.y = carcoordinate.y - yo;
    float rho = std::sqrt((float)(cord.x*cord.x+cord.y*cord.y));
    float theta1 = std::atan((float)cord.y/(float)cord.x);
  //  qDebug()<<cord.x<<" "<<cord.y<<" "<<rho<<" "<<theta1;
    if( theta1 > 0 && cord.y < 0 )
        theta1 = theta1 + M_PI;
    if( theta1 < 0 && cord.y > 0 )
        theta1 = theta1 + M_PI;
    float theta2 = std::atan((float)(mandible.y-centroid.y)/(mandible.x-centroid.x));
    if( theta2 > 0 && centroid.y < mandible.y )
        theta1 = theta1 + 0.5 * M_PI;
    if( theta2 < 0 && centroid.y > mandible.y )
        theta1 = theta1 + 0.5 * M_PI;

    theta1 -= theta2;
    cord.x = rho * std::cos(theta1);
    cord.y = rho * std::sin(theta1);
    return cord;
}
