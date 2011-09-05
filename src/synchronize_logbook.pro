TEMPLATE = app
TARGET = synchronize_logbook

CONFIG += qt release
QT += xml network

VERSION = 1.6.11
DEFINES += VERSION=\\\"$$VERSION\\\"

win32 {

  DEFINES += QT_STATIC
  DESTDIR = "C:\Program Files"

  # this is needed to copy target into relevant versioned name
  exists( \"$$DESTDIR\\upx.exe\" ) {

     # if available, use upx to compress the file
     version.commands = "\"$$DESTDIR\\upx.exe\" -9 -f -o \"$$DESTDIR\\$$TARGET-qt4_"$$VERSION".exe\""  "\"$$DESTDIR\\"$$TARGET".exe\"

  } else {

     # simple copy
     version.commands = @copy "\"$$DESTDIR\\"$$TARGET".exe\" \"$$DESTDIR\\$$TARGET-qt4_"$$VERSION".exe\""

  }

  # add to Post targets
  QMAKE_EXTRA_TARGETS += version
  QMAKE_POST_LINK += $$version.commands

}

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

SOURCES = synchronize_logbook.cpp
