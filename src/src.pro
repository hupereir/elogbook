TEMPLATE = app

CONFIG += qt release
QT += xml network svg

TARGET = elogbook

win32 {
  DESTDIR = "C:/Program Files" 
}

VERSION = 1.1
DEFINES += QT_STATIC VERSION=\"$$VERSION\"

INCLUDEPATH = . ../base ../base-qt ../base-help ../base-server ../base-svg ../base-transparency ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-help ../base-server ../base-svg ../base-transparency../extra-includes

LIBS += \
  ../base-transparency/libbase-transparency.a \
  ../base-svg/libbase-svg.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

POST_TARGETDEPS = \
  ../base-transparency/libbase-transparency.a \
  ../base-svg/libbase-svg.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

RESOURCES = pixmaps.qrc ../base-svg/baseSvg.qrc ../base-qt/basePixmaps.qrc
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
  HtmlHeaderNode.h \
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
  HtmlHeaderNode.cpp \
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
