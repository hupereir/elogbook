
CONFIG += qt
QT += xml network
RESOURCES = ../pixmaps/pixmaps.qrc

TARGET = elogbook
VERSION = Qt4_1.0

INCLUDEPATH = . ../base ../base-qt ../base-server
DEPENDPATH += . ../base ../base-qt ../base-server

LIBS += \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

POST_TARGETDEPS = \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a


HEADERS = \
AskForSaveDialog.h \
AttachmentFrame.h \
Attachment.h \
AttachmentList.h \
AttachmentType.h \
ConfigurationDialog.h \
DeleteAttachmentDialog.h \
DeleteKeywordDialog.h \
EditAttachmentDialog.h \
EditFrame.h \
EditKeywordDialog.h \
FormatBar.h \
HelpText.h \
HtmlUtil.h \
Icons.h \
KeywordList.h \
Logbook.h \
LogbookInfoDialog.h \
LogbookModifiedDialog.h \
LogbookStatisticsDialog.h \
LogEntry.h \
LogEntryInfoDialog.h \
LogEntryList.h \
MainFrame.h \
Menu.h \
NewAttachmentDialog.h \
NewLogbookDialog.h \
OpenAttachmentDialog.h \
SearchPanel.h \
SelectionFrame.h \
SplashScreen.h \
ViewHtmlEntryDialog.h \
ViewHtmlLogbookDialog.h \
XmlDef.h

SOURCES = \
AskForSaveDialog.cc \
Attachment.cc \
AttachmentFrame.cc \
AttachmentList.cc \
AttachmentType.cc \
ConfigurationDialog.cc \
DeleteAttachmentDialog.cc \
DeleteKeywordDialog.cc \
EditAttachmentDialog.cc \
EditFrame.cc \
EditKeywordDialog.cc \
eLogbook.cc \
FormatBar.cc \
HtmlUtil.cc \
KeywordList.cc \
Logbook.cc \
LogbookInfoDialog.cc \
LogbookModifiedDialog.cc \
LogbookStatisticsDialog.cc \
LogEntry.cc \
LogEntryInfoDialog.cc \
LogEntryList.cc \
MainFrame.cc \
Menu.cc \
NewAttachmentDialog.cc \
NewLogbookDialog.cc \
OpenAttachmentDialog.cc \
SearchPanel.cc \
SelectionFrame.cc \
SplashScreen.cc \
ViewHtmlEntryDialog.cc \
ViewHtmlLogbookDialog.cc
