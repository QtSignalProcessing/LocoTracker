#define _USE_MATH_DEFINES
#include <cmath>

#include "displayresult.h"

#include "preprocess.h"
#include "updatenew.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <opencv2/opencv.hpp>

#include <QDebug>

DisplayResult::DisplayResult(const QVector< QVector<int> >& colorIndex,QVector<std::vector<cv::Rect> >&legalBoundRectVec,
                             const cv::Point& centroid,cv::VideoCapture& cap,const cv::Point& mandible):_finished(false)
{
    int frameno = legalBoundRectVec.size();
    int paranum = 15;
    QVector<cv::Mat> trackList;
    for( int i = 0; i < 6; i++ )
    {
        cv::Mat tmp = cv::Mat::zeros(frameno,paranum,CV_64F);
        trackList.push_back(tmp);
    }
    _trackListNew = trackList;
    for( int n = 0; n < frameno - 1; n++ )
    {
        QVector<int> seq = colorIndex[n];
        if( seq.size() > 0 )
        {
            std::vector<cv::Rect> localRect = legalBoundRectVec[n];
            for( int i = 0; i < seq.size(); i++ )
            {
                if( seq[i] == 2 || seq[i] == 4 )
                {
                    if( localRect[i].x > centroid.x )
                    {
                        setTrackList(trackList[5],n,localRect[i]);
                    }
                    else
                    {
                        setTrackList(trackList[seq[i]-1],n,localRect[i]);
                    }
                }
                if( seq[i] != 0 )
                {
                    setTrackList(trackList[seq[i]-1],n,localRect[i]);
                }
                else
                {
                    setTrackList(trackList[5],n,localRect[i]);
                }
            }
        }
    }
    computeTipFromBB(trackList,legalBoundRectVec, cap,centroid);
    fillGapOfTracklets(trackList,_trackListNew,cap,mandible,legalBoundRectVec);
	for( int n = 0; n < frameno-2; n++ )
	{
		if( legalBoundRectVec[n].size()>0 )
		{
			for( int i = 0; i < 6; i++ )
			{
				if( cv::sum(trackList[i].row(n))[0] )
				{
					_trackListNew[i].at<double>(n,0) = n + 1;
					_trackListNew[i].at<double>(n,5) = centroid.x;
					_trackListNew[i].at<double>(n,6) = centroid.y;
					_trackListNew[i].at<double>(n,7) = mandible.x;
					_trackListNew[i].at<double>(n,8) = mandible.y;
					cv::Point2d tip;
					tip.x = _trackListNew[i].at<double>(n,9);
					tip.y = _trackListNew[i].at<double>(n,10);
                    double tmp = std::atan((tip.y-centroid.y)/(tip.x-centroid.x));
					double theta = 0;
					if( tip.x - centroid.x < 0 )
                        theta = std::fabs(tmp/M_PI*180);
					else 
                        theta = std::fabs(tmp/M_PI*180) + 90;
					_trackListNew[i].at<double>(n,11) = theta;
                    double x1 = std::pow(tip.y-centroid.y,2);
                    double y1 = std::pow(tip.x-centroid.x,2);
                    double d = std::sqrt(x1+y1);
					_trackListNew[i].at<double>(n,13) = d;
				}
			}
		}
	}
    _finished = true;
}

bool DisplayResult::getFinished()
{
    return _finished;
}

//const cv::Point& centroid,const cv::Mat& fgmask,const cv::Point& mandible


void DisplayResult::setRect(cv::Mat &trackList, int row, cv::Rect &localRect)
{
    localRect.x = trackList.at<double>(row,1);
    localRect.y = trackList.at<double>(row,2);
    localRect.width = trackList.at<double>(row,3);
    localRect.height = trackList.at<double>(row,4);
}

void DisplayResult::setTrackList(cv::Mat &trackList, int row, cv::Rect &localRect)
{
    trackList.at<double>(row,0) = row;
    trackList.at<double>(row,1) = localRect.x;
    trackList.at<double>(row,2) = localRect.y;
    trackList.at<double>(row,3) = localRect.width;
    trackList.at<double>(row,4) = localRect.height;
}


