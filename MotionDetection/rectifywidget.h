#ifndef RECTIFYWIDGET_H
#define RECTIFYWIDGET_H

#include <QWidget>

class QPushButton;
class QLineEdit;
class QGroupBox;

class RectifyWidget : public QWidget
{
    Q_OBJECT
public:
    RectifyWidget(QImage& img,int cNum,int seq,float prob,QWidget *parent = 0);
    QPushButton* getSbutton();
    QPushButton* getDoneButton();
    
signals:
    void seqModified(int seq);
    
public slots:
    void setButtonText(bool i);
    void checkInput();

private:
    QPushButton* _sButton;
    QPushButton* _cButton;
    QPushButton* _doneButton;
    QLineEdit* _csEdit;
    QGroupBox* _groupBox3;
};

#endif // RECTIFYWIDGET_H
