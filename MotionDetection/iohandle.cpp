#include "iohandle.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>

#include "opencv2/imgproc/imgproc.hpp"

#include <qdebug.h>

IOHandle::IOHandle(){}

void IOHandle::writeLegalRect2File(QVector<std::vector<cv::Rect> > &legalBoundRectVec, QString &dirName)
{
    //1: frame number; 2: sequence in the frame; 3: rect.x; 4: rect.y; 5: rect.width; 6: rect.height
    QFile file(dirName+"/legalRectPos.txt");
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&file);
    for(int i = 0; i < legalBoundRectVec.size(); i++)
    {
        for( uint j = 0; j < legalBoundRectVec[i].size(); j++ )
        {
            out << i <<" "<< (int)j << " "<< legalBoundRectVec[i][j].x <<" "<<legalBoundRectVec[i][j].y<<" "<<legalBoundRectVec[i][j].width<<" "<<legalBoundRectVec[i][j].height<<"\n";
        }
    }
    file.close();
}

void IOHandle::readLegalRectFromFile(QVector<std::vector<cv::Rect> >& legalBoundRectVec,QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream in(&file);
    legalBoundRectVec.clear();
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        int frameNo = fields.at(0).toInt();
        int bIndex = fields.at(1).toInt();
        int x = fields.at(2).toInt();
        int y = fields.at(3).toInt();
        int width = fields.at(4).toInt();
        int height = fields.at(5).toInt();
        int bboxSize = legalBoundRectVec.size();
        while( bboxSize < frameNo )
        {
            std::vector<cv::Rect> vec;
            legalBoundRectVec.push_back(vec);
            bboxSize = legalBoundRectVec.size();
        }
        cv::Rect r(x,y,width,height);
        if( frameNo + 1 > bboxSize )
        {
            std::vector<cv::Rect> vec;
            vec.push_back(r);
            legalBoundRectVec.push_back(vec);
        }
        else
        {
            if( (int)legalBoundRectVec[frameNo].size() >= bIndex )
                legalBoundRectVec[frameNo].push_back(r);
        }
    }
    file.close();
}

void IOHandle::writeFeature2File(QVector<QVector<FeatureExtraction::feature> > &features, QString &dirName)
{
    //1: frame number; 2: sequence in the frame; 3:
    QFile file(dirName+"/Features.txt");
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&file);
    for(int i = 0; i < features.size(); i++)
    {
        for( int j = 0; j < features[i].size(); j++ )
        {
            out << i<<" "<<j<<" "<< features[i][j].distNO<<" "<<features[i][j].xFarest<<" "<<features[i][j].darkRegionArea<<" "<<
                   features[i][j].xComponentOfMV<<" "<<features[i][j].yComponentOfMV<<" "<<features[i][j].mandibleInBb<<" ";
            for(int k = 0; k < features[i][j].edge_counter.size(); k++)
            {
                if(k<features[i][j].edge_counter.size()-1)
                    out<<features[i][j].edge_counter[k]<<" ";
                else
                    out<<features[i][j].edge_counter[k];
            }
            out<<"\n";
        }
    }
    file.close();
}

