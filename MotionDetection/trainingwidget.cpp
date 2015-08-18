#include "trainingwidget.h"

#include "featureextraction.h"
#include "displaywidget.h"

#include <QImage>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

#include <QDebug>

const int NUMOFTRAININGFRAMES = 10;

TrainingWidget::TrainingWidget(VideoProcessing* video,QWidget *parent) :
    QWidget(parent),_video(video),_imgLabel(new QLabel(this)),_object(),_radioButtonList(),_distNM(new QLabel(this)),_distFL(new QLabel(this)),
    _area(new QLabel(this)),_xComponentOfMV(new QLabel(this)),_yComponentOfMV(new QLabel(this)),_cm(new QLabel(this)),_edgeCounter1(new QLabel(this)),
    _edgeCounter2(new QLabel(this)),_edgeCounter3(new QLabel(this)),_edgeCounter4(new QLabel(this)),_edgeCounter5(new QLabel(this)),  _frameNum(new QSpinBox(this)),
    _trainingFeatures(3),_trained(false),_tmpFeature()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    init();
    mainLayout->addWidget(_imgLabel);
    QGroupBox* objectGroupBox = new QGroupBox("Object",this);
    objectGroupBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    QRadioButton *radio1 = new QRadioButton("Antenna",this);
    QRadioButton *radio2 = new QRadioButton("Mandible",this);
    QRadioButton *radio3 = new QRadioButton("Proboscis",this);
    connect(radio1,SIGNAL(clicked()),this,SLOT(updatLabels()));
    connect(radio2,SIGNAL(clicked()),this,SLOT(updatLabels()));
    connect(radio3,SIGNAL(clicked()),this,SLOT(updatLabels()));
    _radioButtonList.push_back(radio1);
    _radioButtonList.push_back(radio2);
    _radioButtonList.push_back(radio3);
    radio1->setChecked(true);
    QGridLayout *vbox = new QGridLayout(objectGroupBox);
    vbox->addWidget(radio1,1,1);
    vbox->addWidget(radio2,1,2);
    vbox->addWidget(radio3,1,3);
    objectGroupBox->setLayout(vbox);
    mainLayout->addWidget(objectGroupBox);
    QGroupBox* featureGroup = new QGroupBox("Features",this);
    featureGroup->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    QLabel* distNML = new QLabel("Distance1:",this);
    QHBoxLayout* firstL = new QHBoxLayout;
    firstL->addWidget(distNML);
    firstL->addWidget(_distNM);
    QLabel* distFLL = new QLabel("Distance2:",this);
    QHBoxLayout* secondL = new QHBoxLayout;
    secondL->addWidget(distFLL);
    secondL->addWidget(_distFL);
    QLabel* areaL= new QLabel("area of dark region:",this);
    QHBoxLayout* thirdL = new QHBoxLayout;
    thirdL->addWidget(areaL);
    thirdL->addWidget(_area);
    QLabel* xComponentOfMVL= new QLabel("X Component Of MV:",this);
    QHBoxLayout* forthL = new QHBoxLayout;
    forthL->addWidget(xComponentOfMVL);
    forthL->addWidget(_xComponentOfMV);
    QLabel* yComponentOfMVL= new QLabel("Y Component Of MV:",this);
    QHBoxLayout* fifthL = new QHBoxLayout;
    fifthL->addWidget(yComponentOfMVL);
    fifthL->addWidget(_yComponentOfMV);
    QLabel* cmL= new QLabel("Contain manible:",this);
    QHBoxLayout* sixthL = new QHBoxLayout;
    sixthL->addWidget(cmL);
    sixthL->addWidget(_cm);
    QLabel* edgeCount1 = new QLabel("Edge Counter 1:",this);
    QHBoxLayout* seventhL = new QHBoxLayout;
    seventhL->addWidget(edgeCount1);
    seventhL->addWidget(_edgeCounter1);
    QLabel* edgeCount2 = new QLabel("Edge Counter 2:",this);
    QHBoxLayout* eighthL = new QHBoxLayout;
    eighthL->addWidget(edgeCount2);
    eighthL->addWidget(_edgeCounter2);
    eighthL->addStretch();
    QLabel* edgeCount3 = new QLabel("Edge Counter 3:",this);
    QHBoxLayout* ninethL = new QHBoxLayout;
    ninethL->addWidget(edgeCount3);
    ninethL->addWidget(_edgeCounter3);
    ninethL->addStretch();
    QLabel* edgeCount4 = new QLabel("Edge Counter 4:",this);
    QHBoxLayout* tenthL = new QHBoxLayout;
    tenthL->addWidget(edgeCount4);
    tenthL->addWidget(_edgeCounter4);
    tenthL->addStretch();
    QLabel* edgeCount5 = new QLabel("Edge Counter 5:",this);
    QHBoxLayout* elethL = new QHBoxLayout;
    elethL->addWidget(edgeCount5);
    elethL->addWidget(_edgeCounter5);
    elethL->addStretch();
    QGridLayout *featureLayout = new QGridLayout(featureGroup);
    featureLayout->addLayout(firstL,1,1);
    featureLayout->addLayout(secondL,1,2);
    featureLayout->addLayout(thirdL,1,3);
    featureLayout->addLayout(forthL,2,1);
    featureLayout->addLayout(fifthL,2,2);
    featureLayout->addLayout(sixthL,2,3);
    featureLayout->addLayout(seventhL,3,1);
    featureLayout->addLayout(eighthL,3,2);
    featureLayout->addLayout(ninethL,3,3);
    featureLayout->addLayout(tenthL,4,1);
    featureLayout->addLayout(elethL,4,2);
    featureGroup->setLayout(featureLayout);
    mainLayout->addWidget(featureGroup);
    QLabel* numOfFrames = new QLabel("Number of Frames",this);
    QHBoxLayout* frameNumLayout = new QHBoxLayout;
    frameNumLayout->addWidget(numOfFrames);
    frameNumLayout->addWidget(_frameNum);
    _frameNum->setRange(0,10);
    connect(_frameNum,SIGNAL(valueChanged(int)),this,SLOT(frameNumChanged(int)));
    mainLayout->addLayout(frameNumLayout);
    QPushButton* clearButton = new QPushButton("Clear image",this);
    QPushButton* pushButton = new QPushButton("Train",this);
    connect(pushButton,SIGNAL(clicked()),this,SLOT(training()));
    connect(clearButton,SIGNAL(clicked()),this,SLOT(clear()));
    QGroupBox* buttonGroup = new QGroupBox("Options",this);
    QGridLayout* buttonLayout = new QGridLayout(buttonGroup);
    QPushButton* storeButton = new QPushButton("Store",this);
    connect(storeButton,SIGNAL(clicked()),this,SLOT(store()));
    QPushButton* finishButton = new QPushButton("Finish",this);
    connect(finishButton,SIGNAL(clicked()),this,SLOT(finish()));
    QPushButton* clearCurrentButton = new QPushButton("Re-select Sample",this);
    QPushButton* clearAllButton = new QPushButton("Re-select All samples",this);
    connect(clearAllButton,SIGNAL(clicked()),this,SLOT(clearAll()));
    connect(clearCurrentButton,SIGNAL(clicked()),this,SLOT(clearCurrent()));
    QPushButton* loadCurrentB = new QPushButton("Load Training Features",this);
    QPushButton* loadAllB = new QPushButton("Load All training Features",this);
    connect(loadCurrentB,SIGNAL(clicked()),this,SLOT(loadCurrent()));
    connect(loadAllB,SIGNAL(clicked()),this,SLOT(loadAll()));
    buttonLayout->addWidget(clearButton,1,1);
    buttonLayout->addWidget(pushButton,1,2);
    buttonLayout->addWidget(clearCurrentButton,2,1);
    buttonLayout->addWidget(clearAllButton,2,2);
    buttonLayout->addWidget(storeButton,3,1);
    buttonLayout->addWidget(finishButton,3,2);
    buttonLayout->addWidget(loadCurrentB,4,1);
    buttonLayout->addWidget(loadAllB,4,2);
    buttonGroup->setLayout(buttonLayout);
    mainLayout->addWidget(buttonGroup);
    setLayout(mainLayout);
    this->setWindowTitle("LocoTracker");
}

