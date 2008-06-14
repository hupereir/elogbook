TEMPLATE = app

CONFIG += qt release
QT += xml network
TARGET = elogbook

VERSION = 1.1
DEFINES += QT_STATIC VERSION="\"$$VERSION\""

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
 Keyword.h \
 KeywordDelegate.h \
 KeywordModel.h \
 Logbook.h \
 LogbookInformationDialog.h \
 LogbookModifiedDialog.h \
 LogbookStatisticsDialog.h \
 LogEntry.h \
 LogEntryDelegate.h \
 LogEntryInformationDialog.h \
 LogEntryModel.h \
 MainFrame.h \
 Menu.h \
 NewAttachmentDialog.h \
 NewLogbookDialog.h \
 OpenAttachmentDialog.h \
 ProgressFrame.h \
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
 elogbook.cpp \
 FormatBar.cpp \
 HtmlUtil.cpp \
 Keyword.cpp \
 KeywordDelegate.cpp \
 KeywordModel.cpp \
 Logbook.cpp \
 LogbookInformationDialog.cpp \
 LogbookModifiedDialog.cpp \
 LogbookStatisticsDialog.cpp \
 LogEntry.cpp \
 LogEntryDelegate.cpp \
 LogEntryInformationDialog.cpp \
 LogEntryModel.cpp \
 MainFrame.cpp \
 Menu.cpp \
 NewAttachmentDialog.cpp \
 NewLogbookDialog.cpp \
 OpenAttachmentDialog.cpp \
 ProgressFrame.cpp \
 SearchPanel.cpp \
 SelectionFrame.cpp \
 SplashScreen.cpp \
 ViewHtmlEntryDialog.cpp \
 ViewHtmlLogbookDialog.cpp
