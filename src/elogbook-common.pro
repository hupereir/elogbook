# $Id$

TEMPLATE = lib
CONFIG = staticlib qt

CONFIG += qt
QT += xml network

mac {
	DEFINES += QT_NO_DBUS
}

unix:!mac {
	QT += dbus
}

INCLUDEPATH += ../base ../base-qt ../base-server ../base-help ../extra-includes
DEPENDPATH += . ../base ../base-qt ../base-server ../base-help ../extra-includes

HEADERS = \
	Attachment.h \
	AttachmentType.h \
	FileCheck.h \
	Keyword.h \
	Logbook.h \
	LogEntry.h

SOURCES = \
	Attachment.cpp \
	AttachmentType.cpp \
	FileCheck.cpp \
	Keyword.cpp \
	Logbook.cpp \
	LogEntry.cpp
