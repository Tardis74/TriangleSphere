/****************************************************************************
** Meta object code from reading C++ file 'trianglescene.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../trianglescene.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'trianglescene.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_TriangleScene_t {
    uint offsetsAndSizes[20];
    char stringdata0[14];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[17];
    char stringdata5[18];
    char stringdata6[22];
    char stringdata7[14];
    char stringdata8[14];
    char stringdata9[7];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_TriangleScene_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_TriangleScene_t qt_meta_stringdata_TriangleScene = {
    {
        QT_MOC_LITERAL(0, 13),  // "TriangleScene"
        QT_MOC_LITERAL(14, 15),  // "triangleUpdated"
        QT_MOC_LITERAL(30, 0),  // ""
        QT_MOC_LITERAL(31, 12),  // "dragFinished"
        QT_MOC_LITERAL(44, 16),  // "sceneDragStarted"
        QT_MOC_LITERAL(61, 17),  // "sceneDragFinished"
        QT_MOC_LITERAL(79, 21),  // "pointPositionChanging"
        QT_MOC_LITERAL(101, 13),  // "massesChanged"
        QT_MOC_LITERAL(115, 13),  // "QList<double>"
        QT_MOC_LITERAL(129, 6)   // "masses"
    },
    "TriangleScene",
    "triangleUpdated",
    "",
    "dragFinished",
    "sceneDragStarted",
    "sceneDragFinished",
    "pointPositionChanging",
    "massesChanged",
    "QList<double>",
    "masses"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_TriangleScene[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   50,    2, 0x06,    1 /* Public */,
       3,    0,   51,    2, 0x06,    2 /* Public */,
       4,    0,   52,    2, 0x06,    3 /* Public */,
       5,    0,   53,    2, 0x06,    4 /* Public */,
       6,    0,   54,    2, 0x06,    5 /* Public */,
       7,    1,   55,    2, 0x06,    6 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject TriangleScene::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsScene::staticMetaObject>(),
    qt_meta_stringdata_TriangleScene.offsetsAndSizes,
    qt_meta_data_TriangleScene,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_TriangleScene_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TriangleScene, std::true_type>,
        // method 'triangleUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'dragFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sceneDragStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sceneDragFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'pointPositionChanging'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'massesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<double> &, std::false_type>
    >,
    nullptr
} };

void TriangleScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TriangleScene *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->triangleUpdated(); break;
        case 1: _t->dragFinished(); break;
        case 2: _t->sceneDragStarted(); break;
        case 3: _t->sceneDragFinished(); break;
        case 4: _t->pointPositionChanging(); break;
        case 5: _t->massesChanged((*reinterpret_cast< std::add_pointer_t<QList<double>>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<double> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TriangleScene::*)();
            if (_t _q_method = &TriangleScene::triangleUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TriangleScene::*)();
            if (_t _q_method = &TriangleScene::dragFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TriangleScene::*)();
            if (_t _q_method = &TriangleScene::sceneDragStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TriangleScene::*)();
            if (_t _q_method = &TriangleScene::sceneDragFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TriangleScene::*)();
            if (_t _q_method = &TriangleScene::pointPositionChanging; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (TriangleScene::*)(const QList<double> & );
            if (_t _q_method = &TriangleScene::massesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *TriangleScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TriangleScene::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TriangleScene.stringdata0))
        return static_cast<void*>(this);
    return QGraphicsScene::qt_metacast(_clname);
}

int TriangleScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void TriangleScene::triangleUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TriangleScene::dragFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TriangleScene::sceneDragStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void TriangleScene::sceneDragFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void TriangleScene::pointPositionChanging()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void TriangleScene::massesChanged(const QList<double> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
