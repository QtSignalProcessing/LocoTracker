#ifndef IOHANDLE_H
#define IOHANDLE_H

#include "featureextraction.h"

#include <QVector>

#include <opencv2/opencv.hpp>

class IOHandle
{
public:
    IOHandle();
    void writeLegalRect2File(QVector<std::vector<cv::Rect> >& legalBoundRectVec,QString& dirName);
    void readLegalRectFromFile(QVector<std::vector<cv::Rect> >& legalBoundRectVec,QString& fileName);
    void writeFeature2File(QVector< QVector<FeatureExtraction::feature> >& features,QString& dirName);
    void readFeatureFromFile(QVector< QVector<FeatureExtraction::feature> >& features,QString& fileName);
    void writeManAndcen2File(cv::Point& mandible,cv::Point& centroid,QString& dirName);
    void readManCenFromFile(cv::Point& mandible,cv::Point& centroid,QString& fileName);
    void writeSeq2File(QVector< QVector<int> >& colorIndex,QString& dirName);
	void readSeqFromFile(QVector< QVector<int> >& colorIndex,QString& fileName);
    void writeProb2file(QVector<float>& probList,QString& dirName);
	void readProbFromFile(QVector<float>& probList,QString& dirName);
    void writeBBoxImg();
	void exportTrackList2CSV(const QString& filename,const QVector<cv::Mat>& trackListNew);
};

#endif // IOHANDLE_H
