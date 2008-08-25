AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = elogbook.rc src.pro pixmaps.qrc

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

if WITH_ASPELL 
libraries = \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  -L$(aspell_libraries) -laspell  \
  libelogbook.a \
  ../base-spellcheck/libbase-spellcheck.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

else

libraries =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  libelogbook.a \
  ../base-server/libbase-server.a \
  ../base-help/libbase-help.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

endif

elogbook_LDADD =  $(libraries)
copy_logbook_LDADD =  $(libraries)
synchronize_logbook_LDADD =  $(libraries)


libelogbook_a_SOURCES = \
  Attachment.cpp \
  AttachmentType.cpp \
  HtmlUtil.cpp \
  Keyword.cpp \
  Logbook.cpp \
  LogEntry.cpp

nodist_libelogbook_a_SOURCES = \
  moc_Logbook.cpp

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
  MainWindow.h \
  Menu.h \
  NewAttachmentDialog.h \
  NewLogbookDialog.h \
  OpenAttachmentDialog.h \
  ProgressBar.h \
  SearchPanel.h \
  SelectionStatusBar.h \
  SplashScreen.h \
  ViewHtmlEntryDialog.h \
  ViewHtmlLogbookDialog.h \
  XmlDef.h

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
  FormatBar.cpp \
  KeywordDelegate.cpp \
  KeywordModel.cpp \
  LogbookInformationDialog.cpp \
  LogbookModifiedDialog.cpp \
  LogbookStatisticsDialog.cpp \
  LogEntryDelegate.cpp \
  LogEntryInformationDialog.cpp \
  LogEntryModel.cpp \
  MainWindow.cpp \
  Menu.cpp \
  NewAttachmentDialog.cpp \
  NewLogbookDialog.cpp \
  OpenAttachmentDialog.cpp \
  ProgressBar.cpp \
  SearchPanel.cpp \
  SelectionStatusBar.cpp \
  SplashScreen.cpp \
  ViewHtmlEntryDialog.cpp \
  ViewHtmlLogbookDialog.cpp \
  elogbook.cpp

nodist_elogbook_SOURCES = \
  qrc_pixmaps.cpp \
  moc_Application.cpp \
  moc_AskForSaveDialog.cpp \
  moc_AttachmentFrame.cpp \
  moc_AttachmentWindow.cpp \
  moc_ConfigurationDialog.cpp \
  moc_EditionWindow.cpp \
  moc_FormatBar.cpp \
  moc_KeywordModel.cpp \
  moc_LogbookModifiedDialog.cpp \
  moc_LogEntryModel.cpp \
  moc_MainWindow.cpp \
  moc_Menu.cpp \
  moc_NewAttachmentDialog.cpp \
  moc_ProgressBar.cpp \
  moc_SearchPanel.cpp \
  moc_SelectionStatusBar.cpp \
  moc_SplashScreen.cpp

copy_logbook_SOURCES = copy_logbook.cpp
synchronize_logbook_SOURCES = synchronize_logbook.cpp

clean-local:
	rm -f moc_*.cpp
	rm -f qrc_*.cpp

### rule to generate qt meta objects
moc_%.cpp: %.h
	$(qt_moc) $^ -o $@

qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
