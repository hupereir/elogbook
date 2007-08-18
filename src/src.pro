TEMPLATE = app

CONFIG += qt release static staticlib
QT += xml network
TARGET = elogbook

VERSION = 1.0
DEFINES += QT_STATIC VERSION="\"$$VERSION\""

INCLUDEPATH = . ../base ../base-qt ../base-server ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-server ../extra-includes

LIBS += \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

POST_TARGETDEPS = \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

RESOURCES = pixmaps.qrc

HEADERS = \
 AskForSaveDialog.h \
 AttachmentFrame.h \
 Attachment.h \
 AttachmentList.h \
 AttachmentType.h \
 ConfigurationDialog.h \
 DefaultOptions.h \
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
 AskForSaveDialog.cpp \
 Attachment.cpp \
 AttachmentFrame.cpp \
 AttachmentList.cpp \
 AttachmentType.cpp \
 ConfigurationDialog.cpp \
 DeleteAttachmentDialog.cpp \
 DeleteKeywordDialog.cpp \
 EditAttachmentDialog.cpp \
 EditFrame.cpp \
 EditKeywordDialog.cpp \
 eLogbook.cpp \
 FormatBar.cpp \
 HtmlUtil.cpp \
 KeywordList.cpp \
 Logbook.cpp \
 LogbookInfoDialog.cpp \
 LogbookModifiedDialog.cpp \
 LogbookStatisticsDialog.cpp \
 LogEntry.cpp \
 LogEntryInfoDialog.cpp \
 LogEntryList.cpp \
 MainFrame.cpp \
 Menu.cpp \
 NewAttachmentDialog.cpp \
 NewLogbookDialog.cpp \
 OpenAttachmentDialog.cpp \
 SearchPanel.cpp \
 SelectionFrame.cpp \
 SplashScreen.cpp \
 ViewHtmlEntryDialog.cpp \
 ViewHtmlLogbookDialog.cpp
