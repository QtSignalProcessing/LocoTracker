/****************************************************************************
** Meta object code from reading C++ file 'trainingwidget.h'
**
** Created: Fri Jun 5 11:42:09 2015
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../trainingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'trainingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_TrainingWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,
      29,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      44,   15,   15,   15, 0x0a,
      55,   15,   15,   15, 0x0a,
      63,   15,   15,   15, 0x0a,
      72,   15,   15,   15, 0x0a,
      80,   15,   15,   15, 0x0a,
      95,   15,   15,   15, 0x0a,
     106,   15,   15,   15, 0x0a,
     120,   15,   15,   15, 0x0a,
     130,   15,   15,   15, 0x0a,
     148,  144,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_TrainingWidget[] = {
    "TrainingWidget\0\0closed(bool)\0"
    "traingFinish()\0training()\0clear()\0"
    "finish()\0store()\0clearCurrent()\0"
    "clearAll()\0loadCurrent()\0loadAll()\0"
    "updatLabels()\0num\0frameNumChanged(int)\0"
};

void TrainingWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        TrainingWidget *_t = static_cast<TrainingWidget *>(_o);
        switch (_id) {
        case 0: _t->closed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->traingFinish(); break;
        case 2: _t->training(); break;
        case 3: _t->clear(); break;
        case 4: _t->finish(); break;
        case 5: _t->store(); break;
        case 6: _t->clearCurrent(); break;
        case 7: _t->clearAll(); break;
        case 8: _t->loadCurrent(); break;
        case 9: _t->loadAll(); break;
        case 10: _t->updatLabels(); break;
        case 11: _t->frameNumChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData TrainingWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TrainingWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_TrainingWidget,
      qt_meta_data_TrainingWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TrainingWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TrainingWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TrainingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TrainingWidget))
        return static_cast<void*>(const_cast< TrainingWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int TrainingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void TrainingWidget::closed(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TrainingWidget::traingFinish()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
