# $Id$

TEMPLATE = app
TARGET = elogbook

CONFIG += qt release
QT += xml network

VERSION = 1.8.9
DEFINES += VERSION=\\\"$$VERSION\\\"

mac {
	DEFINES += QT_NO_DBUS
}

unix:!mac {
	QT += dbus
	LIBS += -lX11
}

win32 {

	DEFINES += QT_STATIC
	DESTDIR = "C:\Program Files"
	QMAKE_LFLAGS += -static-libgcc -static-libstdc++

	# this is needed to copy target into relevant versioned name
	exists( \"$$DESTDIR\\upx.exe\" ) {

		 # if available, use upx to compress the file
		 version.commands = "\"$$DESTDIR\\upx.exe\" -9 -f -o \"$$DESTDIR\\$$TARGET-"$$VERSION".exe\""  "\"$$DESTDIR\\"$$TARGET".exe\"

	} else {

		 # simple copy
		 version.commands = @copy "\"$$DESTDIR\\"$$TARGET".exe\" \"$$DESTDIR\\$$TARGET-"$$VERSION".exe\""

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

HEADERS = \
Application.h \
AskForSaveDialog.h \
AttachmentFrame.h \
AttachmentModel.h \
AttachmentWindow.h \
BackupManagerDialog.h \
BackupManagerWidget.h \
ConfigurationDialog.h \
DeleteAttachmentDialog.h \
DeleteKeywordDialog.h \
EditAttachmentDialog.h \
EditionWindow.h \
EditKeywordDialog.h \
FileCheckDialog.h \
FormatBar.h \
HtmlHeaderNode.h \
InsertLinkDialog.h \
KeywordModel.h \
LogbookHtmlHelper.h \
LogbookInformationDialog.h \
LogbookModifiedDialog.h \
LogbookPrintHelper.h \
LogbookPrintOptionWidget.h \
LogbookStatisticsDialog.h \
LogEntryHtmlHelper.h \
LogEntryInformationDialog.h \
LogEntryModel.h \
LogEntryPrintHelper.h \
LogEntryPrintOptionWidget.h \
LogEntryPrintSelectionWidget.h \
MainWindow.h \
Menu.h \
NewAttachmentDialog.h \
NewLogbookDialog.h \
OpenAttachmentDialog.h \
ProgressBar.h \
SearchPanel.h \
OpenLinkDialog.h

SOURCES = \
Application.cpp \
AskForSaveDialog.cpp \
AttachmentFrame.cpp \
AttachmentModel.cpp \
AttachmentWindow.cpp \
BackupManagerDialog.cpp \
BackupManagerWidget.cpp \
ConfigurationDialog.cpp \
DeleteAttachmentDialog.cpp \
DeleteKeywordDialog.cpp \
EditAttachmentDialog.cpp \
EditionWindow.cpp \
EditKeywordDialog.cpp \
FileCheckDialog.cpp \
FormatBar.cpp \
HtmlHeaderNode.cpp \
InsertLinkDialog.cpp \
KeywordModel.cpp \
LogbookHtmlHelper.cpp \
LogbookInformationDialog.cpp \
LogbookModifiedDialog.cpp \
LogbookPrintHelper.cpp \
LogbookPrintOptionWidget.cpp \
LogbookStatisticsDialog.cpp \
LogEntryHtmlHelper.cpp \
LogEntryInformationDialog.cpp \
LogEntryPrintHelper.cpp \
LogEntryPrintOptionWidget.cpp \
LogEntryPrintSelectionWidget.cpp \
LogEntryModel.cpp \
MainWindow.cpp \
Menu.cpp \
NewAttachmentDialog.cpp \
NewLogbookDialog.cpp \
OpenAttachmentDialog.cpp \
SearchPanel.cpp \
OpenLinkDialog.cpp \
elogbook.cpp
