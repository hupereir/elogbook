AUTOMAKE_OPTIONS = foreign

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
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

else

libraries =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  libelogbook.a \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

endif

elogbook_LDADD =  $(libraries)
copy_logbook_LDADD =  $(libraries)
synchronize_logbook_LDADD =  $(libraries)


libelogbook_a_SOURCES = \
  Attachment.cc \
  AttachmentType.cc \
  HtmlUtil.cc \
  Logbook.cc \
  LogEntry.cc

nodist_libelogbook_a_SOURCES = \
  moc_Logbook.cc

noinst_HEADERS = \
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

elogbook_SOURCES = \
  AskForSaveDialog.cc \
  AttachmentFrame.cc \
  AttachmentList.cc \
  ConfigurationDialog.cc \
  DeleteAttachmentDialog.cc \
  DeleteKeywordDialog.cc \
  EditAttachmentDialog.cc \
  EditFrame.cc \
  EditKeywordDialog.cc \
  FormatBar.cc \
  KeywordList.cc \
  LogbookInfoDialog.cc \
  LogbookModifiedDialog.cc \
  LogbookStatisticsDialog.cc \
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
  ViewHtmlLogbookDialog.cc \
  eLogbook.cc

nodist_elogbook_SOURCES = \
  moc_AskForSaveDialog.cc \
  moc_AttachmentFrame.cc \
  moc_AttachmentList.cc \
  moc_EditFrame.cc \
  moc_FormatBar.cc \
  moc_KeywordList.cc \
  moc_LogbookModifiedDialog.cc \
  moc_LogEntryList.cc \
  moc_MainFrame.cc \
  moc_Menu.cc \
  moc_NewAttachmentDialog.cc \
  moc_SearchPanel.cc \
  moc_SelectionFrame.cc \
  moc_SplashScreen.cc

copy_logbook_SOURCES = copy_logbook.cc
synchronize_logbook_SOURCES = synchronize_logbook.cc

### rule to generate qt meta objects
moc_%.cc: %.h
	$(qt_moc) $^ -o $@
