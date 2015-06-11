/****************************************************************************
** Meta object code from reading C++ file 'rectifywidget.h'
**
** Created: Fri Jun 5 11:42:08 2015
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../rectifywidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rectifywidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RectifyWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      19,   15,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      38,   36,   14,   14, 0x0a,
      58,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RectifyWidget[] = {
    "RectifyWidget\0\0seq\0seqModified(int)\0"
    "i\0setButtonText(bool)\0checkInput()\0"
};

void RectifyWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RectifyWidget *_t = static_cast<RectifyWidget *>(_o);
        switch (_id) {
        case 0: _t->seqModified((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->setButtonText((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->checkInput(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RectifyWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RectifyWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_RectifyWidget,
      qt_meta_data_RectifyWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RectifyWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RectifyWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RectifyWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RectifyWidget))
        return static_cast<void*>(const_cast< RectifyWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int RectifyWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void RectifyWidget::seqModified(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