void IOHandle::readFeatureFromFile(QVector<QVector<FeatureExtraction::feature> > &features, QString &fileName)
{
    features.clear();
    QFile file(fileName);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream in(&file);
    while( !in.atEnd() )
    {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        int frameNo = fields.at(0).toInt();
        int bIndex = fields.at(1).toInt();
        int featureSize = features.size();
        while( featureSize < frameNo )
        {
            QVector<FeatureExtraction::feature> vec;
            features.push_back(vec);
            featureSize = features.size();
        }

        FeatureExtraction::feature tmp;
        tmp.distNO = fields.at(2).toFloat();
        tmp.xFarest = fields.at(3).toFloat();
        tmp.darkRegionArea = fields.at(4).toFloat();
        tmp.xComponentOfMV = fields.at(5).toFloat();
        tmp.yComponentOfMV = fields.at(6).toFloat();
        if(fields.at(7).toInt()!=0)
            tmp.mandibleInBb = true;
        else tmp.mandibleInBb = false;
        tmp.edge_counter[0]=(fields.at(8).toInt());
        tmp.edge_counter[1]=(fields.at(9).toInt());
        tmp.edge_counter[2]=(fields.at(10).toInt());
        tmp.edge_counter[3]=(fields.at(11).toInt());
        tmp.edge_counter[4]=(fields.at(12).toInt());
        if( frameNo + 1 > featureSize )
        {
            QVector<FeatureExtraction::feature> vec;
             vec.push_back(tmp);
             features.push_back(vec);
        }
        else
        {
            if( features[frameNo].size() >= bIndex )
                features[frameNo].push_back(tmp);
        }
    }
   /* while(_features.size() < _legalBoundRectVec.size())
    {
        QVector<FeatureExtraction::feature> vec;
        _features.push_back(vec);
    }*/
    file.close();
}

void IOHandle::writeManAndcen2File(cv::Point &mandible, cv::Point &centroid, QString &dirName)
{
    QFile file(dirName+"/Mandible and centroid.txt");
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&file);
    out<<mandible.x<<" "<<mandible.y<<" "<<centroid.x<<" "<<centroid.y;
}

void IOHandle::readManCenFromFile(cv::Point &mandible, cv::Point &centroid, QString &fileName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream in(&file);
    QString line = in.readLine();
    QStringList fields = line.split(" ");
    mandible.x = fields.at(0).toInt();
    mandible.y = fields.at(1).toInt();
    centroid.x = fields.at(2).toInt();
    centroid.y = fields.at(3).toInt();
    file.close();
}

void IOHandle::writeSeq2File(QVector<QVector<int> > &colorIndex, QString &dirName)
{
    //1: frame number; 2: sequence in the frame; 3:
    QFile file(dirName+"/Sequence.txt");
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&file);
    for(int i = 0; i < colorIndex.size(); i++)
    {
        for( int j = 0; j < colorIndex[i].size(); j++ )
        {
            out << i<<" "<<j<<" "<< colorIndex[i][j];
            out<<"\n";
        }
    }
    file.close();
}

void IOHandle::readSeqFromFile(QVector< QVector<int> >& colorIndex,QString& fileName)
{
	QFile file(fileName);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream in(&file);
    colorIndex.clear();
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        int frameNo = fields.at(0).toInt();
        int bIndex = fields.at(1).toInt();
        int color = fields.at(2).toInt();
        int bboxSize = colorIndex.size();
        while( bboxSize < frameNo )
        {
            QVector<int> vec;
            colorIndex.push_back(vec);
            bboxSize = colorIndex.size();
        }
        if( frameNo + 1 > bboxSize )
        {
            QVector<int> vec;
            vec.push_back(color);
            colorIndex.push_back(vec);
        }
        else
        {
            if( (int)colorIndex[frameNo].size() >= bIndex )
                colorIndex[frameNo].push_back(color);
        }
    }
    file.close();
}

void IOHandle::writeProb2file(QVector<float> &probList, QString &dirName)
{
    QFile file(dirName+"/Probilities.txt");
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&file);
    for(int i = 0; i < probList.size(); i++)
    {
        out << i<<" "<< probList[i];
        out<<"\n";
    }
    file.close();
}

void IOHandle::readProbFromFile(QVector<float>& probList,QString& fileName)
{
	QFile file(fileName);
    file.open(QIODevice::ReadOnly|QIODevice::Text);
    QTextStream in(&file);
	probList.clear();
	while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        probList.push_back(fields.at(1).toInt());
    }
    file.close();
}