void TrainingWidget::setImg(cv::Mat img)
{
    cv::Mat tmp1;
    cv::cvtColor(img, tmp1, CV_BGR2RGB);
    _object = img;
   // imshow("test",_object);
    QImage tmp((uchar*) tmp1.data, tmp1.cols, tmp1.rows, tmp1.step, QImage::Format_RGB888);
    _imgLabel->setPixmap(QPixmap::fromImage(tmp));
    _imgLabel->update();
}

void TrainingWidget::training()
{
    if(_object.size().width == 0 || _object.size().height == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("Please select a region of interest");
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        qDebug()<<ret;
    }
    else
    {
     /*   std::vector<cv::Rect>legalBoundRect;
        cv::Rect r(_video->getForeground()->getStartPt().x(),_video->getForeground()->getStartPt().y(),
                   _video->getForeground()->getEndpt().x()-_video->getForeground()->getStartPt().x(),
                   _video->getForeground()->getEndpt().y()-_video->getForeground()->getStartPt().y());
        legalBoundRect.push_back(r);
        cv::Mat t = _video->getFrame();
        if(t.channels()!=1)
            cvtColor( t, t, CV_BGR2GRAY );
        FeatureExtraction f(_video->getMandible(),_video->getCentroid(),legalBoundRect,t,_video->getPreFrame(),_video->isFirstTime());*/
        int index = getActiveIndex();
        QString msg;
        switch(index)
        {
            case ANTENNA:
                if(_trainingFeatures[ANTENNA].size()<NUMOFTRAININGFRAMES)
                {
                    _trainingFeatures[ANTENNA].push_back( _tmpFeature );
                }
               else
                   msg = QString("Enough antenna features, please train others and/or store the trained features or re-training them/it");
                break;
            case MANDIBLE:
                if(_trainingFeatures[MANDIBLE].size()< NUMOFTRAININGFRAMES)
                {
                    _trainingFeatures[MANDIBLE].push_back(_tmpFeature);
                }
                else
                    msg = QString("Enough mandible features, please train others and/or store the trained features or re-train them/it");
                break;
            case PROBOSCIS:
                if(_trainingFeatures[PROBOSCIS].size()<NUMOFTRAININGFRAMES)
                {
                    _trainingFeatures[PROBOSCIS].push_back(_tmpFeature);
                }
                else
                    msg = QString("Enough right proboscis features, please train others and/or store the trained features or re-train them/it");
                break;

        }
        if(msg.size()!=0)
        {
            messageBox(msg);
            return;
        }
        setLabels(_tmpFeature,index);
    }  
}

