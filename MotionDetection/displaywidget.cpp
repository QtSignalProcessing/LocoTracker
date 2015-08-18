#include "displaywidget.h"

#include <QImage>
#include <QPainter>
#include <QMouseEvent>

#include <cmath>

#include <QDebug>


DisplayWidget::DisplayWidget(QImage img,QWidget *parent) :
    QWidget(parent),_img(img),_lightingSubRegion(),_stored(false),_selected(false),_rectifyMode(false),_mouseDraged(false),_trainingMode(false)
{
    //setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(img.width(),img.height());
    update();
}

DisplayWidget::DisplayWidget(QWidget *parent):
    QWidget(parent),_img(),_lightingSubRegion(),_stored(false),_selected(false),_rectifyMode(false),_mouseDraged(false),_trainingMode(false)
{//setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void DisplayWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawImage(this->rect(), _img);
    if( !_rectifyMode && !_trainingMode )
    {
        if(!(_startingPt==_endPt))
        {
            if(!_stored)
            {
                 drawRect(p);
            }
            if(_stored)
            {
                if(_lightingSubRegion.size().width() >0)
                _lightingSubRegion = _img.copy(_startingPt.x(),_startingPt.y(),fabs(float(_endPt.x()-_startingPt.x())),fabs((float)(_endPt.y()-_startingPt.y())));
                emit lightRegionSelected();
            }
        }
    }
    else
    {
        if(!(_startingPt==_endPt))
		{
            drawRect(p);
		}
    }
}

void DisplayWidget::setImage(QImage& img)
{
    _img = img;
    update();
	//repaint();
}

void DisplayWidget::mousePressEvent(QMouseEvent *event)
{
    if(!_rectifyMode && !_trainingMode)
    {
        if(_selected)
        {
            _startingPt = _endPt;
        }
        if (event->button() == Qt::LeftButton && !_selected)
        {
             _startingPt = event->pos();
             _stored = false;
        }
        else if(event->button() == Qt::RightButton && (_startingPt!=_endPt))
        {
            if(_img.size() != this->size())
            {
                rescale(_startingPt);
                rescale(_endPt);
            }
            int x,y,width,height;
            x = _startingPt.x() < _endPt.x() ? _startingPt.x():_endPt.x();
            y = _startingPt.y() < _endPt.y() ? _startingPt.y():_endPt.y();
            width = abs(_startingPt.x() - _endPt.x());
            height = abs(_startingPt.y() - _endPt.y());
            _startingPt.setX(x);
            _startingPt.setY(y);
            _endPt.setX(x+width);
            _endPt.setY(y+height);
            _lightingSubRegion = _img.copy(_startingPt.x(),_startingPt.y(),abs(_endPt.x()-_startingPt.x()),abs(_endPt.y()-_startingPt.y()));
            _stored = true;
            update();
            emit lightRegionSelected();
        }
        if(event->type() == QEvent::MouseButtonDblClick)
        {
             _stored = false;
             _startingPt = _endPt;
             update();
        }
    }
    else
    {
        if(event->button() == Qt::RightButton&&!_mouseDraged)
        {
            _startingPt = event->pos();
            _endPt = event->pos();
            rescale(_startingPt);
            rescale(_endPt);
            emit pointSelected();
        }
        else if( event->button() == Qt::LeftButton )
        {
            _startingPt = event->pos();
        }
        else if( event->button() == Qt::RightButton&&_mouseDraged && _rectifyMode )
        {
            rescale(_startingPt);
            rescale(_endPt);
            emit newBBAdded();
        }
        _mouseDraged = false;
    }
}

void DisplayWidget::mouseMoveEvent(QMouseEvent *event)
{
    //if( !_rectifyMode )
    {
        if ((event->buttons() & Qt::LeftButton))
        {
            _endPt = event->pos();
			_mouseDraged = true;
            update();
        }
    }
}

void DisplayWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //if(!_rectifyMode)
    {
        if ((event->buttons() & Qt::LeftButton))
        {
            _endPt = event->pos();
            update();
        }
    }
}

void DisplayWidget::resizeEvent(QResizeEvent *)
{
     _img.scaled(this->width(),this->height());
}

void DisplayWidget::drawRect(QPainter& p)
{
    p.setPen(QPen(Qt::black,0,Qt::DashLine));
    p.drawRect(QRect(_startingPt,_endPt));
}

QImage DisplayWidget::getLight()
{
    if(_img.size()!=this->size())
    {
        double xratio = (double)this->width()/_img.width();
        double yratio = (double)this->height()/_img.height();
       _lightingSubRegion =  _lightingSubRegion.scaled(abs(_endPt.x()-_startingPt.x())*xratio,abs(_endPt.y()-_startingPt.y())*yratio);
    }
   return _lightingSubRegion;
}

void DisplayWidget::rescale(double &x, double &y)
{
    double xratio = (double)_img.width() / this->width();
    x *= xratio;
    double yratio = (double)_img.height() / this->height();
    y *= yratio;
}

void DisplayWidget::rescale(QPoint &p)
{
    double xratio = (double)_img.width() / this->width();
    p.setX(p.x()*xratio);
    double yratio = (double)_img.height() / this->height();
    p.setY(p.y()*yratio);
}

QPoint DisplayWidget::getStartPt()
{
    return _startingPt;
}

QPoint DisplayWidget::getEndpt()
{
    return _endPt;
}

void DisplayWidget::clearPt()
{
    _startingPt.setX(0);
    _startingPt.setY(0);
    _endPt.setX(0);
    _endPt.setY(0);
}

void DisplayWidget::setRectifyMode(bool i)
{
    _rectifyMode = i;
	_trainingMode = !i;

}

void DisplayWidget::setTrainingMode(bool i)
{
    _trainingMode = i;
	_rectifyMode = !i;
}
