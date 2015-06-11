/****************************************************************************
** Meta object code from reading C++ file 'videoprocessing.h'
**
** Created: Fri Jun 5 11:42:09 2015
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../videoprocessing.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'videoprocessing.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_VideoProcessing[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   17,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      46,   16,   16,   16, 0x0a,
      53,   16,   16,   16, 0x0a,
      61,   16,   16,   16, 0x0a,
      68,   16,   16,   16, 0x0a,
      89,   80,   16,   16, 0x0a,
     116,  110,   16,   16, 0x0a,
     136,   16,   16,   16, 0x0a,
     155,   16,   16,   16, 0x0a,
     172,   16,   16,   16, 0x0a,
     184,   16,   16,   16, 0x0a,
     195,   16,   16,   16, 0x0a,
     214,  212,   16,   16, 0x0a,
     232,   16,   16,   16, 0x0a,
     252,  248,   16,   16, 0x0a,
     270,   16,   16,   16, 0x0a,
     290,   16,   16,   16, 0x0a,
     303,   16,   16,   16, 0x0a,
     316,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_VideoProcessing[] = {
    "VideoProcessing\0\0finished\0videoFinished(bool)\0"
    "play()\0pause()\0stop()\0showLight()\0"
    "fileName\0setFileName(QString)\0frame\0"
    "setFramePosion(int)\0changeSelectMode()\0"
    "changeMode(bool)\0trainData()\0loadData()\0"
    "trainingFinish()\0i\0rectifyMode(bool)\0"
    "readyToModify()\0seq\0modification(int)\0"
    "currentFrameReady()\0removeBBox()\0"
    "addNewBBox()\0reay2Update()\0"
};

void VideoProcessing::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        VideoProcessing *_t = static_cast<VideoProcessing *>(_o);
        switch (_id) {
        case 0: _t->videoFinished((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->play(); break;
        case 2: _t->pause(); break;
        case 3: _t->stop(); break;
        case 4: _t->showLight(); break;
        case 5: _t->setFileName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->setFramePosion((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->changeSelectMode(); break;
        case 8: _t->changeMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->trainData(); break;
        case 10: _t->loadData(); break;
        case 11: _t->trainingFinish(); break;
        case 12: _t->rectifyMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 13: _t->readyToModify(); break;
        case 14: _t->modification((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->currentFrameReady(); break;
        case 16: _t->removeBBox(); break;
        case 17: _t->addNewBBox(); break;
        case 18: _t->reay2Update(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData VideoProcessing::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject VideoProcessing::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_VideoProcessing,
      qt_meta_data_VideoProcessing, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &VideoProcessing::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *VideoProcessing::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *VideoProcessing::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VideoProcessing))
        return static_cast<void*>(const_cast< VideoProcessing*>(this));
    return QWidget::qt_metacast(_clname);
}

int VideoProcessing::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void VideoProcessing::videoFinished(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
