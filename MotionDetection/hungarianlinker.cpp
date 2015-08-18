#include "hungarianlinker.h"

#include "munkres.h"

#include <cmath>

#include <QVector>

#include <QDebug>
#include <iostream>
using namespace std;

Hungarianlinker::Hungarianlinker(const cv::Mat& source,const cv::Mat& target):_opt()
{
    int numOFSourcePt = source.rows;
    int numOfTargetPt = target.rows;
    cv::Mat D = cv::Mat::ones(numOFSourcePt,numOfTargetPt,CV_64F);
    for( int i = 0; i < numOFSourcePt; i++ )
    {
        cv::Mat currentPt(numOfTargetPt,source.cols,CV_64F);
        for( int j = 0; j < currentPt.rows; j++ )
            source.row(i).copyTo(currentPt.row(j));
        cv::Mat diff_coords = target - currentPt;

     //   diff_coords = diff_coords * diff_coords;
        for(int j = 0; j < diff_coords.rows; j++ )
        {
            for( int k = 0; k < diff_coords.cols; k++ )
            {
                diff_coords.at<double>(j,k) *= diff_coords.at<double>(j,k);
            }
        }
        cv::Mat squareDist=cv::Mat::zeros(1,numOfTargetPt,CV_64F);
        for( int j = 0; j < numOfTargetPt; j++ )
        {
            squareDist.at<double>(0,j) = cv::sum(diff_coords.row(j))[0];
        }
        squareDist.copyTo(D.row(i));
    }


    Munkres m;
   // m.diag(true);
    m.solve(D);
   _opt = m.getOptVec();
}

QVector<int> Hungarianlinker::getTargetinice()
{
    return _opt;
}