void TrainingWidget::clear()
{
    cv::Mat temp;
    _object = temp;
    _imgLabel->setText("No Object Selected");
    _imgLabel->update();
    _distFL->setText(QString::number(0));
    _distNM->setText(QString::number(0));
    _area->setText(QString::number(0));
    _xComponentOfMV->setText(QString::number(0));
    _yComponentOfMV->setText(QString::number(0));
    _cm->setText("No");
    _edgeCounter1->setText(QString::number(0));
    _edgeCounter2->setText(QString::number(0));
    _edgeCounter3->setText(QString::number(0));
    _edgeCounter4->setText(QString::number(0));
    _edgeCounter5->setText(QString::number(0));
    _frameNum->setValue(0);
}

void TrainingWidget::closeEvent(QCloseEvent *event)
{
    if( !_trained )
    {
        QMessageBox msgBox;
        msgBox.setText("Not Trained. Do you want to exit?");
        QString s("Yes");
        QString s1("No");
        QPushButton* yesButton = msgBox.addButton(s,QMessageBox::AcceptRole);
        QPushButton* noButton = msgBox.addButton(s1,QMessageBox::RejectRole);
        msgBox.exec();
        if( msgBox.clickedButton() == yesButton )
        {
            emit closed(false);
            event->accept();
        }
        else if( msgBox.clickedButton() == noButton )
            event->ignore();
        return;
    }
    else
        event->accept();
}

