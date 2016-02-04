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
  app.cpp \
  about_box.cpp \
  chat_window.cpp \
  chat_incoming_message.cpp \
  chat_outgoing_message.cpp \
  subnet_set.cpp

HEADERS += \
  main_window.h \
  app.h \
  about_box.h \
  chat_window.h \
  chat_incoming_message.h \
  chat_outgoing_message.h \
  subnet_set.h

FORMS += \
  main_window.ui \
  about_box.ui \
  chat_window.ui \
  chat_incoming_message.ui \
  chat_outgoing_message.ui

RESOURCES += \
  resources.qrc

RC_FILE += \
  resources.rc

unix: {
  QMAKE_CXXFLAGS *= -std=c++11
}

DEFINES += GIT_DESCRIBE=\\\"$$system(git describe --long --always)\\\"

include(../Gqx/Gqx/GJson.pri)
include(../Gqx/Gqx/GSelfOwnedThread.pri)
include(../Gqx/Gqx/GUiHelpers.pri)
