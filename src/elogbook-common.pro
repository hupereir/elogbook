TEMPLATE = lib
CONFIG = staticlib qt

CONFIG += qt
QT += xml network

INCLUDEPATH += ../base ../base-qt ../base-server ../base-help ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-server ../base-help ../extra-includes

HEADERS = \
  Attachment.h \
  AttachmentType.h \
  HtmlHeaderNode.h \
  Keyword.h \
  Logbook.h \
  LogEntry.h

SOURCES = \
  Attachment.cpp \
  AttachmentType.cpp \
  HtmlHeaderNode.cpp \
  Keyword.cpp \
  Logbook.cpp \
  LogEntry.cpp
