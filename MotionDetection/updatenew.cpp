#include "updatenew.h"

#include <QVector>
#include <QStringList>
#include "HungarianAlg.h"

#include <QDebug>
#include <QFile>
#include"iostream"
using namespace std;

UpdateNew::UpdateNew(){}

void UpdateNew::update(QVector<float> &pL, const QVector<QVector<FeatureExtraction::feature> > &features, QVector<QVector<int> > &seqList)
{
    /*
     * Initial ltablelist
     */
    int l = pL.size();
    QVector<cv::Mat> sortProbList;
     QVector<cv::Mat> ltableList(l);
    for( int i = 0;i < seqList.size(); i++ )
    {
        int s = seqList[i].size();
        cv::Mat tmp = cv::Mat::zeros(s,3,CV_64F);
        if( s != 0 )
        {

            for( int j = 0; j < s; j++ )
            {
                if( seqList[i][j] == 1 || seqList[i][j] == 5 )
                    tmp.at<double>(j,0) = 1;
                else if( seqList[i][j] == 2 || seqList[i][j] == 4 )
                    tmp.at<double>(j,1) = 1;
                else
                    tmp.at<double>(j,2) = 1;
            }
        }
        ltableList[i] = tmp;
    }
    sortProbList = ltableList;
    /*
     *end
     */

   // sortProbList = ltableList;
  //  for( int i = 0; i < ltableList.size(); i++ )
       // sortProbList.push_back(ltableList[i]);

   //  cv::Mat cost = cv::Mat::zeros(3,1,CV_32F);
    // cv::Mat visit = cv::Mat::zeros(3,1,CV_32F);
   //  int n = 14;
     int iter = 0;
     while( iter < 10 )
     {
     for( int n = 1; n < l-1; n++ )
     {
     if( pL[n] > 0.99 )
     {
         QVector<int> seq = seqList[n];
         QVector<FeatureExtraction::feature> sortFeature = features[n];
         cv::Mat ltable = ltableList[n];
         cv::Mat sortProb = sortProbList[n];
       //  visit.at<float>(n) = 1;
         for( int k = -1; k <= 1; k += 2 )
         {
             if( pL[n+k] >= 0 && pL[n+k] < 1 && seq.size() > 1 && pL[n+k] != -1 && seqList[n+k].size() >= 1 )
             {
               // visit.at<float>(n+k) = 1;
                cv::Mat target = cv::Mat::zeros(sortFeature.size(),2,CV_64F);
                cv::Mat source = cv::Mat::zeros(features[n+k].size(),2,CV_64F);
                for( int i = 0; i < sortFeature.size(); i++ )
                {
                    target.at<double>(i,0) = sortFeature[i].distNO;
                    target.at<double>(i,1) = sortFeature[i].xFarest;
                }
                for( int i = 0; i < features[n+k].size(); i++ )
                {
                    source.at<double>(i,0) = features[n+k][i].distNO;
                    source.at<double>(i,1) = features[n+k][i].xFarest;
                }
                QVector<int> targetIndice = HungarianLinker(source,target);
                int length = source.rows;
                cv::Mat counter = cv::Mat::zeros(3,1,CV_32F);
                QVector<int> tempseq;
                for( int j = 0; j < length; j++ )
                {
                    if( targetIndice.size() > j && targetIndice[j] >=0 && targetIndice[j] < seq.size() )
                    {
                        tempseq.push_back(targetIndice[j]);
                        switch (seq[targetIndice[j]])
                        {
                        case 1:
                            counter.at<float>(0) += 1;
                            break;
                        case 2:
                            counter.at<float>(1) += 1;
                            break;
                        case 3:
                            counter.at<float>(2) += 1;
                            break;
                        case 4:
                            counter.at<float>(1) += 1;
                            break;
                        case 5:
                            counter.at<float>(0) += 1;
                            break;
                        }
                    }
                }
               // for( int i =0;i<3;i++ )
                  //  qDebug()<<counter.at<float>(i);
                bool inorder = isUniqueSorted(tempseq);
                bool allpos = true;
                for( int i = 0; i < length; i++ )
                {
                    if( targetIndice[i] < 0 )
                    {
                        allpos = false;
                    }
                }
                if( inorder && allpos )
                {

                    for( int j = 0; j < length; j++ )
                    {
                        pL[n+k] = 1;
                        ltable.row(targetIndice[j]).copyTo(ltableList[n+k].row(j));
                        sortProb.row(targetIndice[j]).copyTo(sortProbList[n+k].row(j));
                        seqList[n+k][j] = seq[targetIndice[j]];
                    }
                }

                else if( inorder )
                {

                    for( int j = 0; j < length; j++ )
                    {
                        if( targetIndice[j] < 0 )
                        {
                            if( counter.at<float>(0)> 2 || counter.at<float>(1) > 2 || counter.at<float>(2) > 1 )
                            {
                                pL[n+k] = 0;
                            }
                            else if( counter.at<float>(0) == 2 )//not antenna
                            {
                                if( sortProbList[n+k].at<double>(j,1) + sortProbList[n+k].at<double>(j,2) != 0 )
                                {
                                    sortProbList[n+k].at<double>(j,0) = 0;
                                    sortProbList[n+k].at<double>(j,1) /= ( sortProbList[n+k].at<double>(j,1) + sortProbList[n+k].at<double>(j,2) );
                                    sortProbList[n+k].at<double>(j,2) = 1 - sortProbList[n+k].at<double>(j,1);
                                }
                                else
                                {
                                    sortProbList[n+k].at<double>(j,0) = 0;
                                    sortProbList[n+k].at<double>(j,1) = 0.5;
                                    sortProbList[n+k].at<double>(j,2) = 0.5;
                                }

                                if( sortProbList[n+k].at<double>(j,1) < sortProbList[n+k].at<double>(j,2) )//ltable is [0 0 1],tongue
                                {
                                    ltableList[n+k].at<double>(j,0) = 0;
                                    ltableList[n+k].at<double>(j,1) = 1;
                                    ltableList[n+k].at<double>(j,2) = 0;
                                    seqList[n+k][j] = 3;
                                    pL[n+k] = sortProbList[n+k].at<double>(j,2);
                                }
                                else if( sortProbList[n+k].at<double>(j,1) > sortProbList[n+k].at<double>(j,2) )//ltable is [0 1 0], mandible
                                {
                                    ltableList[n+k].at<double>(j,0) = 0;
                                    ltableList[n+k].at<double>(j,1) = 1;
                                    ltableList[n+k].at<double>(j,2) = 0;
                                    pL[n+k] = sortProbList[n+k].at<double>(j,1);
                                    if( counter.at<float>(1) == 0 )
                                        seqList[n+k][j] = 2;
                                    else
                                        seqList[n+k][j] = 4;
                                }
                            }// end of counter.at<float>(0) == 2

                            else if( counter.at<float>(1) == 2 )//not mandible
                            {
                                if( sortProbList[n+k].at<double>(j,0) + sortProbList[n+k].at<double>(j,2) != 0 )
                                {
                                    sortProbList[n+k].at<double>(j,1) = 0;
                                    sortProbList[n+k].at<double>(j,0) /= (sortProbList[n+k].at<double>(j,0) + sortProbList[n+k].at<double>(j,2));
                                    sortProbList[n+k].at<double>(j,2) = 1 - sortProbList[n+k].at<double>(j,0);
                                }
                                else
                                {
                                    sortProbList[n+k].at<double>(j,0) = 0;
                                    sortProbList[n+k].at<double>(j,1) = 0.5;
                                    sortProbList[n+k].at<double>(j,2) = 0.5;
                                }

                                if( sortProbList[n+k].at<double>(j,0) < sortProbList[n+k].at<double>(j,2) )//ltable is [0 0 1],tongue
                                {
                                    ltableList[n+k].at<double>(j,0) = 0;
                                    ltableList[n+k].at<double>(j,1) = 0;
                                    ltableList[n+k].at<double>(j,2) = 1;
                                    seqList[n+k][j] = 3;
                                    pL[n+k] = sortProbList[n+k].at<double>(j,2);
                                }
                                else if( sortProbList[n+k].at<double>(j,0) > sortProbList[n+k].at<double>(j,2) )//ltable is [1 0 0],antenna
                                {
                                    ltableList[n+k].at<double>(j,0) = 1;
                                    ltableList[n+k].at<double>(j,1) = 0;
                                    ltableList[n+k].at<double>(j,2) = 0;
                                    if( counter.at<float>(0) == 0 )
                                        seqList[n+k][j] = 1;
                                    else
                                        seqList[n+k][j] = 5;
                                    pL[n+k] = sortProbList[n+k].at<double>(j,0);
                                }
                            }//end of counter.at<double>(1) == 2

                            else if( counter.at<float>(2) == 1 )
                            {
                                if( sortProbList[n+k].at<double>(j,0) + sortProbList[n+k].at<double>(j,1) != 0 )
                                {
                                    sortProbList[n+k].at<double>(j,2) = 0;
                                    sortProbList[n+k].at<double>(j,0) /= (sortProbList[n+k].at<double>(j,0)+sortProbList[n+k].at<double>(j,1));
                                    sortProbList[n+k].at<double>(j,1) = 1 - sortProbList[n+k].at<double>(j,0);
                                }
                                else
                                {
                                    sortProbList[n+k].at<double>(j,2) = 0;
                                    sortProbList[n+k].at<double>(j,0) = 0.5;
                                    sortProbList[n+k].at<double>(j,1) = 0.5;
                                }

                                if( sortProbList[n+k].at<double>(j,0) < sortProbList[n+k].at<double>(j,1) ) //ltable is [0 1 0],mandible
                                {
                                    ltableList[n+k].at<double>(j,0) = 0;
                                    ltableList[n+k].at<double>(j,1) = 1;
                                    ltableList[n+k].at<double>(j,2) = 0;
                                    if( counter.at<float>(1) == 0 )
                                        seqList[n+k][j] = 2;
                                    else
                                        seqList[n+k][j] = 4;
                                    pL[n+k] = sortProbList[n+k].at<double>(j,1);
                                }
                                else if( sortProbList[n+k].at<double>(j,0) > sortProbList[n+k].at<double>(j,1) )//ltable is [1 0 0],antenna
                                {
                                    ltableList[n+k].at<double>(j,0) = 1;
                                    ltableList[n+k].at<double>(j,1) = 0;
                                    ltableList[n+k].at<double>(j,2) = 0;
                                    if( counter.at<float>(0) == 0 )
                                        seqList[n+k][j] = 1;
                                    else
                                        seqList[n+k][j] = 5;
                                    pL[n+k] = sortProbList[n+k].at<double>(j,0);
                                }

                            }// end of counter.at<double>(2) == 1

                        }// end of if( targetIndice[j] < 0 )
                        else
                        {
                            ltable.row(targetIndice[j]).copyTo(ltableList[n+k].row(j));
                            sortProb.row(targetIndice[j]).copyTo(sortProbList[n+k].row(j));
                            seqList[n+k][j] = seq[targetIndice[j]];
                        }
                    }//end of for loop in else if( inorder )
                    inorder = isUniqueSorted(seqList[n+k]);
                    if( !inorder )
                    {
                        pL[n+k] = 0;
                        for( int j = 0; j < length; j++ )
                            if( targetIndice[j] < 0 )
                                seqList[n+k][j] = 0;
                    }
                }// end of else if( inorder )
                else
                    if( pL[n+k] != 0 )
                        pL[n+k] = 1;
                /*for( int i = 0; i< sortProb.rows;i++ )
                {
                    cout<<sortProb.at<double>(i,0)<<" "<<sortProb.at<double>(i,1)<<" "<<sortProb.at<double>(i,2);
                }*/
             }//TODO
         }
     }
     }
     iter++;
     }
  /*  for( int i = 0; i < seqList.size(); i++ )
     {
         cout<<i+1<<" ";
         for( int j = 0; j < seqList[i].size(); j++ )
         {
             cout<<seqList[i][j]<<" ";
         }
         cout<<endl;
     }*/
}