void DisplayResult::computeTipFromBB(QVector<cv::Mat>& trackList,QVector<std::vector<cv::Rect> >&legalBoundRectVec,
                                     cv::VideoCapture& cap,const cv::Point& centroid)
{
    InitialProb init;
    for( int n = 0; n < legalBoundRectVec.size()-1; n++ )
    {
        cap.set(CV_CAP_PROP_POS_FRAMES,n);
        cv::Mat frame;
        cap >> frame;
        cv::Mat fgmask =  init.getFgmask(frame);
        if( (int)legalBoundRectVec[n].size() > 0 )
        {
            for( int i = 0; i < 5; i++ )
            {
                cv::Scalar r = cv::sum(trackList[i].row(n));
                cv::Point tip;
                if( r[0] != 0 )
                {
                    cv::Rect tmp;
                    setRect(trackList[i],n,tmp);
                    if( i == 3 )
                        tip = getTip(tmp,fgmask,true,centroid);
                    else
                        tip = getTip(tmp,fgmask,false,centroid);
                    trackList[i].at<double>(n,9) = tip.x;
                    trackList[i].at<double>(n,10) = tip.y;
                }
            }
        }
    }
}

cv::Point DisplayResult::getTip(const cv::Rect &rect,const cv::Mat &fgmask,bool tongue,const cv::Point& centroid)
{
    cv::Mat tmp;
    fgmask.colRange(rect.x, rect.x + rect.width).rowRange(rect.y,rect.y + rect.height).copyTo(tmp);
    QVector<int> rows;
    QVector<int> cols;
    Preprocess preprocess;
    preprocess.bwareaopen(tmp,30);
    preprocess.find(tmp,1,Preprocess::EQUAL,rows,cols);
    cv::Point tip;
    if( rows.size() == cols.size() )
    {
        int max = 0;

        for( int j = 0; j < rows.size(); j ++ )
        {
            rows[j] += rect.y;
            cols[j] += rect.x;
            float dist;
            if( !tongue )
            dist = ( rows[j] - centroid.y ) * ( rows[j] - centroid.y ) + (cols[j] - centroid.x ) * (cols[j] - centroid.x );
            else
                dist = ( rows[j] - centroid.y ) * ( rows[j] - centroid.y );
            if( dist > max )
            {
                max = dist;
                tip.y = rows[j];
                tip.x = cols[j];
            }
        }
    }
    return tip;
}

