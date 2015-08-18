#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QWidget>

class DisplayWidget : public QWidget
{
    Q_OBJECT
public:
    DisplayWidget(QImage img,QWidget *parent = 0);
    DisplayWidget(QWidget *parent = 0);
    QImage getLight();
    QPoint getStartPt();
    QPoint getEndpt();
    void clearPt();

signals:
    void lightRegionSelected();
    void pointSelected();
    void newBBAdded();

public slots:
    void setRectifyMode(bool i);
    void setTrainingMode(bool i);
	void setImage(QImage &image);

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);

private:
    QImage _img;
    void paintEvent(QPaintEvent *);

    void drawRect(QPainter& p);
    void rescale(double &x,double &y);
    void rescale(QPoint& p);

    QImage _lightingSubRegion;
    QPoint _startingPt;
    QPoint _endPt;
    bool _stored;
    bool _selected;
    bool _rectifyMode;
    bool _mouseDraged;
    bool _trainingMode;
};

#endif // DISPLAYWIDGET_H
