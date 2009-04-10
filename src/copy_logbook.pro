TEMPLATE = app
TARGET = copy_logbook

CONFIG += qt release
QT += xml network

win32 {
  DEFINES += QT_STATIC
  DESTDIR = "C:/Program Files" 
}

VERSION = 1.6.2
DEFINES += VERSION=\\\"$$VERSION\\\"

INCLUDEPATH = . ../base ../base-qt ../base-help ../base-server ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-help ../base-server ../extra-includes

LIBS += \
  ./libelogbook-common.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

POST_TARGETDEPS = \
  ./libelogbook-common.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

RESOURCES = pixmaps.qrc ../base-qt/basePixmaps.qrc
RC_FILE = elogbook.rc

SOURCES = copy_logbook.cpp