void DisplayResult::fillGapOfTracklets(QVector<cv::Mat>& trackList,QVector<cv::Mat>& trackListNew,
                                       cv::VideoCapture& cap,const cv::Point& mandible,
                                       QVector<std::vector<cv::Rect> >&legalBoundRectVec)
{
    InitialProb init;
    int win = 10;
    for( int n = 0; n < legalBoundRectVec.size()-1; n++ )
    {
        cap.set(CV_CAP_PROP_POS_FRAMES,n);
        cv::Mat frame;
        cap >> frame;
        cv::Mat fgmask =  init.getFgmask(frame);
        if( (int)legalBoundRectVec[n].size() > 0 )
        {
            int flag = 0;
            if( n >= 15 && n < legalBoundRectVec.size() - 15 )
            {
                for( int j = 0; j < win; j++ )
                {
                    if( cv::sum(trackList[2].row(n-j))[0] != 0 || cv::sum(trackList[2].row(n+j))[0] != 0 )
                    {
                        flag = 1;
                        break;
                    }

                }
            }
            if( cv::sum(trackList[2].row(n))[0] == 0 && flag == 1 && n >= 15 && n < legalBoundRectVec.size() - 15 && trackList[0].at<double>(n,3) && trackList[0].at<double>(n,4) )
            {
                cv::Rect box;
                setRect(trackList[0],n,box);
                Preprocess preprocess;
                cv::Mat tmp;

                fgmask.colRange(box.x, box.x + box.width).rowRange(box.y,box.y + box.height).copyTo(tmp);

                QVector<int> rows,cols;
                preprocess.bwareaopen(tmp,30);
                preprocess.find(tmp,0,Preprocess::GREATER,rows,cols);
                int number = rows.size();
                if( number > 1500 && box.x <= mandible.x && ( box.x + box.width ) > mandible.x && box.y <= mandible.y && ( box.y + box.height ) > mandible.y )
                {
                    cv::Mat im;
                    cv::Rect rect;
                    setRect(trackList[0],n,rect);
                    fgmask.colRange(rect.x - 4, rect.x + rect.width + 4 ).rowRange(rect.y - 4 ,rect.y + rect.height + 4 ).copyTo(im);
                    cv::Mat can = findCorner(im,rect.x,rect.y);
                    cv::Point Ttip,Atip;
                    for( int j = 0; j < win; j++ )
                    {
                        if( cv::sum(trackList[2].row(n-j))[0] != 0 )
                        {
                            Ttip.x = trackList[2].at<double>(n-j,9);
                            Ttip.y = trackList[2].at<double>(n-j,10);
                            break;
                        }
                        else if( cv::sum(trackList[2].row(n+j))[0] != 0 )
                        {
                            Ttip.x = trackList[2].at<double>(n+j,9);
                            Ttip.y = trackList[2].at<double>(n+j,10);
                            break;
                        }
                    }
                    for( int j = 0; j < win; j++ )
                    {
                        cv::Rect Bbox,Fbox;
                        setRect(trackList[0],n-j,Bbox);
                        setRect(trackList[0],n+j,Fbox);
                        if( cv::sum(trackList[0].row(n-j))[0] != 0 &&
                          ( Bbox.x < mandible.x && Bbox.x + Bbox.width >= mandible.x && Bbox.y <= mandible.y &&
                            Bbox.y + Bbox.height >= mandible.y ) == false )
                        {
                            Atip.x = trackList[0].at<double>(n-j,9);
                            Atip.y = trackList[0].at<double>(n-j,10);
                            break;
                        }
                        else if( cv::sum(trackList[0].row(n+j))[0] != 0 &&
                                 ( Fbox.x <= mandible.x && Fbox.x + Fbox.width >= mandible.x &&
                                   Fbox.y <= mandible.y && Fbox.y + Fbox.height >= mandible.y) == false )
                        {
                            Atip.x = trackList[0].at<double>(n+j,9);
                            Atip.y = trackList[0].at<double>(n+j,10);
                            break;
                        }
                    }
                    cv::Mat target(2,2,CV_64F);
                    target.at<double>(0,0) = Ttip.x;
                    target.at<double>(0,1) = Ttip.y;
                    target.at<double>(1,0) = Atip.x;
                    target.at<double>(1,1) = Atip.y;
                    UpdateNew u;
                    QVector<int> ind = u.HungarianLinker(can,target);
                    for( int i = 0; i < ind.size(); i++ )
                    {
                        if( ind[i] > 0 )
                            can.row(i).copyTo(target.row(ind[i]));
                    }
                    trackListNew[2].at<double>(n,9) = target.at<double>(0,0);
                    trackListNew[2].at<double>(n,10) = target.at<double>(0,1);
                    trackListNew[0].at<double>(n,9) = target.at<double>(1,0);
                    trackListNew[0].at<double>(n,10) = target.at<double>(1,1);
                }
                //find candidate tip for tongue from track 5
                if( trackList[4].at<double>(n,3) && trackList[4].at<double>(n,4) )
                {
                    cv::Mat im;
                    setRect(trackList[4],n,box);
                    fgmask.colRange(box.x - 4, box.x + box.width + 4 ).rowRange(box.y - 4 ,box.y + box.height + 4 ).copyTo(im);
                    preprocess.find(im,0,Preprocess::GREATER,rows,cols);
                    number = rows.size();
                    if( number > 1500 && box.x <= mandible.x && ( box.x + box.width ) > mandible.x && box.y <= mandible.y && ( box.y + box.height ) > mandible.y )
                    {
                        cv::Point Ttip,Atip;
                        cv::Mat can = findCorner(im,box.x,box.y);
                        for( int j = 0; j < win; j++ )
                        {
                            if( cv::sum(trackList[2].row(n-j))[2] != 0 )
                            {
                                Ttip.x = trackList[2].at<double>(n-j,9);
                                Ttip.y = trackList[2].at<double>(n-j,10);
                                break;
                            }
                            else if( cv::sum(trackList[2].row(n+j))[2] != 0 )
                            {
                                Ttip.x = trackList[2].at<double>(n+j,9);
                                Ttip.y = trackList[2].at<double>(n+j,10);
                                break;
                            }
                        }
                        for( int j = 0; j < win; j++ )
                        {
                            cv::Rect Bbox,Fbox;
                            setRect(trackList[4],n-j,Bbox);
                            setRect(trackList[4],n+j,Fbox);
                            if( cv::sum(trackList[4].row(n-j))[0] != 0 &&
                              ( Bbox.x < mandible.x && Bbox.x + Bbox.width >= mandible.x && Bbox.y <= mandible.y &&
                                Bbox.y + Bbox.height >= mandible.y ) == false )
                            {
                                Atip.x = trackList[4].at<double>(n-j,9);
                                Atip.y = trackList[4].at<double>(n-j,10);
                                break;
                            }
                            else if( cv::sum(trackList[4].row(n-j))[0] != 0 &&
                              ( Fbox.x < mandible.x && Fbox.x + Fbox.width >= mandible.x && Fbox.y <= mandible.y &&
                                Fbox.y + Fbox.height >= mandible.y ) == false )
                            {
                                Atip.x = trackList[4].at<double>(n+j,9);
                                Atip.y = trackList[4].at<double>(n+j,10);
                                break;
                            }
                        }
                        cv::Mat target(2,2,CV_64F);
                        target.at<double>(0,0) = Ttip.x;
                        target.at<double>(0,1) = Ttip.y;
                        target.at<double>(1,0) = Atip.x;
                        target.at<double>(1,1) = Atip.y;
                        UpdateNew u;
                        QVector<int> ind = u.HungarianLinker(can,target);
                        for( int i = 0; i < ind.size(); i++ )
                        {
                            if( ind[i] > 0 )
                                can.row(i).copyTo(target.row(ind[i]));
                        }
                        trackListNew[2].at<double>(n,9) = target.at<double>(0,0);
                        trackListNew[2].at<double>(n,10) = target.at<double>(0,1);
                        trackListNew[4].at<double>(n,9) = target.at<double>(1,0);
                        trackListNew[4].at<double>(n,10) = target.at<double>(1,1);                   }
                }
            }
        }
    }
}