void TrainingWidget::init()
{
    _imgLabel->setText("No Object Selected");
    _distFL->setText(QString::number(0));
    _distNM->setText(QString::number(0));
    _area->setText(QString::number(0));
    _xComponentOfMV->setText(QString::number(0));
    _yComponentOfMV->setText(QString::number(0));
    _cm->setText("No");
    _edgeCounter1->setText(QString::number(0));
    _edgeCounter2->setText(QString::number(0));
    _edgeCounter3->setText(QString::number(0));
    _edgeCounter4->setText(QString::number(0));
    _edgeCounter5->setText(QString::number(0));
    _frameNum->setValue(_trainingFeatures[ANTENNA].size());
}

void TrainingWidget::setLabels(const FeatureExtraction::feature &feature,int index,int framNum)
{
    _distNM->setText(QString::number(feature.distNO));
    _distFL->setText(QString::number(feature.xFarest));
    _area->setText(QString::number(feature.darkRegionArea));
    _xComponentOfMV->setText(QString::number(feature.xComponentOfMV));
    _yComponentOfMV->setText(QString::number(feature.yComponentOfMV));
    if(feature.mandibleInBb)
        _cm->setText("Yes");
    else _cm->setText("No");
    _edgeCounter1->setText(QString::number(feature.edge_counter[0]));
    _edgeCounter2->setText(QString::number(feature.edge_counter[1]));
    _edgeCounter3->setText(QString::number(feature.edge_counter[2]));
    _edgeCounter4->setText(QString::number(feature.edge_counter[3]));
    _edgeCounter5->setText(QString::number(feature.edge_counter[4]));
    if(framNum == 0)
        _frameNum->setValue(_trainingFeatures[index].size());
    else
        _frameNum->setValue(framNum);
}

void TrainingWidget::finish()
{
    bool ok = errorMessage();
    _trained = ok;
    if(ok)
    {
        qDebug()<<_trainingFeatures[0].size()<<" "<<_trainingFeatures[1].size()<<" "<<_trainingFeatures[2].size();
        this->close();
        emit traingFinish();
    }
}

bool TrainingWidget::getTrained()
{
    return _trained;
}

void TrainingWidget::store()
{
    int index = getActiveIndex();
    QString text;
    QString fileName;
    bool ok = true;
    switch(index)
    {
        case ANTENNA:
        if(_trainingFeatures[ANTENNA].size()<NUMOFTRAININGFRAMES)
        {
            text = QString("Need more antenna training samples");
            ok = false;
        }
        break;
        case MANDIBLE:
        if(_trainingFeatures[MANDIBLE].size()< NUMOFTRAININGFRAMES)
        {
            text = QString("Need more manible training samples");
            ok = false;
        }
        break;
        case PROBOSCIS:
        if(_trainingFeatures[PROBOSCIS].size()<NUMOFTRAININGFRAMES)
        {
            text = QString("Need more probosics training samples");
            ok = false;
        }
    }
    if(!ok)
    {
        messageBox(text);
    }
    else
    {
        QVector<FeatureExtraction::feature> data;
        fileName = QFileDialog::getSaveFileName(this,"Save as","","Text Files(*.txt)");
        if (fileName.isEmpty())
              return;
        switch(index)
        {
            case ANTENNA:

         //   fileName = QString("AntTrainingFeatures.txt");
            data = _trainingFeatures[ANTENNA];
            break;
            case MANDIBLE:
         //   fileName = QString("ManTrainingFeatures.txt");
            data = _trainingFeatures[MANDIBLE];
            break;
            case PROBOSCIS:
         //   fileName = QString("probTrainingFeatures.txt");
            data = _trainingFeatures[PROBOSCIS];
            break;
        }
        writToFile(fileName,data);
    }

}

