TEMPLATE = app
TARGET = elogbook

CONFIG += qt release
QT += xml network

win32 {
  DEFINES += QT_STATIC
  DESTDIR = "C:/Program Files" 
}

VERSION = 1.6.0
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

HEADERS = \
  Application.h \
  AskForSaveDialog.h \
  AttachmentFrame.h \
  AttachmentModel.h \
  AttachmentWindow.h \
  ConfigurationDialog.h \
  DeleteAttachmentDialog.h \
  DeleteKeywordDialog.h \
  EditAttachmentDialog.h \
  EditionWindow.h \
  EditKeywordDialog.h \
  FileCheckDialog.h \
  FormatBar.h \
  KeywordModel.h \
  LogbookInformationDialog.h \
  LogbookModifiedDialog.h \
  LogbookStatisticsDialog.h \
  LogEntryInformationDialog.h \
  LogEntryModel.h \
  MainWindow.h \
  Menu.h \
  NewAttachmentDialog.h \
  NewLogbookDialog.h \
  OpenAttachmentDialog.h \
  PrintDialog.h \
  PrintLogEntryDialog.h \
  PrintLogbookDialog.h \
  ProgressBar.h \
  SearchPanel.h \
  SelectionStatusBar.h \
  elogbook.h

SOURCES = \
  Application.cpp \
  AskForSaveDialog.cpp \
  AttachmentFrame.cpp \
  AttachmentModel.cpp \
  AttachmentWindow.cpp \
  ConfigurationDialog.cpp \
  DeleteAttachmentDialog.cpp \
  DeleteKeywordDialog.cpp \
  EditAttachmentDialog.cpp \
  EditionWindow.cpp \
  EditKeywordDialog.cpp \
  FileCheckDialog.cpp \
  FormatBar.cpp \
  KeywordModel.cpp \
  LogbookInformationDialog.cpp \
  LogbookModifiedDialog.cpp \
  LogbookStatisticsDialog.cpp \
  LogEntryInformationDialog.cpp \
  LogEntryModel.cpp \
  MainWindow.cpp \
  Menu.cpp \
  NewAttachmentDialog.cpp \
  NewLogbookDialog.cpp \
  OpenAttachmentDialog.cpp \
  PrintDialog.cpp \
  PrintLogEntryDialog.cpp \
  PrintLogbookDialog.cpp \
  ProgressBar.cpp \
  SearchPanel.cpp \
  SelectionStatusBar.cpp \
  elogbook.cpp
