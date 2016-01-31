#-------------------------------------------------
#
# Project created by QtCreator 2016-01-28T19:03:50
#
#-------------------------------------------------

QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lanchat
TEMPLATE = app

SOURCES += \
  main.cpp\
  main_window.cpp \
  app.cpp

HEADERS += \
  main_window.h \
  app.h

FORMS += \
  main_window.ui

RESOURCES += \
  resources.qrc

RC_FILE += \
  resources.rc

unix: {
  QMAKE_CXXFLAGS *= -std=c++11
}

DEFINES += GIT_DESCRIBE=\\\"$$system(git describe --long --always)\\\"
