QT += widgets opengl openglwidgets testlib
TARGET = TriangleSphere
CONFIG += c++17

SOURCES += main.cpp \
           complexplaneview.cpp \
           dragpoint.cpp \
           functioninputdialog.cpp \
           trianglescene.cpp \
           mainwindow.cpp \
           coordtransform.cpp \
           spherewidget.cpp \
           complexplaneview2.cpp

HEADERS += dragpoint.h \
           complexplaneview.h \
           functioninputdialog.h \
           trianglescene.h \
           mainwindow.h \
           coordtransform.h \
           spherewidget.h \
           complexplaneview2.h