inline int TrainingWidget::getActiveIndex()
{
    int index;
    for(int i = 0 ;i < _radioButtonList.size();i++)
    {
        if(_radioButtonList[i]->isChecked())
        {
            index = i;
        }
    }
    return index;
}

bool TrainingWidget::errorMessage()
{
    QString text;
    bool ok = true;
    if( _trainingFeatures[ANTENNA].size() < NUMOFTRAININGFRAMES )
    {
        text = QString("Need more antenna training samples");
        ok = false;
    }
    else if( _trainingFeatures[MANDIBLE].size() < NUMOFTRAININGFRAMES )
    {
        text = QString("Need more manible training samples");
        ok = false;
    }
    else if( _trainingFeatures[PROBOSCIS].size() < NUMOFTRAININGFRAMES )
    {
        text = QString("Need more probosics training samples");
        ok = false;
    }
    if(!ok)
    {
        messageBox(text);
    }
    return ok;
}

void TrainingWidget::messageBox(QString message)
{
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    qDebug()<<ret;
}

void TrainingWidget::writToFile(QString fileName, const QVector<FeatureExtraction::feature> &vec)
{
    QFile file(fileName);
    file.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&file);
    for(int i = 0; i < vec.size(); i++)
    {
        out << vec[i].distNO<<" "<<vec[i].xFarest<<" "<<vec[i].darkRegionArea<<" "<<
               vec[i].xComponentOfMV<<" "<<vec[i].yComponentOfMV<<" "<<vec[i].mandibleInBb<<" ";
        for(int j = 0; j < vec[i].edge_counter.size(); j++)
        {
            if(j<vec[i].edge_counter.size()-1)
                out<<vec[i].edge_counter[j]<<" ";
            else
                out<<vec[i].edge_counter[j];
        }
        out<<"\n";
    }
    file.close();
    fileName += " stored!";
    messageBox(fileName);
}

void TrainingWidget::clearAll()
{
    int index = getActiveIndex();
    switch(index)
    {
        case ANTENNA:
        if(_trainingFeatures[ANTENNA].size() > 0)
        {
            _trainingFeatures[ANTENNA].clear();
            init();
            _frameNum->setValue(_trainingFeatures[ANTENNA].size());;
        }
        break;
        case MANDIBLE:
        if(_trainingFeatures[MANDIBLE].size() > 0)
        {
            _trainingFeatures[MANDIBLE].clear();
            init();
            _frameNum->setValue(_trainingFeatures[MANDIBLE].size());
        }
        break;
        case PROBOSCIS:
        if(_trainingFeatures[PROBOSCIS].size() > 0)
        {
            _trainingFeatures[PROBOSCIS].clear();
            init();
            _frameNum->setValue(_trainingFeatures[PROBOSCIS].size());
        }
        break;
    }
    clear();
}

void TrainingWidget::clearCurrent()
{
    int index = getActiveIndex();
    switch(index)
    {
        case ANTENNA:
        if(_trainingFeatures[ANTENNA].size() > 0)
        {
            _trainingFeatures[ANTENNA].pop_back();
            init();
            _frameNum->setValue(_trainingFeatures[ANTENNA].size());
        }
        break;
        case MANDIBLE:
        if(_trainingFeatures[MANDIBLE].size() > 0)
        {
            _trainingFeatures[MANDIBLE].pop_back();
            init();
            _frameNum->setValue(_trainingFeatures[MANDIBLE].size());
        }
        break;
        case PROBOSCIS:
        if(_trainingFeatures[PROBOSCIS].size() > 0)
        {
            _trainingFeatures[PROBOSCIS].pop_back();
            init();
            _frameNum->setValue(_trainingFeatures[PROBOSCIS].size());
        }
        break;
    }
    clear();
}

