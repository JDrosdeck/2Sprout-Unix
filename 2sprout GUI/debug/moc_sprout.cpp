/****************************************************************************
** Meta object code from reading C++ file 'sprout.h'
**
** Created: Wed May 27 20:55:29 2009
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../sprout.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sprout.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_sprout[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x08,
      34,    7,    7,    7, 0x08,
      60,    7,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_sprout[] = {
    "sprout\0\0on_pushButton_4_clicked()\0"
    "on_pushButton_6_clicked()\0"
    "on_pushButton_3_clicked()\0"
};

const QMetaObject sprout::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_sprout,
      qt_meta_data_sprout, 0 }
};

const QMetaObject *sprout::metaObject() const
{
    return &staticMetaObject;
}

void *sprout::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_sprout))
        return static_cast<void*>(const_cast< sprout*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int sprout::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_pushButton_4_clicked(); break;
        case 1: on_pushButton_6_clicked(); break;
        case 2: on_pushButton_3_clicked(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
