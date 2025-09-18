QT += widgets opengl openglwidgets testlib
TARGET = TriangleSphere
CONFIG += c++17

SOURCES += main.cpp \
           dragpoint.cpp \
           trianglescene.cpp \
           mainwindow.cpp \
           coordtransform.cpp \
           spherewidget.cpp

HEADERS += dragpoint.h \
           trianglescene.h \
           mainwindow.h \
           coordtransform.h \
           spherewidget.h

