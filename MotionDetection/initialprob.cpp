#include "initialprob.h"


#include "preprocess.h"
#include "trainingwidget.h"

#include <cmath>

#include <QDebug>
#include "iostream"
using namespace std;

InitialProb::InitialProb( cv::Point& mandible,cv::Point& centroid):
    _seq(),_prob(),_colorIndex(),_ltableList(),_sortProbList(),_sortFeatureList(),_centroid(centroid),_mandible(mandible){}

InitialProb::InitialProb(){}

void InitialProb::setFrameNum(std::vector<cv::Rect> &legalBoundRectVec,QVector<FeatureExtraction::feature> &features,QVector<int> &labels,cv::Mat& frame)
{
    int frameBoundRectsize = legalBoundRectVec.size();
    cv::Mat ltable=cv::Mat::zeros(frameBoundRectsize,3,CV_64F);
    cv::Mat sortProb =cv::Mat::zeros(frameBoundRectsize,3,CV_64F);
    QVector<int> colorIndex;
    if( frameBoundRectsize > 0 ) //go through the frame
    {
        cv::Mat fgmask;
        fgmask = getFgmask(frame);
        int frameBoundRectsize = legalBoundRectVec.size();
        cv::Mat post = cv::Mat::zeros(frameBoundRectsize,3,CV_64F);
        int index;
        for( int j = 0; j < frameBoundRectsize; j++ )
        {
            post.at<double>(j,labels[j]-1) = 1;
            if( frameBoundRectsize == 1 )
                index = labels[j]-1;
        }
        Preprocess preprocess;
        QVector<int> orgIndex = preprocess.sortRows(legalBoundRectVec);
        QVector<FeatureExtraction::feature> tmp = features;
        cv::Mat sortFeature = cv::Mat::zeros(frameBoundRectsize,features[0].numOfFields,CV_64F);// not used in computation
        for( int j = 0; j < frameBoundRectsize; j++)
        {
            post.row(orgIndex[j]).copyTo(sortProb.row(j));
            sortProb.row(j).copyTo(ltable.row(j));
            sortFeature.at<double>(j,0) = features[orgIndex[j]].distNO;
            sortFeature.at<double>(j,1) = features[orgIndex[j]].xFarest;
            sortFeature.at<double>(j,2) = features[orgIndex[j]].darkRegionArea;
            sortFeature.at<double>(j,3) = features[orgIndex[j]].xComponentOfMV;
            sortFeature.at<double>(j,4) = features[orgIndex[j]].yComponentOfMV;
            if( features[orgIndex[j]].mandibleInBb )
                sortFeature.at<double>(j,5) = 1;
            else
                sortFeature.at<double>(j,5) = 0;
            for( int k = 0; k < features[orgIndex[j]].edge_counter.size(); k++ )
            {
                sortFeature.at<double>(j,6 + k) = features[orgIndex[j]].edge_counter[k];
            }

            features[j] = tmp[orgIndex[j]];
        }
        _sortFeatureList.push_back(sortFeature);
        switch (frameBoundRectsize)
        {
            case 1:// only one bbox in the frame
            {
                switch (index)
                {
                    case TrainingWidget::ANTENNA:
                    findAntena(legalBoundRectVec[0],fgmask);
                    break;
                    case TrainingWidget::MANDIBLE:
                    findMandible(legalBoundRectVec[0],fgmask);
                    break;
                    case TrainingWidget::PROBOSCIS:
                    QVector<int> seq;
                    seq.push_back(3);
                    _prob.push_back(0.1);
                    _seq.push_back(seq);
                }
                break;
            }
            case 2: //2 bboxes in the frame
            {
                twoBoxesInFrame(frameBoundRectsize,ltable,legalBoundRectVec,fgmask);
                break;
            }
            case 3://3 boxes in the frame
            {
                threeBoxesInFrame(frameBoundRectsize,ltable,legalBoundRectVec,fgmask);
                break;
            }
            case 4:// 4 boxes in the frame
            {
                fourBoxesInFrame(frameBoundRectsize,ltable,legalBoundRectVec,fgmask);
                break;
            }
            case 5:// 5 boxes in the frame
            {
                fiveBoxesInFrame(frameBoundRectsize,ltable,legalBoundRectVec,fgmask);
                break;
            }
            default:
             _prob.push_back(-1);
             QVector<int> seq;
             for( int i = 0; i < frameBoundRectsize; i++ )
                 seq.push_back(0);
             _seq.push_back(seq);
                    break;
        }
        // draw bbox on frame

        for(int k = 0; k < frameBoundRectsize; k++ )
        {
            for( int j = 0; j < 3; j++ )
            {
                if(ltable.at<double>(k,j) ==1 )
                {
                    colorIndex.push_back(j);
                    break;
                }
            }
        }
    }
    else
    {
        _prob.push_back(-1);
        QVector<int> seq;
        _seq.push_back(seq);
    }
     _colorIndex.push_back(colorIndex);
     _ltableList.push_back(ltable);
     _sortProbList.push_back(sortProb);

}