bool UpdateNew::isUniqueSorted(const QVector<int>& seq)
{
    bool result;
    for( int i = 0; i < seq.size()-1; i++ )
    {
        if( seq[i] < seq[i+1] )
        {
            result = true;
        }
        else
        {
            result = false;
            return result;
        }
    }
    return true;
}

void UpdateNew::intoF(int *a, FeatureExtraction::feature &f)
{
    f.distNO = a[0];
    f.xFarest = a[1];
    f.darkRegionArea = a[2];
    f.xComponentOfMV = a[3];
    f.yComponentOfMV = a[4];
    f.mandibleInBb = a[5];
    f.edge_counter[0] = a[6];
    f.edge_counter[1] = a[7];
    f.edge_counter[2] = a[8];
    f.edge_counter[3] = a[9];
    f.edge_counter[4] = a[10];
}

QVector<int> UpdateNew::HungarianLinker(const cv::Mat &source, const cv::Mat &target)
{
    int numOFSourcePt = source.rows;//2
    int numOfTargetPt = target.rows;//3
    cv::Mat scr = source.clone();
    cv::Mat D = cv::Mat::ones(numOFSourcePt,numOfTargetPt,CV_64F);
    for( int i = 0; i < numOFSourcePt; i++ )
    {
     //   cv::Mat currentPt(1,source.cols,CV_64F);
       // source.row(i).copyTo(currentPt);
       // cv::Mat tmp;
        cv::Mat diff_coords = cv::Mat::zeros(numOfTargetPt,source.cols,CV_64F);
        for( int k = 0; k < numOfTargetPt; k++ )
            scr.row(i).copyTo(diff_coords.row(k));
            //scr.row(i).copyTo(tmp);
        diff_coords = target - diff_coords;

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
    vector< vector<double> >dist;
    for( int i=0;i<D.rows;i++ )
    {
        vector<double> tmp;
        for( int j=0;j<D.cols;j++ )
        {
            tmp.push_back(D.at<double>(i,j));
        }
        dist.push_back(tmp);
    }
    vector<int> labels;
    AssignmentProblemSolver s;
    s.Solve(dist,labels,AssignmentProblemSolver::many_forbidden_assignments);
  //  for( int i =0;i < (int)labels.size();i++ )
       // qDebug()<<labels[i];
  //  Munkres m;
   // m.diag(true);
  //  m.solve(D);
    QVector<int> optVec = QVector<int>::fromStdVector(labels);
   // optVec.fromStdVector(labels);
   return optVec;
}
