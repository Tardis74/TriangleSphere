QT += widgets opengl openglwidgets testlib
TARGET = TriangleSphere
CONFIG += c++17

SOURCES += main.cpp \
           dragpoint.cpp \
           functioninputdialog.cpp \
           trianglescene.cpp \
           mainwindow.cpp \
           coordtransform.cpp \
           spherewidget.cpp

HEADERS += dragpoint.h \
           functioninputdialog.h \
           trianglescene.h \
           mainwindow.h \
           coordtransform.h \
           spherewidget.h
