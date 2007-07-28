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
  -I$(top_srcdir)/server \
  -I$(top_srcdir)/spellcheck
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
  -I$(top_srcdir)/server
endif

CXXFLAGS = @CXXFLAGS@

noinst_LIBRARIES = libeLogbook.a
bin_PROGRAMS = eLogbook copy_logbook

if WITH_ASPELL 

eLogbook_LDADD =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  -L$(aspell_libraries) -laspell  \
  libeLogbook.a \
  ../spellcheck/libspellcheck.a \
  ../server/libserver.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

copy_logbook_LDADD =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  -L$(aspell_libraries) -laspell  \
  libeLogbook.a \
  ../spellcheck/libspellcheck.a \
  ../server/libserver.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

else

eLogbook_LDADD =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  libeLogbook.a \
  ../server/libserver.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

copy_logbook_LDADD =  \
  -L$(qt_libraries) -l$(qt_lib) -lQtGui -lQtXml -lQtNetwork\
  libeLogbook.a \
  ../server/libserver.a \
  ../base-qt/libbase-qt.a \
  ../base/libbase.a

endif

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
  HtmlUtil.h \
  KeywordList.h \
  Logbook.h \
  LogbookInfoDialog.h \
  LogbookModifiedDialog.h \
  LogbookStatisticsDialog.h \
  LogEntry.h \
  LogEntryList.h \
  MainFrame.h \
  Menu.h \
  NewAttachmentDialog.h \
  NewLogbookDialog.h \
  OpenAttachmentDialog.h \
  SearchPanel.h \
  SelectionFrame.h \
  SplashScreen.h \
  TopWidget.h \
  ViewHtmlEntryDialog.h \
  ViewHtmlLogbookDialog.h \
  XmlDef.h

eLogbook_SOURCES = eLogbook.cc
copy_logbook_SOURCES = copy_logbook.cc
  
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
  HtmlUtil.cc \
  KeywordList.cc \
  Logbook.cc \
  LogbookInfoDialog.cc \
  LogbookModifiedDialog.cc \
  LogbookStatisticsDialog.cc \
  LogEntry.cc \
  LogEntryList.cc \
  MainFrame.cc \
  Menu.cc \
  NewAttachmentDialog.cc \
  NewLogbookDialog.cc \
  OpenAttachmentDialog.cc \
  SearchPanel.cc \
  SelectionFrame.cc \
  SplashScreen.cc \
  TopWidget.cc \
  ViewHtmlEntryDialog.cc \
  ViewHtmlLogbookDialog

nodist_libeLogbook_a_SOURCES = \
  moc_AskForSaveDialog.h.cc \
  moc_AttachmentFrame.h.cc \
  moc_AttachmentList.h.cc \
  moc_EditFrame.h.cc \
  moc_KeywordList.h.cc \
  moc_Logbook.h.cc \
  moc_LogbookModifiedDialog.h.cc \
  moc_MainFrame.h.cc \
  moc_Menu.h.cc \
  moc_NewAttachmentDialog.h.cc \
  moc_SearchPanel.h.cc \
  moc_SelectionFrame.h.cc \
  moc_SplashScreen.h.cc \
  moc_TopWidget.h.cc

### rule to generate qt meta objects
moc_%.cc: %.h
	$(qt_moc) $^ -o $@
