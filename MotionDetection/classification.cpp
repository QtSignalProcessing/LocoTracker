#include "classification.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

const int SAMPLE_NUM = 30;
const int FEATURE_NUM = 11;

#include <QDebug>
#include <iostream>
using namespace std;

using namespace cv;

Classification::Classification(const QVector< QVector<FeatureExtraction::feature> >& trainingData)
    :_trainingDataMat(cv::Mat(SAMPLE_NUM,FEATURE_NUM,CV_32FC1)),
      _labelsMat(cv::Mat(SAMPLE_NUM,1,CV_32FC1)),_SVM(),_shift(cv::Mat::zeros(1,FEATURE_NUM,CV_32F)),_scalingF(cv::Mat::ones(1,FEATURE_NUM,CV_32F))
{
    setTrainingData(trainingData);
    //automatically centers the data points at their mean, and scales them to have unit standard deviation, before training.
    for( int i = 0; i < _trainingDataMat.cols; i++ )
    {
        cv::Scalar avg;
        cv::Scalar ssd;
        cv::meanStdDev(_trainingDataMat.col(i),avg,ssd);
        _shift.at<float>(i) = -avg[0];
        if( ssd[0] != 0 )
        {
             _scalingF.at<float>(i) = 1.0/(ssd[0] * sqrt(30.0/29.0));
        }
        else
            _scalingF.at<float>(i) = 1;
        _trainingDataMat.col(i) = _scalingF.at<float>(i) * (_trainingDataMat.col(i) + (_shift.at<float>(i)));
    }


    // Set up SVM's parameters
   // CvParamGrid CvParamGrid_C(pow(2.0,-40), pow(2.0,40), 2);
  //  CvParamGrid CvParamGrid_gamma(pow(2.0,-40), pow(2.0,40), 2);
    CvSVMParams params;
    params.svm_type    = CvSVM::C_SVC;
    params.kernel_type = CvSVM::RBF;
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 1e-6);
    params.gamma = 1;
    params.C = 1;
    // Train the SVM
    _SVM.train(_trainingDataMat, _labelsMat, Mat(), Mat(), params);
   // _SVM.train_auto(_trainingDataMat, _labelsMat, Mat(), Mat(), params);
  //  _SVM.train_auto(_trainingDataMat, _labelsMat, Mat(), Mat(), params,4,CvParamGrid_C,CvParamGrid_gamma);
}

void Classification::setTrainingData(const QVector<QVector<FeatureExtraction::feature> > &trainingData)
{
    int k = 0;
    for(int i = 0; i < trainingData.size(); i++ )
    {
        for( int j = 0; j < trainingData[i].size(); j++)
        {
            FeatureExtraction::feature tmp = trainingData[i][j];
            feature2Mat(tmp,k,_trainingDataMat); 
            _labelsMat.at<float>(k,0) = i+1;
            k++;
        }
    }
}

void Classification::feature2Mat(const FeatureExtraction::feature &tmp, int k,cv::Mat& target)
{
    target.at<float>(k,0) = tmp.distNO;
    target.at<float>(k,1) = tmp.xFarest;
    target.at<float>(k,2) = tmp.darkRegionArea;
    target.at<float>(k,3) = tmp.xComponentOfMV;
    target.at<float>(k,4) = tmp.yComponentOfMV;
    if(tmp.mandibleInBb)
        target.at<float>(k,5) = 1;
    else
        target.at<float>(k,5) = 0;
    target.at<float>(k,6) = tmp.edge_counter[0];
    target.at<float>(k,7) = tmp.edge_counter[1];
    target.at<float>(k,8) = tmp.edge_counter[2];
    target.at<float>(k,9) = tmp.edge_counter[3];
    target.at<float>(k,10) = tmp.edge_counter[4];
}

QVector<int> Classification::getClassifiedLabels(const QVector<FeatureExtraction::feature> &samples)
{
    QVector<int> labels;
    for(int i = 0; i < samples.size(); i++)
    {
        cv::Mat target(1,11,CV_32FC1);
        feature2Mat(samples[i],0,target);

        for( int i = 0; i < target.cols; i++ )
        {
            target.at<float>(i) = _scalingF.at<float>(i) * ( target.at<float>(i) + _shift.at<float>(i) );
           // cout<<target.at<float>(i)<<" ";
        }

        int response = _SVM.predict(target);
      /*  if( samples[i].mandibleInBb )
            response = 2;
        else if( response == 2 && !samples[i].mandibleInBb )
            response = 0;*/
        labels.push_back(response);
    }
    return labels;
}
