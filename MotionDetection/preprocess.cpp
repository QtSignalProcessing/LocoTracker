#include "preprocess.h"

#include <QVector>
#include <QString>

#include <QDebug>
const int BACKWARDPIX = 10;
const float SHADOWMASKTHRESHOLD = 0.6;

Preprocess::Preprocess():_mean(),_std(),_mandible(),_shadowMask(),_centroid(){}

void Preprocess::computeMeanAndStd(const QString &filename, int numOfFrames)
{
    cv::VideoCapture cap(filename.toAscii().data());
    QVector<cv::Mat> frames(numOfFrames);
    for(int i = 0; i < numOfFrames; i++)
    {
        cap >> frames[i];
    }
    frames[0].convertTo(_mean,CV_32FC3,1.0/255.0);
    frames[0].convertTo(_std,CV_32FC3,1.0/255.0);
    int cn = frames[0].channels();
    for(int k = 0; k < _mean.rows; k++)
    {
        for(int j = 0; j < _mean.cols; j ++)
        {
            float r = 0;
            float g = 0;
            float b = 0;
            for( int i = 0; i < frames.size(); i++ )
            {
                uint8_t* pixelPtr = (uint8_t*)frames[i].data;
                b += pixelPtr[k*_mean.cols*cn + j*cn + 0]; // B
                g += pixelPtr[k*_mean.cols*cn + j*cn + 1]; // G
                r += pixelPtr[k*_mean.cols*cn + j*cn + 2]; // R
            }
            _mean.at<cv::Vec3f>(k,j)[0] = b/numOfFrames/255.0;
            _mean.at<cv::Vec3f>(k,j)[1] = g/numOfFrames/255.0;
            _mean.at<cv::Vec3f>(k,j)[2] = r/numOfFrames/255.0;
            float rstd = 0;
            float gstd = 0;
            float bstd = 0;
            for( int i = 0; i < frames.size(); i++ )
            {
                uint8_t* pixelPtr = (uint8_t*)frames[i].data;
                bstd += pow(((float)pixelPtr[k*_mean.cols*cn + j*cn + 0]/255.0 - _mean.at<cv::Vec3f>(k,j)[0]),2) ; // B
                gstd += pow((float)(pixelPtr[k*_mean.cols*cn + j*cn + 1]/255.0 - _mean.at<cv::Vec3f>(k,j)[1]),2); // G
                rstd += pow((float)(pixelPtr[k*_mean.cols*cn + j*cn + 2]/255.0 - _mean.at<cv::Vec3f>(k,j)[2]),2) ; // R
            }
            bstd /= (double)numOfFrames;
            bstd = sqrt(bstd);

            if( !bstd == 0 )
                _std.at<cv::Vec3f>(k,j)[0] = bstd;
            else
                _std.at<cv::Vec3f>(k,j)[0] = 1;
            gstd /= numOfFrames;
            gstd = sqrt(gstd);
            if( !gstd == 0 )
                 _std.at<cv::Vec3f>(k,j)[1] = gstd;
            else
                 _std.at<cv::Vec3f>(k,j)[1] = 1;
            rstd /= numOfFrames;
            rstd = sqrt(rstd);
            if( !rstd == 0)
            {
                 _std.at<cv::Vec3f>(k,j)[2] = rstd;
            }
            else
                 _std.at<cv::Vec3f>(k,j)[2] = 1;
        }
    }
    _shadowMask = cv::Mat_<float>(frames[0].size());
    //imshow("mean",_mean);
    cap.release();
}

cv::Mat Preprocess::getMean()
{
    return _mean;
}

cv::Mat Preprocess::getStd()
{
    return _std;
}