cv::Mat DisplayResult::findCorner(cv::Mat &im,int x,int y)
{
    cv::Mat can = cv::Mat::zeros(10,2,CV_64F);;
    Preprocess preprocess;
    preprocess.bwareaopen(im,10);
    cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(3,3));
    cv::Mat im1;
    morphologyEx( im, im1, cv::MORPH_CLOSE, element );
    cv::Mat corners = cv::Mat::zeros(im.size(),CV_32FC1);
    cv::cornerMinEigenVal(im1,corners,2);
    double max;
    cv::minMaxIdx(corners,NULL,&max);
    double th = 0.05;
    cv::Mat largeEnoughCorners = cv::Mat::zeros(im1.size(),CV_8U);
    for( int i = 0; i < largeEnoughCorners.rows; i++ )
    {
        for( int j = 0; j < largeEnoughCorners.cols; j++ )
        {
            if( corners.at<float>(i,j) > th * max )
                largeEnoughCorners.at<uchar>(i,j) = 255;
        }
    }
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(largeEnoughCorners,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    std::vector<cv::Moments> mu(contours.size() );
     for( uint i = 0; i < contours.size(); i++ )
      { mu[i] = cv::moments( contours[i], false ); }

     ///  Get the mass centers:
     QVector<cv::Point2f> mc( contours.size() );
     for( uint i = 0; i < contours.size(); i++ )
     {
         if( mu[i].m00!=0 )
            mc[i] = cv::Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
         else
             mc[i] = cv::Point2f(0,0);
     }
     int flag = 0;
     QVector<cv::Point2f> centers2;
     for( uint i = 0; i < contours.size(); i++ )
     {
         flag = 0;
         float x = mc[i].x;
         float y = mc[i].y;
         for( uint j = 0; j < contours.size(); j++ )
         {
             if( j != i )
             {
                 if( mc[j].x <= x && mc[j].x >= x-5 && mc[j].y <= y+5 && mc[i].y >= y-5 )
                     flag = 1;
             }
         }
         if( flag == 0 )
         {
             cv::Point2f p(x,y);
             centers2.push_back(p);
         }
     }
     preprocess.sortRows(centers2);


   // if( centers2.size() > 10 )
    // {
       //  can = cv::Mat::zeros(10,2,CV_32F);
    // }
     if( centers2.size() < 10 )
        // can = cv::Mat::zeros(centers2.size(),2,CV_32F);
         can.resize(centers2.size());
     int size = 0;
     if( centers2.size() < 10 )
         size = centers2.size();
     else {
         size = 10;
     }
     for( int i = 0; i < size; i++ )
     {
         can.at<double>(i,0) = centers2[i].x + x -1;
         can.at<double>(i,1) = centers2[i].y + y -1 ;
     }
    return can;
}

QVector<cv::Mat>& DisplayResult::getTrackList()
{
    return _trackListNew;
}