void IOHandle::writeBBoxImg()
{
    //write the boundbox to img
  /*  cv::Mat tmp;
    cv::Mat tmp1 = cv::Mat::ones(180,180,CV_8UC3);
    tmp1.setTo(cv::Scalar(255,255,255));
   // qDebug()<<_legalBoundRectVec[_currentFrame][i].x<<" "<<_legalBoundRectVec[_currentFrame][i].x+_legalBoundRectVec[_currentFrame][i].width<<" "<<_legalBoundRectVec[_currentFrame][i].y<<" "<<_legalBoundRectVec[_currentFrame][i].y+_legalBoundRectVec[_currentFrame][i].height;
    frame.colRange(_legalBoundRectVec[_currentFrame][i].tl().x,_legalBoundRectVec[_currentFrame][i].tl().x+_legalBoundRectVec[_currentFrame][i].width).rowRange(_legalBoundRectVec[_currentFrame][i].tl().y,_legalBoundRectVec[_currentFrame][i].tl().y+_legalBoundRectVec[_currentFrame][i].height).copyTo(tmp);
    cv::Rect roi = cv::Rect((tmp1.cols-tmp.cols)/2,(tmp1.rows-tmp.rows)/2,tmp.cols,tmp.rows);
    cv::Mat d = tmp1(roi);
    tmp.copyTo(d);
    imshow("sdfsf",tmp1);
    QDir dir;
    dir.mkdir("boundingBox");
    QString name1 ="boundingBox/"+QString::number(k) +  ".png";
  //  cout<<name1.toStdString()<<endl;
    imwrite(name1.toStdString(),tmp1);
    k++;*/
    //end of writing

    // Another way of converting cv::Mat to QImage
    /* QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
      for (int y = 0; y < src.rows; ++y)
      {
             const cv::Vec3b *srcrow = src[y];
             QRgb *destrow = (QRgb*)dest.scanLine(y);
             for (int x = 0; x < src.cols; ++x)
             {
                     destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
             }
      }
      return dest;*/
}

void IOHandle::exportTrackList2CSV(const QString& filename,const QVector<cv::Mat>& trackListNew)
{	
	int ind = filename.indexOf(".");
    QString fname = filename;
    fname.remove(ind,filename.length()-1);
	QFile* file = new QFile(fname+" track.csv");
	if( file->exists() )
	{
		file->remove();
		delete file;
		file = new QFile(fname+" track.csv");
	}
    file->open(QIODevice::WriteOnly|QIODevice::Text);
	int index = filename.indexOf("part");
	QString tmp = filename;
	tmp.remove(index-1,filename.length() - index+1);
	int num = fname.at(index+4).digitValue();
    QTextStream out(file);
	//write the first row of the excel
    for(int i = 0; i < 80; i++)
    {
		if( i % 16 == 0 )
			out <<tmp<<",";
		else
			out<<num<<",";
    }
	out<<"\n";
    //write second row
	num = filename.at(ind-1).digitValue();
	for(int i = 0; i < 80; i++)
    {
		if( i % 16 == 0 )
			out <<"bee"<<",";
		else
			out<<num<<",";
    }
	out<<"\n";
	//write third row
	num = 0;
	for(int i = 0; i < 80; i++)
    {
		if( i % 16 == 0 )
		{
			out <<"track"<<",";
			num++;
		}
		else
			out<<num<<",";
    }
	out<<"\n";
	//write the forth row
	QStringList fields;
	fields<<"frameno"<<"xbbox"<<"ybbox"<<"width"<<"height"<<"xc"<<"yc"<<"xm"<<"ym"<<"xtip"<<"ytip"<<"angle"<<"distance_c"<<"distance_d"<<"light"<<"sugar";
	for(int i = 0; i < 80; i++)
    {
		out <<fields[i%16]<<",";
    }
	out<<"\n";
    qDebug()<<trackListNew.size();
	int frameno = trackListNew[0].rows;
	for( int i = 0; i < frameno; i++ )
	{
		for( int j = 0; j < 6; j++ )
		{
			for( int k = 0; k < trackListNew[0].cols; k++ )
				out<<trackListNew[j].at<double>(i,k)<<",";
		}
		out<<"\n";
	}
    file->close();
}


