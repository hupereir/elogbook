TEMPLATE = app

CONFIG += qt release
QT += xml network
TARGET = elogbook

VERSION = 1.1
DEFINES += QT_STATIC VERSION=\"$$VERSION\"

INCLUDEPATH = . ../base ../base-qt ../base-help ../base-server ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-help ../base-server ../extra-includes

LIBS += \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base-help/libbase-help.a \
  ../base/libbase.a

POST_TARGETDEPS = \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base/libbase.a

RESOURCES = pixmaps.qrc ../base-qt/basePixmaps.qrc
RC_FILE = elogbook.rc

HEADERS = \
  AskForSaveDialog.h \
  Attachment.h \
  AttachmentFrame.h \
  AttachmentModel.h \
  AttachmentType.h \
  AttachmentWindow.h \
  ConfigurationDialog.h \
  DefaultOptions.h \
  DeleteAttachmentDialog.h \
  DeleteKeywordDialog.h \
  EditAttachmentDialog.h \
  EditionWindow.h \
  EditKeywordDialog.h \
  FormatBar.h \
  HelpText.h \
  HtmlUtil.h \
  Icons.h \
  Keyword.h \
  KeywordModel.h \
  Logbook.h \
  LogbookInformationDialog.h \
  LogbookModifiedDialog.h \
  LogbookStatisticsDialog.h \
  LogEntry.h \
  LogEntryInformationDialog.h \
  LogEntryModel.h \
  Application.h \
  Menu.h \
  NewAttachmentDialog.h \
  NewLogbookDialog.h \
  OpenAttachmentDialog.h \
  ProgressBar.h \
  SearchPanel.h \
  MainWindow.h \
  SelectionStatusBar.h \
  SplashScreen.h \
  ViewHtmlEntryDialog.h \
  ViewHtmlLogbookDialog.h \
  XmlDef.h

SOURCES = \
  AskForSaveDialog.cpp \
  Attachment.cpp \
  AttachmentFrame.cpp \
  AttachmentModel.cpp \
  AttachmentType.cpp \
  AttachmentWindow.cpp \
  ConfigurationDialog.cpp \
  DeleteAttachmentDialog.cpp \
  DeleteKeywordDialog.cpp \
  EditAttachmentDialog.cpp \
  EditionWindow.cpp \
  EditKeywordDialog.cpp \
  elogbook.cpp \
  FormatBar.cpp \
  HtmlUtil.cpp \
  Keyword.cpp \
  KeywordModel.cpp \
  Logbook.cpp \
  LogbookInformationDialog.cpp \
  LogbookModifiedDialog.cpp \
  LogbookStatisticsDialog.cpp \
  LogEntry.cpp \
  LogEntryInformationDialog.cpp \
  LogEntryModel.cpp \
  Application.cpp \
  Menu.cpp \
  NewAttachmentDialog.cpp \
  NewLogbookDialog.cpp \
  OpenAttachmentDialog.cpp \
  ProgressBar.cpp \
  SearchPanel.cpp \
  MainWindow.cpp \
  SelectionStatusBar.cpp \
  SplashScreen.cpp \
  ViewHtmlEntryDialog.cpp \
  ViewHtmlLogbookDialog.cpp
