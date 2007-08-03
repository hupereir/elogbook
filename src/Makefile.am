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

noinst_LIBRARIES = libeLogbook.a
bin_PROGRAMS = eLogbook copy_logbook synchronize_logbook

if WITH_ASPELL 
libraries = \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  -L$(aspell_libraries) -laspell  \
  libeLogbook.a \
  ../base-spellcheck/libbase-spellcheck.a \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

else

libraries =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  libeLogbook.a \
  ../base-server/libbase-server.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

endif

eLogbook_LDADD =  $(libraries)
copy_logbook_LDADD =  $(libraries)
synchronize_logbook_LDADD =  $(libraries)

noinst_HEADERS = \
  AskForSaveDialog.h \
  AttachmentFrame.h \
  Attachment.h \
  AttachmentList.h \
  AttachmentType.h \
  ConfigDialog.h \
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

eLogbook_SOURCES = eLogbook.cc
copy_logbook_SOURCES = copy_logbook.cc
synchronize_logbook_SOURCES = synchronize_logbook.cc

libeLogbook_a_SOURCES = \
  AskForSaveDialog.cc \
  Attachment.cc \
  AttachmentFrame.cc \
  AttachmentList.cc \
  AttachmentType.cc \
  ConfigDialog.cc \
  copy_logbook.cc \
  DeleteAttachmentDialog.cc \
  DeleteKeywordDialog.cc \
  EditAttachmentDialog.cc \
  EditFrame.cc \
  EditKeywordDialog.cc \
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

nodist_libeLogbook_a_SOURCES = \
  moc_AskForSaveDialog.cc \
  moc_AttachmentFrame.cc \
  moc_AttachmentList.cc \
  moc_EditFrame.cc \
  moc_FormatBar.cc \
  moc_KeywordList.cc \
  moc_Logbook.cc \
  moc_LogbookModifiedDialog.cc \
  moc_LogEntryList.cc \
  moc_MainFrame.cc \
  moc_Menu.cc \
  moc_NewAttachmentDialog.cc \
  moc_SearchPanel.cc \
  moc_SelectionFrame.cc \
  moc_SplashScreen.cc

### rule to generate qt meta objects
moc_%.cc: %.h
	$(qt_moc) $^ -o $@
