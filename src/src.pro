
CONFIG += qt
QT += xml

TARGET = Top
VERSION = 2.0
DESTDIR = $$(HOME)/bin/.

INCLUDEPATH = . ../base ../base-qt ../server
DEPENDPATH += . ../base ../base-qt ../server

LIBS += \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a \
  ../server/libserver.a

POST_TARGETDEPS = \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a \
  ../server/libserver.a


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
copy_logbook.cc \
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
synchronize_logbook.cc \
ViewHtmlEntryDialog.cc \
ViewHtmlLogbookDialog.cc