inline void InitialProb::sortProb021(cv::Mat &sortProb, int k)
{
    sortProb.at<double>(k,0) = 0;
    sortProb.at<double>(k,2) = sortProb.at<double>(k,2) / ( sortProb.at<double>(k,2) + sortProb.at<double>(k,1) );
    sortProb.at<double>(k,1) = 1 - sortProb.at<double>(k,2);
}

inline void InitialProb::sortProb120(cv::Mat &sortProb, int k)
{
    sortProb.at<double>(k,1) = 0;
    sortProb.at<double>(k,2) = sortProb.at<double>(k,2) / ( sortProb.at<double>(k,2) + sortProb.at<double>(k,1) );
    sortProb.at<double>(k,0) = 1 - sortProb.at<double>(k,2);
}

inline void InitialProb::setLtable(cv::Mat &ltable, int k, int index)
{
    switch (index) {
    case 0:
        ltable.at<double>(k,0) = 1;
        ltable.at<double>(k,1) = 0;
        ltable.at<double>(k,2) = 0;
        break;
    case 1:
        ltable.at<double>(k,0) = 0;
        ltable.at<double>(k,1) = 1;
        ltable.at<double>(k,2) = 0;
        break;
    case 2:
        ltable.at<double>(k,0) = 0;
        ltable.at<double>(k,1) = 0;
        ltable.at<double>(k,2) = 1;
        break;
    default:
        break;
    }
}

inline bool InitialProb::contains3(const QVector<int> &seq)
{
    for( int i = 0; i < seq.size(); i++ )
    {
        if( seq[i] == 3 )
            return true;
    }
    return false;
}

cv::Mat InitialProb::getFgmask(const cv::Mat &frame)
{
    cv::Mat tmp;
    frame.convertTo(tmp,CV_32FC3,1.0/255.0);
    cv::cvtColor(tmp,tmp,CV_BGR2HSV);
    std::vector<cv::Mat> hsv;
    cv::split(tmp,hsv);
    cv::Scalar x = cv::mean(hsv[2]);
    for( int k = 0; k < hsv[2].rows; k++ )
    {
        for( int j = 0; j < hsv[2].cols; j++ )
        {
            hsv[2].at<float>(k,j) -= x[0];
            if(hsv[2].at<float>(k,j) < 0 )
                hsv[2].at<float>(k,j) = 1;
            else
                hsv[2].at<float>(k,j) = 0;
        }
    }
    return hsv[2];
}

void InitialProb::findAntena(const cv::Rect &rect,const cv::Mat& fgmask)
{
    cv::Point pointa = getPointa(rect,fgmask);
    QVector<int> seq;
    if( pointa.y < -25 )
    {
        seq.push_back(1);
        _prob.push_back(1);
    }
    else if( pointa.y > -25 && pointa.y < 0 )
    {
        seq.push_back(1);
        _prob.push_back(0.5);
    }
    else if( pointa.y < 25 && pointa.y >= 0 )
    {
        seq.push_back(5);
        _prob.push_back(0.5);
    }
    else
    {
        seq.push_back(5);
        _prob.push_back(1);
    }
    _seq.push_back(seq);
}

void InitialProb::findMandible(const cv::Rect &rect,const cv::Mat &fgmask)
{
    cv::Point pointa = getPointa(rect,fgmask);
    QVector<int> seq;
    if( pointa.y <= 0 )
    {
        seq.push_back(2);
        _prob.push_back(0.6);
    }
    else
    {
        seq.push_back(4);
        _prob.push_back(0.6);
    }
    _seq.push_back(seq);
}