void TrainingWidget::loadCurrent()
{
    int index = getActiveIndex();
    QString fileName;
    switch(index)
    {
        case ANTENNA:
        fileName = QString("AntTrainingFeatures.txt");
        break;
        case MANDIBLE:
        fileName = QString("ManTrainingFeatures.txt");
        break;
        case PROBOSCIS:
        fileName = QString("probTrainingFeatures.txt");
        break;
    }
    if(_trainingFeatures[index].size() == NUMOFTRAININGFRAMES)
    {
        QString msg("Already loaded");
        messageBox(msg);
        return;
    }
    readFromFile(fileName,index);

}

void TrainingWidget::loadAll()
{
    QStringList filenames;
   // filenames.append(QString("AntTrainingFeatures.txt"));
   // filenames.append(QString("ManTrainingFeatures.txt"));
   // filenames.append(QString("probTrainingFeatures.txt"));
    filenames.append(QString("ann.txt"));
    filenames.append(QString("man.txt"));
    filenames.append(QString("prob.txt"));
    for(int i = 0; i < filenames.size(); i++)
    {
        readFromFile(filenames[i],i);
    }
    _trained = true;
    emit traingFinish();
}

bool TrainingWidget::readFromFile(QString fileName,int index)
{
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while(!in.atEnd())
        {
            QString line = in.readLine();
            QStringList fields = line.split(" ");
            FeatureExtraction::feature tmp;
            tmp.distNO = fields.at(0).toFloat();
            tmp.xFarest = fields.at(1).toFloat();
            tmp.darkRegionArea = fields.at(2).toFloat();
            tmp.xComponentOfMV = fields.at(3).toFloat();
            tmp.yComponentOfMV = fields.at(4).toFloat();
            if(fields.at(5).toInt()!=0)
                tmp.mandibleInBb = true;
            else tmp.mandibleInBb = false;
            tmp.edge_counter[0]=(fields.at(6).toInt());
            tmp.edge_counter[1]=(fields.at(7).toInt());
            tmp.edge_counter[2]=(fields.at(8).toInt());
            tmp.edge_counter[3]=(fields.at(9).toInt());
            tmp.edge_counter[4]=(fields.at(10).toInt());
            _trainingFeatures[index].push_back(tmp);
        }
        setLabels(_trainingFeatures[index][0],index);
    }
    else
    {
        QString msg("No Training Samples Stored!");
        messageBox(msg);
        return false;
    }
    return true;
}

void TrainingWidget::updatLabels()
{
    int i = getActiveIndex();
    if(_trainingFeatures[i].size() == 0)
    {
        clear();
        return;
    }
    if(_frameNum->value() < _trainingFeatures[i].size())
        setLabels(_trainingFeatures[i][_frameNum->value()],i);
    else
    {
        setLabels(_trainingFeatures[i][_trainingFeatures[i].size()-1],i);
        _frameNum->setValue(_trainingFeatures[i].size());
    }
}

void TrainingWidget::frameNumChanged(int num)
{
    int index = getActiveIndex();
    if(_trainingFeatures[index].size() == 0 || num == 0 )
    {
        clear();
        return;
    }

    if( num <= _trainingFeatures[index].size() )
    {
        setLabels(_trainingFeatures[index][num-1],index,num);
    }
    else
    {
        setLabels(_trainingFeatures[index][_trainingFeatures[index].size()-1],index);
       // _frameNum->blockSignals(true);
       // _frameNum->setValue(_trainingFeatures[index].size());
       // _frameNum->blockSignals(false);
        QString msg("No more samples");
        messageBox(msg);
    }
}

const QVector< QVector<FeatureExtraction::feature> >& TrainingWidget::getTrainingFeatures()
{
    return _trainingFeatures;
}

void TrainingWidget::setFeature(FeatureExtraction::feature f)
{
    _tmpFeature = f;
}
