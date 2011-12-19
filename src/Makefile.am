EXTRA_DIST = elogbook.rc src.pro elogbook-common.pro elogbook.pro copy_logbook.pro synchronize_logbook.pro pixmaps.qrc

if WITH_ASPELL
INCLUDES = \
	-I$(aspell_includes) \
	-I$(qt_includes) \
	-I$(qt_includes)/Qt \
	-I$(qt_includes)/QtCore  \
	-I$(qt_includes)/QtNetwork  \
	-I$(qt_includes)/QtXml \
	-I$(qt_includes)/QtGui \
	-I$(top_srcdir)/base  \
	-I$(top_srcdir)/base-qt \
	-I$(top_srcdir)/base-help \
	-I$(top_srcdir)/base-server \
	-I$(top_srcdir)/base-spellcheck
else
INCLUDES = \
	-I$(qt_includes) \
	-I$(qt_includes)/Qt \
	-I$(qt_includes)/QtCore  \
	-I$(qt_includes)/QtNetwork  \
	-I$(qt_includes)/QtXml \
	-I$(qt_includes)/QtGui \
	-I$(top_srcdir)/base  \
	-I$(top_srcdir)/base-qt \
	-I$(top_srcdir)/base-help \
	-I$(top_srcdir)/base-server
endif

CXXFLAGS = @CXXFLAGS@

noinst_LIBRARIES = libelogbook.a
bin_PROGRAMS = elogbook copy_logbook synchronize_logbook

##################
# shared libraries
if ENABLE_SHARED
if WITH_ASPELL
libraries = \
  -L$(libdir) -lbase-spellcheck -lbase-server -lbase-help \
	libelogbook.a

else

libraries =  \
  -L$(libdir) -lbase-server -lbase-help \
	libelogbook.a

endif

else
##################
# static libraries
if WITH_ASPELL
libraries = \
-L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork \
-L$(aspell_libraries) -laspell  \
libelogbook.a \
../base-spellcheck/libbase-spellcheck.a \
../base-server/libbase-server.a \
../base-help/libbase-help.a \
../base-qt/libbase-qt.a \
../base/libbase.a

else

libraries =  \
-L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork \
libelogbook.a \
../base-server/libbase-server.a \
../base-help/libbase-help.a \
../base-qt/libbase-qt.a \
../base/libbase.a

endif
endif

#################
# headers
noinst_HEADERS = \
Application.h \
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
FileCheck.h \
FileCheckDialog.h \
FormatBar.h \
HelpText.h \
Icons.h \
Keyword.h \
KeywordModel.h \
Logbook.h \
LogbookInformationDialog.h \
LogbookModifiedDialog.h \
LogbookPrintHelper.h \
LogbookPrintOptionWidget.h \
LogbookStatisticsDialog.h \
LogEntry.h \
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
SelectionStatusBar.h \
XmlDef.h

#####################
# elogbook library sources
libelogbook_a_SOURCES = \
Attachment.cpp \
AttachmentType.cpp \
FileCheck.cpp \
Keyword.cpp \
Logbook.cpp \
LogEntry.cpp

nodist_libelogbook_a_SOURCES = \
moc_FileCheck.cpp \
moc_Logbook.cpp

###############################
# elogbook
elogbook_LDADD =  $(libraries)
elogbook_SOURCES = \
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
LogbookPrintHelper.cpp \
LogbookPrintOptionWidget.cpp \
LogbookStatisticsDialog.cpp \
LogEntryInformationDialog.cpp \
LogEntryModel.cpp \
LogEntryPrintHelper.cpp \
LogEntryPrintOptionWidget.cpp \
LogEntryPrintSelectionWidget.cpp \
MainWindow.cpp \
Menu.cpp \
NewAttachmentDialog.cpp \
NewLogbookDialog.cpp \
OpenAttachmentDialog.cpp \
ProgressBar.cpp \
SearchPanel.cpp \
SelectionStatusBar.cpp \
elogbook.cpp

nodist_elogbook_SOURCES = \
qrc_pixmaps.cpp \
moc_Application.cpp \
moc_AskForSaveDialog.cpp \
moc_AttachmentFrame.cpp \
moc_AttachmentModel.cpp \
moc_AttachmentWindow.cpp \
moc_EditionWindow.cpp \
moc_FormatBar.cpp \
moc_KeywordModel.cpp \
moc_LogbookModifiedDialog.cpp \
moc_LogbookPrintHelper.cpp \
moc_LogEntryModel.cpp \
moc_LogEntryPrintHelper.cpp \
moc_MainWindow.cpp \
moc_Menu.cpp \
moc_NewAttachmentDialog.cpp \
moc_ProgressBar.cpp \
moc_SearchPanel.cpp \
moc_SelectionStatusBar.cpp

###############################
# copy_logbook
copy_logbook_LDADD =  $(libraries)
copy_logbook_SOURCES = copy_logbook.cpp

###############################
# synchronize_logbook
synchronize_logbook_LDADD =  $(libraries)
synchronize_logbook_SOURCES = synchronize_logbook.cpp

clean-local:
	rm -f moc_*.cpp
	rm -f qrc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