std::vector<cv::Point> Preprocess::findBeeHead(cv::Mat img)
{
    cv::Mat binary;
    cv::Mat im_gray;
    cvtColor(img,im_gray,CV_RGB2GRAY);
    cv::Mat img1=im_gray.colRange(im_gray.cols*0.5,im_gray.cols);
    cv::threshold(img1, binary, 128, 255, cv::THRESH_BINARY_INV);
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(binary,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    std::vector<cv::Mat> conVec;
    for(uint i = 0; i < contours.size(); i++)
    {
         conVec.push_back(cv::Mat::zeros(img1.size(),CV_8UC3));
    }
    for(uint i = 0; i< contours.size();i++)
    {
        cv::drawContours(conVec[i],contours,i,cv::Scalar(255,255,255));
    }
    uint max =0;
    int index;
    for( uint i =0; i < contours.size(); i++ )
    {
        if( contours[i].size() > max)
        {
            max = contours[i].size();
            index = i;
        }
    }
    for(uint i = 0 ;i < contours[index].size(); i++)
    {
        contours[index][i].x += im_gray.cols*0.5;
    }
    return contours[index];
}

cv::Point Preprocess::getMandible(const std::vector<cv::Point> &headVec,int xmax,int ymax,cv::Mat& tp)
{
    cv::Moments mu=cv::moments(headVec,false);
    cv::Point2f centroid(mu.m10/mu.m00 , mu.m01/mu.m00);
    cv::Point org;//original pt

    cv::Point xend = centroid;
    _mandible.y = centroid.y;
    _centroid.y = centroid.y;
    _centroid.x = centroid.x;
    xend.x += xmax/4.0;
    QVector<cv::Point> vec;
    for(uint i = 0; i < headVec.size()  ; i++)
    {
        if(headVec[i].y == qRound(centroid.y) && headVec[i].x < centroid.x )
        {
            vec.append(headVec[i]);
        }
    }
    int minn = 100000;
    int index=0;
    for(int i = 0; i < vec.size();i++)
    {
        if(vec[i].x < minn)
        {
            minn = vec[i].x;
            index = i;
        }
    }
    org = vec[index];
    _mandible.x = org.x;
    QVector<int> y;
    int ma = 0;
    int mi = 200000;
    for(uint i = 0; i < headVec.size(); i++)
    {
        if(headVec[i].x == qRound(org.x+BACKWARDPIX))
        {
            y.append(headVec[i].y);
            if(headVec[i].y > ma && (headVec[i].y-centroid.y)<30)
            {
                ma = headVec[i].y;
            }
            if(headVec[i].y < mi&& (-headVec[i].y+centroid.y)<30)
            {
                mi = headVec[i].y;
            }
        }
    }
    if(y.size() > 1)
    {
        org.y = (mi+ma)/2.0;
    }
    int thickness = 1;
    int lineType = 8;
    double w = 100;
    //x-axis
    line(tp,org,xend,cv::Scalar( 255, 255, 255 ),thickness,lineType );
    //y-axis
    cv::Point yend = org;
    yend.y += ymax/2.0;
    line(tp,org,yend,cv::Scalar( 255, 255, 255 ),thickness,lineType );
    //centroid
    circle(tp,centroid,w/32.0,cv::Scalar( 0, 0, 255 ),-1,lineType );
    //mandible
    circle(tp,_mandible,w/32.0,cv::Scalar( 255, 255, 255 ),-1,lineType );
    //org pt
    circle(tp,org,w/32.0,cv::Scalar( 0, 0, 255 ),-1,lineType );
   // imshow("dfs",tp);
    return _mandible;
}

cv::Point Preprocess::getMandible()
{
    return _mandible;
}

cv::Point Preprocess::getCentroid()
{
    return _centroid;
}

void Preprocess::brightnessDistortion(cv::Mat &frame)
{
    cv::Mat numeratorG = cv::Mat_<float>(frame.size());
    cv::Mat numeratorB = cv::Mat_<float>(frame.size());
    cv::Mat denominatorR = cv::Mat_<float>(frame.size());
    cv::Mat denominatorG = cv::Mat_<float>(frame.size());
    cv::Mat denominatorB = cv::Mat_<float>(frame.size());
    for(int i = 0 ; i < _shadowMask.rows; i++)
    {
        for(int j = 0; j < _shadowMask.cols; j++)
        {
           _shadowMask.at<float>(i,j) = frame.at<cv::Vec3b>(i,j)[2]/255.0 * _mean.at<cv::Vec3f>(i,j)[2] / pow(_std.at<cv::Vec3f>(i,j)[2],2);
           numeratorG.at<float>(i,j) = frame.at<cv::Vec3b>(i,j)[1]/255.0 * _mean.at<cv::Vec3f>(i,j)[1] / pow(_std.at<cv::Vec3f>(i,j)[1],2);
           numeratorB.at<float>(i,j) = frame.at<cv::Vec3b>(i,j)[0]/255.0 * _mean.at<cv::Vec3f>(i,j)[0] / pow(_std.at<cv::Vec3f>(i,j)[0],2);
           denominatorR.at<float>(i,j) = pow(_mean.at<cv::Vec3f>(i,j)[2] / _std.at<cv::Vec3f>(i,j)[2],2);
           if(denominatorR.at<float>(i,j)==0)
               denominatorR.at<float>(i,j) = 1;
           denominatorG.at<float>(i,j) = pow(_mean.at<cv::Vec3f>(i,j)[1] / _std.at<cv::Vec3f>(i,j)[1],2);
           if(denominatorG.at<float>(i,j) == 0 )
               denominatorG.at<float>(i,j) = 1;
           denominatorB.at<float>(i,j) = pow(_mean.at<cv::Vec3f>(i,j)[0] / _std.at<cv::Vec3f>(i,j)[0],2);
           if(denominatorB.at<float>(i,j) == 0 )
               denominatorB.at<float>(i,j) = 1;
        }
    }
    _shadowMask = _shadowMask + numeratorG + numeratorB;
    denominatorR = denominatorR + denominatorG + denominatorB;
    _shadowMask = _shadowMask / denominatorR;
    //imshow("alpha",_shadowMask);
    for(int i = 0; i < _shadowMask.rows ; i++)
    {
        for(int j = 0; j < _shadowMask.cols; j++)
        {
            if(_shadowMask.at<float>(i,j) > SHADOWMASKTHRESHOLD)
            {
                _shadowMask.at<float>(i,j) = 1;
            }
            else
            {
                _shadowMask.at<float>(i,j) = 0;
            }
        }
    }
   // imshow("d",_shadowMask);
}

bool Preprocess::legalRect(int x, int y, int width, int height, std::vector<cv::Point> &beeHead)
{
    cv::Mat temp = cv::Mat_<float>( height,width);
    if(width < _shadowMask.cols && height < _shadowMask.rows)
       _shadowMask.colRange(x,x + width ).rowRange(y,y + height  ).copyTo(temp);
    /*
     * Start of "Exclusion of false measurement"
     */
    cv::Point p0( x , y );
    cv::Point p1( x + width , y );
    cv::Point p2( x , y + height  );
    cv::Point p3( x + width , y + height );
    if( (pointPolygonTest(beeHead,p0,true) >= 0) &&
        (pointPolygonTest(beeHead,p1,true) >= 0) &&
        (pointPolygonTest(beeHead,p2,true) >= 0) &&
        (pointPolygonTest(beeHead,p3,true) >= 0))//totally in beehead
    {
        return false;
    }

    if( (pointPolygonTest(beeHead,p0,true) < 0) &&
        (pointPolygonTest(beeHead,p1,true) < 0) &&
        (pointPolygonTest(beeHead,p2,true) < 0) &&
        (pointPolygonTest(beeHead,p3,true) < 0)) // not attach to beeHead
    {
       // cv::namedWindow("reject in 2",cv::WINDOW_NORMAL);
       // imshow("reject in 2",temp);
        return false;
    }
    /*
     * End of "Exclusion of false measurement"
     */


    return true;
}

bool Preprocess::shadowRemoval(int x,int y, int width,int height)
{
    //double area = contourArea(contours_poly) ;
    cv::Mat temp = cv::Mat_<float>( height,width);
   // std::vector<std::vector<cv::Point> > contours;
    if(width < _shadowMask.cols && height < _shadowMask.rows)
       _shadowMask.colRange(x,x + width ).rowRange(y,y + height  ).copyTo(temp);

    float sum = 0;
    for( int i = 0; i < temp.rows; i++ )
    {
        for(int j = 0; j < temp.cols; j++)
        {
            //cv::Point p( i + x ,j + y );
           // if(pointPolygonTest(contours_poly,p,true) >= 0)
            {
                sum += temp.at<float>(i,j);
            }
        }
    }
    if(sum > width * height * 0.95)
    {
        //for( uint i =0 ; i< contours_poly.size();i++)
     //   {
          //  cv::Point pt;
        //    pt.x = contours_poly[i].x - x-1;
       //     pt.y = contours_poly[i].y - y-1;
       //     contour.push_back(pt);
      //  }
      //  contours.push_back(contour);
       // cv::drawContours(temp,contours,-1,cv::Scalar(0,0,255),2);
       // cv::namedWindow("reject in shadow removal",cv::WINDOW_NORMAL);
       // imshow("reject in shadow removal",temp);
        return false;
    }

   // cv::waitKey();
    return true;
}

bool Preprocess::splitBBox(cv::Rect &bbox, cv::Mat fore, cv::Point mandible,std::vector<cv::Rect>& newRect)
{
    int xm = mandible.x;
    int ym = mandible.y;
    if( bbox.x < xm && bbox.x + bbox.width > xm &&
        bbox.y < ym && bbox.y + bbox.height > ym &&
        bbox.width > 50 && bbox.height > 50 )
    {
        cv::Mat I;
        fore.colRange(bbox.x,bbox.x + bbox.width).rowRange(bbox.y,bbox.y + bbox.height).copyTo(I);
        //float fudgeFactor = 0.7;
        int seD_v1 = 5;
        int seD_v2 = 3;
        /*
         * erode the image
         */
        cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(seD_v1,seD_v1));
        cv::Mat I_erode;
        cv::erode( I, I_erode, element );
        /*
         * dilate the Image
         */
        element = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(seD_v2,seD_v2));
        cv::Mat I_dilate;
        cv::dilate( I_erode, I_dilate, element );

        /*
         * detect edges using Canny Edge Detector
         */
        cv::Mat bw;
        cv::Canny(I_dilate,bw,100,200);

        /*
         *fill holes in the edges
         */
        cv::Mat bwfill = imFillHoles(bw);

        /*
         *denoise again
         */
        bwareaopen(bwfill,100);

        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(bwfill,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
        if( contours.size() > 1 )
        {
            std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
            std::vector<cv::Rect> boundRect( contours.size() );
            for( uint i = 0; i < contours.size(); i++ )
            {
                approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 5, true );
                boundRect[i] = boundingRect( cv::Mat(contours_poly[i]) );
            }
            sortRows(boundRect);
            for( uint i = 0; i < contours.size(); i++ )
            {
                if( boundRect[i].width * boundRect[i].height < 0.31 * (float)bbox.width * (float)bbox.height && boundRect[i].width > 0.39 * (float)bbox.width )
                {
                    newRect.push_back(boundRect[i]);

                }
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

QVector<int> Preprocess::sortRows(std::vector<cv::Rect >& legalBoundRectVec)
{
   // std::vector<cv::Rect > legalBoundRect = legalBoundRectVec;
    QVector<int> index;
    for(uint i =0; i < legalBoundRectVec.size(); i++)
    {
        index.push_back(i);
    }
    for(uint i =0; i < legalBoundRectVec.size(); i++)
    {
        for(uint j = 0; j < legalBoundRectVec.size(); j++)
        {
            if(legalBoundRectVec[i].y < legalBoundRectVec[j].y)
            {
                cv::Rect tmp = legalBoundRectVec[i];
                legalBoundRectVec[i] = legalBoundRectVec[j];
                legalBoundRectVec[j] = tmp;
                int t = index[i];
                index[i] = index[j];
                index[j] =t;
            }
        }
    }
    return index;
}

void Preprocess::sortRows(QVector<cv::Point2f> &centers2)
{
    for( int i = 0; i < centers2.size(); i++ )
    {
        for( int j = i + 1; j < centers2.size(); j++ )
        {
            if( centers2[i].x > centers2[j].x )
            {
                cv::Point2f tmp = centers2[i];
                centers2[i] = centers2[j];
                centers2[j] = tmp;
            }
        }
    }
}

cv::Mat Preprocess::imFillHoles(cv::Mat input)
{
    cv::Mat imShow = cv::Mat::zeros(input.size(),CV_8UC3);    // for show result

    //
    // detect external contours
   //
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(input, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    //
    // fill external contours
    //
    if( !contours.empty() && !hierarchy.empty() )
    {
        for (uint idx=0;idx < contours.size();idx++)
        {
            cv::drawContours(imShow,contours,idx,cv::Scalar::all(255),CV_FILLED,8);
        }
    }

    cv::Mat imFilledHoles;
    cv::cvtColor(imShow,imFilledHoles,CV_BGR2GRAY);
    imFilledHoles = imFilledHoles > 0;

    // release temp Mat
    imShow.release();

    return imFilledHoles;
}

void Preprocess::bwareaopen(cv::Mat& im, double size)
{
    im.convertTo(im,CV_8UC1,255.0);
    // Only accept CV_8UC1
    if (im.channels() != 1 || im.type() != CV_8U)
        return;
    // Find all contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(im.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    for (uint i = 0; i < contours.size(); i++)
    {
        // Calculate contour area
        double area = cv::contourArea(contours[i]);
        // Remove small objects by drawing the contour with black color
        if (area > 0 && area <= size)
            cv::drawContours(im, contours, i, CV_RGB(0,0,0), -1);
    }
    im.convertTo(im,CV_32F,1.0/255);
}

void Preprocess::find(const cv::Mat& data,float number, Relation type,QVector<int>& rows, QVector<int>& cols) //TODO: Need to take care of the type of Mat, CV_32FC3 default
{
    for( int i = 0; i < data.rows; i++ )
    {
        for( int j = 0; j < data.cols; j++ )
        {
            switch (type) {
            case EQUAL:
                if( data.at<float>(i,j) == number )
                {
                    rows.push_back(i);
                    cols.push_back(j);
                }
            case GREATER:
                if( data.at<float>(i,j) > number )
                {
                    rows.push_back(i);
                    cols.push_back(j);
                }
                break;
            default:
                if( data.at<float>(i,j) < number )
                {
                    rows.push_back(i);
                    cols.push_back(j);
                }
                break;
            }
        }
    }
}
