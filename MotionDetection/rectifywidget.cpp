#include "rectifywidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QRegExp>

#include <QDebug>

RectifyWidget::RectifyWidget(QImage& img,int cNum,int seq,float prob,QWidget *parent) :
    QWidget(parent),_sButton(new QPushButton("Save",this)),_cButton(new QPushButton("Cancel",this)),_doneButton(new QPushButton("Done",this)),_csEdit(new QLineEdit(this)),
    _groupBox3(new QGroupBox("Selection and/or Correction"))
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QGroupBox* groupBox1 = new QGroupBox("Tracking Hyp.");
    QVBoxLayout* fVLayout = new QVBoxLayout(groupBox1);
    QLabel* imgLabel = new QLabel(this);
    imgLabel->setPixmap(QPixmap::fromImage(img));
    QLabel* cLabel = new QLabel("Class Label:",this);
    QLabel* cNumL = new QLabel(QString::number(cNum),this);
    QHBoxLayout* cHLayout = new QHBoxLayout();
    cHLayout->addWidget(cLabel);
    cHLayout->addWidget(cNumL);
    QLabel* sLabel = new QLabel("Object Label:",this);
    QLabel* sNumL = new QLabel(QString::number(seq),this);
    QHBoxLayout* sHLayout = new QHBoxLayout();
    sHLayout->addWidget(sLabel);
    sHLayout->addWidget(sNumL);
    QLabel* pLabel = new QLabel("Probability:");
    QLabel* pNumL = new QLabel(QString::number(prob),this);
    QHBoxLayout* pHLayout = new QHBoxLayout();
    pHLayout->addWidget(pLabel);
    pHLayout->addWidget(pNumL);
    mainLayout->addWidget(imgLabel);
    fVLayout->addLayout(cHLayout);
    fVLayout->addLayout(sHLayout);
    fVLayout->addLayout(pHLayout);
    mainLayout->addWidget(groupBox1);
    //cVlayout->addLayout(csHLayout);
   // QGroupBox* groupBox2 = new QGroupBox("No need of modification");
  //  QVBoxLayout* yesVLayout = new QVBoxLayout(groupBox2);
  //  QCheckBox* yesBox = new QCheckBox("Frame classified right",this);
  //  connect(yesBox,SIGNAL(toggled(bool)),this,SLOT(setButtonText(bool)));
  //  yesVLayout->addWidget(yesBox);
  //  QLabel* yesNote = new QLabel("*If you think this frame is right, please tog the check box.");
  //  yesVLayout->addWidget(yesNote);
   // cVlayout->addLayout(cpHlayout);
  //  QGroupBox* groupBox3 = new QGroupBox("Modification");
  //  QVBoxLayout* cVlayout = new QVBoxLayout(groupBox2);
    QHBoxLayout* csHLayout = new QHBoxLayout(_groupBox3);
    QLabel* csLabel = new QLabel("Correct Label:");
  //  QLineEdit* csEdit = new QLineEdit(this);
    csHLayout->addWidget(csLabel);
    csHLayout->addWidget(_csEdit);
    QGroupBox* groupBox2 = new QGroupBox("Remove False Positive");
    QHBoxLayout* eraseLayout= new QHBoxLayout(groupBox2);
    QCheckBox* removeCheck = new QCheckBox("Remove this bounding box",this);
    eraseLayout->addWidget(removeCheck);
    connect(removeCheck,SIGNAL(toggled(bool)),this,SLOT(setButtonText(bool)));
    QGroupBox* groupBox4 = new QGroupBox("");
    QHBoxLayout* buttonVLayout = new QHBoxLayout(groupBox4);
    buttonVLayout->addWidget(_sButton);
    connect(_sButton,SIGNAL(clicked()),this,SLOT(checkInput()));
    buttonVLayout->addWidget(_doneButton);
    _doneButton->setVisible(false);
    connect(_doneButton,SIGNAL(clicked()),this,SLOT(close()));
    buttonVLayout->addWidget(_cButton);
    connect(_cButton,SIGNAL(clicked()),this,SLOT(close()));
    mainLayout->addWidget(groupBox2);
    mainLayout->addWidget(_groupBox3);
    mainLayout->addWidget(groupBox4);
    this->setWindowTitle("Correction Mode");
    this->setLayout(mainLayout);
}

void RectifyWidget::setButtonText(bool i)
{
    _sButton->setVisible(!i);
    _doneButton->setVisible(i);
    _cButton->setVisible(!i);
  //  _groupBox3->setVisible(!i);
}

void RectifyWidget::checkInput()
{
    QString str = _csEdit->text();
    QRegExp r("\\D");
    if( str.length() > 1 || r.exactMatch(str) || str.toInt() < 1 || str.toInt() > 5 )
    {
        QMessageBox msgBox;
        msgBox.setText("Wrong Input. Please input numbers from 1 to 5");
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if( ret )
            _csEdit->clear();
        return;

    }
    emit seqModified(str.toInt());
    close();
}

QPushButton* RectifyWidget::getSbutton()
{
    return _sButton;
}

QPushButton* RectifyWidget::getDoneButton()
{
    return _doneButton;
}