cv::Point InitialProb::getPointa(const cv::Rect &rect,const cv::Mat &fgmask)
{
    cv::Mat tmp;
    fgmask.colRange(rect.x, rect.x + rect.width).rowRange(rect.y,rect.y + rect.height).copyTo(tmp);
    QVector<int> rows;
    QVector<int> cols;
    Preprocess preprocess;
    preprocess.bwareaopen(tmp,30);
    preprocess.find(tmp,1,Preprocess::EQUAL,rows,cols);
    cv::Point tip = getTip(rows,cols,rect,_centroid);
    cv::Point pointa = changeCoord(_centroid,tip);
    return pointa;
}

cv::Point InitialProb::getTip(QVector<int>& rows, QVector<int>& cols,const cv::Rect &rect,const cv::Point& centroid)
{
    cv::Point tip;
    if( rows.size() == cols.size() )
    {
        int max = 0;

        for( int j = 0; j < rows.size(); j ++ )
        {
            rows[j] += rect.y;
            cols[j] += rect.x;
            float dist;
           // if( !T )
            dist = ( rows[j] - centroid.y ) * ( rows[j] - centroid.y ) + (cols[j] - centroid.x ) * (cols[j] - centroid.x );
           // else
              //  dist = ( rows[j] - centroid.y ) * ( rows[j] - centroid.y );
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

cv::Point InitialProb::changeCoord(const cv::Point &centroid, const cv::Point &tip)
{
    cv::Point cor;
    cor.x = tip.x - _mandible.x;
    cor.y = tip.y - _mandible.y;
    double rho = std::sqrt((float)(cor.x * cor.x + cor.y * cor.y));
    double theta1 = std::atan((double)cor.y/(double)cor.x);
    if( theta1 * cor.y < 0 ) // changeCoordinate.m line : 21-37
    {
        theta1 += M_PI;
    }
    double theta2 = std::atan( (double)(_mandible.y - centroid.y) / (double)( _mandible.x - centroid.x ) );
    if( theta2 * ( centroid.y - _mandible.y ) < 0 )
    {
        theta1 += 0.5 * M_PI;
    }
    double theta = theta1 - theta2;
    cor.x = rho * std::cos(theta);
    cor.y = rho * std::sin(theta);
    return cor;
}

bool InitialProb::isUniqueSorted(const QVector<int>& seq)
{
    bool result;
    for( int i = 0; i < seq.size()-1; i++ )
    {
        if( seq[i] < seq[i+1] )
            result = true;
        else
        {
            result = false;
            return result;
        }
    }
    return true;
}

void InitialProb::twoBoxesInFrame(int frameBoundRectsize,const cv::Mat& sortProb,const std::vector<cv::Rect> &legalBoundRectVec,const cv::Mat &fgmask)
{
    cv::Mat counter = cv::Mat::zeros(3,1,CV_64F);
    QVector<int> seq;
    for( int k = 0; k < frameBoundRectsize; k++ )
    {
        if( sortProb.at<double>(k,0) == 1 && sortProb.at<double>(k,1) == 0 && sortProb.at<double>(k,2) == 0 )
        {
            if( counter.at<double>(0) == 0 )
            {
                counter.at<double>(0) = 1;
                seq.push_back(1);
            }
            else
                seq.push_back(5);
        }
        else if( sortProb.at<double>(k,0) == 0 && sortProb.at<double>(k,1) == 1 && sortProb.at<double>(k,2) == 0 )
        {
            if( counter.at<double>(1) == 0 )
            {
                cv::Point pointa = getPointa(legalBoundRectVec[k],fgmask);
                counter.at<double>(1) = 1;
                if( pointa.y <= 0 )
                {
                    seq.push_back(2);
                }
                else
                    seq.push_back(4);
            }
            else
                seq.push_back(4);
        }
        else
            seq.push_back(3);
    }
    float prob;
    if( isUniqueSorted(seq) )
    {
        if( !contains3(seq) )
        {
            if( seq[0] == 1 && seq[1] == 5 )
                prob = 1;
            else if( seq[0] == 2 && seq[1] ==4 )
                prob = 1;
            else
                prob = 0.3;
        }
        else
            prob = 0.3;
    }
    else
        prob = 0;
    _seq.push_back(seq);
    _prob.push_back(prob);
}

void InitialProb::threeBoxesInFrame(int frameBoundRectsize, cv::Mat &ltable, const std::vector<cv::Rect> &legalBoundRectVec,const cv::Mat &fgmask)
{
    cv::Mat tmp;
    QVector<int> seq;
    cv::reduce(ltable,tmp,0,CV_REDUCE_SUM);// 021 = 2
    if( tmp.at<double>(0) > 2 )
    {
        for( int i = 0;i < frameBoundRectsize; i++ )//modified
            seq.push_back(0);
    }

    float prob = 0;
    if( tmp.at<double>(0) <= 2 ) // if the number of antenna is less than or equal to 2
    {
        cv::Mat counter = cv::Mat::zeros(3,1,CV_64F);
        for( int k = 0; k < frameBoundRectsize; k++ )
        {
            if( ltable.at<double>(k,0) == 1 && ltable.at<double>(k,1) == 0 && ltable.at<double>(k,2) == 0 )
            {
                if( counter.at<double>(0) ==0 )
                {
                    counter.at<double>(0) = 1;
                    seq.push_back(1);
                }
                else
                    seq.push_back(5);
            }
            else if( ltable.at<double>(k,0) == 0 && ltable.at<double>(k,1) == 1 && ltable.at<double>(k,2) == 0 )
            {
                if( counter.at<double>(1) == 0 )
                {
                    cv::Point pointa = getPointa(legalBoundRectVec[k],fgmask);
                    counter.at<double>(1) = 1;
                    if( pointa.y <= 0 )
                        seq.push_back(2);
                    else
                        seq.push_back(4);
                }
                else
                    seq.push_back(4);
            }
            else
                seq.push_back(3);
        }
        if( isUniqueSorted(seq) )
        {
            if( seq[0] == 1 && seq[1] == 3 && seq[2] == 5 )
                prob = 1;
            else
                prob = 0.1;
        }
    }
    _seq.push_back(seq);
    _prob.push_back(prob);
}

void InitialProb::fourBoxesInFrame(int frameBoundRectsize,cv::Mat& ltable,const std::vector<cv::Rect> &legalBoundRectVec,const cv::Mat &fgmask)
{
    cv::Mat tmp;
    cv::reduce(ltable,tmp,0,CV_REDUCE_SUM);
    QVector<int> seq;
    
    float prob = 0 ;
    if( !( tmp.at<double>(0) > 2 || tmp.at<double>(1) > 2 ) )
    {
        cv::Mat counter = cv::Mat::zeros(3,1,CV_64F);
        for( int k = 0; k < frameBoundRectsize; k++ )
        {
            if( ltable.at<double>(k,0) == 1 && ltable.at<double>(k,1) == 0 && ltable.at<double>(k,2) == 0 )
            {
                if( counter.at<double>(0) == 0 )
                {
                    FeatureExtraction f;
                    cv::Point far;
                    cv::Point near;
                    f.computeNearestPt(_centroid,legalBoundRectVec[k],far,near);
                    cv::Point pointa = changeCoord(_centroid,near);
                    counter.at<double>(0) = 1;
                    if( pointa.y <= 0 )
                        seq.push_back(1);
                    else
                        seq.push_back(5);
                }
                else
                    seq.push_back(5);
            }
            else if( ltable.at<double>(k,0) == 0 && ltable.at<double>(k,1) == 1 && ltable.at<double>(k,2) == 2 )
            {
                if( counter.at<double>(1) == 0 )
                {
                    cv::Point pointa = getPointa(legalBoundRectVec[k],fgmask);
                    counter.at<double>(1) = 1;
                    if( pointa.y <= 0 )
                        seq.push_back(2);
                    else
                        seq.push_back(4);
                }
                else
                    seq.push_back(4);
            }
            else
                seq.push_back(3);
            if( isUniqueSorted(seq) )
            {
                prob = 0.7;
            }
        }
    }
	if( tmp.at<double>(0) > 2 || tmp.at<double>(1) > 2 || tmp.at<double>(2) > 1 ) //if the number of antenna/mandible is greater than 2
    {
		seq.clear();
        for( int i = 0; i < frameBoundRectsize; i++ )
            seq.push_back(0);
    }
	_seq.push_back(seq);
    _prob.push_back(prob);
}

void InitialProb::fiveBoxesInFrame(int frameBoundRectsize, cv::Mat &ltable, const std::vector<cv::Rect> &legalBoundRectVec, const cv::Mat &fgmask)
{
    cv::Mat tmp;
    cv::reduce(ltable,tmp,0,CV_REDUCE_SUM);
    QVector<int> seq;
    if( tmp.at<double>(0) > 2 || tmp.at<double>(1) > 2 )
        for( int i = 0; i < frameBoundRectsize; i++ )
            seq.push_back(0);

    float prob = 0;
    if( !(tmp.at<double>(0) > 2 || tmp.at<double>(1) >2) )
    {
        cv::Mat counter = cv::Mat::zeros(3,1,CV_64F);
        for( int k = 0; k < frameBoundRectsize; k++ )
        {
            if( ltable.at<double>(k,0) == 1 && ltable.at<double>(k,1) == 0 && ltable.at<double>(k,2) == 0 )
            {
                if( counter.at<double>(0) == 0 )
                {
                    FeatureExtraction f;
                    cv::Point far;
                    cv::Point near;
                    f.computeNearestPt(_centroid,legalBoundRectVec[k],far,near);
                    cv::Point pointa = changeCoord(_centroid,near);
                    counter.at<double>(0) = 1;
                    if( pointa.y <= 0 )
                        seq.push_back(1);
                    else
                        seq.push_back(5);
                }
                else
                    seq.push_back(5);
            }
            else if( ltable.at<double>(k,0) == 0 && ltable.at<double>(k,1) == 1 && ltable.at<double>(k,2) == 0 )
            {
                if( counter.at<double>(1) == 0 )
                {
                    cv::Point pointa = getPointa(legalBoundRectVec[k],fgmask);
                    counter.at<double>(1) = 1;
                    if( pointa.y <= 0 )
                        seq.push_back(2);
                    else
                        seq.push_back(4);
                }
                else
                    seq.push_back(4);
            }
            else
                seq.push_back(3);
        }
        if( isUniqueSorted(seq) )
        {
            prob = 1;
        }
    }
    _seq.push_back(seq);
    _prob.push_back(prob);
}

QVector< QVector<int> > InitialProb::getSeq()
{
    return _seq;
}

QVector<float> InitialProb::getProb()
{
    return _prob;
}

QVector< QVector<int> > InitialProb::getColorIndex()
{
    return _colorIndex;
}

QVector<cv::Mat>& InitialProb::getLtableList()
{
    return _ltableList;
}

QVector<cv::Mat>& InitialProb::getSortProbList()
{
    return _sortProbList;
}

void InitialProb::setLtableList(int indexOfList, int rowNum, const cv::Mat &source)
{
    source.row(0).copyTo(_ltableList[indexOfList].row(rowNum));
}

void InitialProb::setLtableList(int indexOfList, const cv::Mat &source)
{
    source.copyTo(_ltableList[indexOfList]);
}

void InitialProb::setSortProbList(int indexOfList, int rowNum, const cv::Mat &source)
{
    source.row(0).copyTo(_sortProbList[indexOfList].row(rowNum));
}

void InitialProb::setSeq(int indexOfLsit, int rowNum, int value)
{
    _seq[indexOfLsit][rowNum] = value;
}

void InitialProb::setSeq(int indexOfList, QVector<int> source)
{
    _seq[indexOfList] = source;
}

void InitialProb::setLtableList(int indexOfList, int rowNum,int colNum, double value)
{
    _ltableList[indexOfList].at<double>(rowNum,colNum) = value;
}

void InitialProb::setSortProbList(int indexOfList, int rowNum, int colNum, double value)
{
    _sortProbList[indexOfList].at<double>(rowNum,colNum) = value;
}

QVector<cv::Mat>& InitialProb::getSortFeatureList()
{
    return _sortFeatureList;
}

cv::Point InitialProb::getCentroid()
{
    return _centroid;
}

cv::Point InitialProb::getMandible()
{
    return _mandible;
}
