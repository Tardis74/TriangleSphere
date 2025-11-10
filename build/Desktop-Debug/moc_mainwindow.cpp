/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_MainWindow_t {
    uint offsetsAndSizes[38];
    char stringdata0[11];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[14];
    char stringdata5[18];
    char stringdata6[25];
    char stringdata7[6];
    char stringdata8[15];
    char stringdata9[7];
    char stringdata10[8];
    char stringdata11[10];
    char stringdata12[23];
    char stringdata13[23];
    char stringdata14[17];
    char stringdata15[20];
    char stringdata16[6];
    char stringdata17[16];
    char stringdata18[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MainWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
        QT_MOC_LITERAL(0, 10),  // "MainWindow"
        QT_MOC_LITERAL(11, 15),  // "fixMaxTimeInput"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 13),  // "fixSpeedInput"
        QT_MOC_LITERAL(42, 13),  // "autoScaleView"
        QT_MOC_LITERAL(56, 17),  // "updateSpherePoint"
        QT_MOC_LITERAL(74, 24),  // "handleSpherePointClicked"
        QT_MOC_LITERAL(99, 5),  // "point"
        QT_MOC_LITERAL(105, 14),  // "onDragFinished"
        QT_MOC_LITERAL(120, 6),  // "zoomIn"
        QT_MOC_LITERAL(127, 7),  // "zoomOut"
        QT_MOC_LITERAL(135, 9),  // "setMasses"
        QT_MOC_LITERAL(145, 22),  // "updatePointCoordinates"
        QT_MOC_LITERAL(168, 22),  // "onAnimationModeClicked"
        QT_MOC_LITERAL(191, 16),  // "onAnimationReset"
        QT_MOC_LITERAL(208, 19),  // "onTimeSliderChanged"
        QT_MOC_LITERAL(228, 5),  // "value"
        QT_MOC_LITERAL(234, 15),  // "updateAnimation"
        QT_MOC_LITERAL(250, 15)   // "updateTimeLabel"
    },
    "MainWindow",
    "fixMaxTimeInput",
    "",
    "fixSpeedInput",
    "autoScaleView",
    "updateSpherePoint",
    "handleSpherePointClicked",
    "point",
    "onDragFinished",
    "zoomIn",
    "zoomOut",
    "setMasses",
    "updatePointCoordinates",
    "onAnimationModeClicked",
    "onAnimationReset",
    "onTimeSliderChanged",
    "value",
    "updateAnimation",
    "updateTimeLabel"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  104,    2, 0x08,    1 /* Private */,
       3,    0,  105,    2, 0x08,    2 /* Private */,
       4,    0,  106,    2, 0x08,    3 /* Private */,
       5,    0,  107,    2, 0x08,    4 /* Private */,
       6,    1,  108,    2, 0x08,    5 /* Private */,
       8,    0,  111,    2, 0x08,    7 /* Private */,
       9,    0,  112,    2, 0x08,    8 /* Private */,
      10,    0,  113,    2, 0x08,    9 /* Private */,
      11,    0,  114,    2, 0x08,   10 /* Private */,
      12,    0,  115,    2, 0x08,   11 /* Private */,
      13,    0,  116,    2, 0x08,   12 /* Private */,
      14,    0,  117,    2, 0x08,   13 /* Private */,
      15,    1,  118,    2, 0x08,   14 /* Private */,
      17,    0,  121,    2, 0x08,   16 /* Private */,
      18,    0,  122,    2, 0x08,   17 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QVector3D,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.offsetsAndSizes,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'fixMaxTimeInput'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fixSpeedInput'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'autoScaleView'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateSpherePoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSpherePointClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector3D &, std::false_type>,
        // method 'onDragFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomIn'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomOut'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setMasses'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updatePointCoordinates'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAnimationModeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAnimationReset'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTimeSliderChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'updateAnimation'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateTimeLabel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->fixMaxTimeInput(); break;
        case 1: _t->fixSpeedInput(); break;
        case 2: _t->autoScaleView(); break;
        case 3: _t->updateSpherePoint(); break;
        case 4: _t->handleSpherePointClicked((*reinterpret_cast< std::add_pointer_t<QVector3D>>(_a[1]))); break;
        case 5: _t->onDragFinished(); break;
        case 6: _t->zoomIn(); break;
        case 7: _t->zoomOut(); break;
        case 8: _t->setMasses(); break;
        case 9: _t->updatePointCoordinates(); break;
        case 10: _t->onAnimationModeClicked(); break;
        case 11: _t->onAnimationReset(); break;
        case 12: _t->onTimeSliderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->updateAnimation(); break;
        case 14: _t->updateTimeLabel(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
