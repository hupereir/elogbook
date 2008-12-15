TEMPLATE = app

CONFIG += qt release
QT += xml network svg

win32 {
  DESTDIR = "C:/Program Files" 
}

VERSION = 1.1
DEFINES += QT_STATIC VERSION=\"$$VERSION\"

INCLUDEPATH = . ../base ../base-qt ../base-help ../base-server ../base-svg ../base-transparency ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-help ../base-server ../base-svg ../base-transparency../extra-includes

LIBS += \
  ./libelogbook-common.a \
  ../base-transparency/libbase-transparency.a \
  ../base-svg/libbase-svg.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

POST_TARGETDEPS = \
  ./libelogbook-common.a \
  ../base-transparency/libbase-transparency.a \
  ../base-svg/libbase-svg.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

RESOURCES = pixmaps.qrc ../base-svg/baseSvg.qrc ../base-qt/basePixmaps.qrc
RC_FILE = elogbook.rc

SOURCES = synchronize_logbook.cpp
