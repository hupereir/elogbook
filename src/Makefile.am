AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = src.pro pixmaps.qrc

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
  Logbook.cpp \
  LogEntry.cpp

nodist_libelogbook_a_SOURCES = \
  moc_Logbook.cpp

noinst_HEADERS = \
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
  LogbookInformationDialog.h \
  LogbookModifiedDialog.h \
  LogbookStatisticsDialog.h \
  LogEntry.h \
  LogEntryInformationDialog.h \
  LogEntryModel.h \
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
  AskForSaveDialog.cpp \
  AttachmentFrame.cpp \
  AttachmentList.cpp \
  ConfigurationDialog.cpp \
  DeleteAttachmentDialog.cpp \
  DeleteKeywordDialog.cpp \
  EditAttachmentDialog.cpp \
  EditFrame.cpp \
  EditKeywordDialog.cpp \
  FormatBar.cpp \
  KeywordList.cpp \
  LogbookInformationDialog.cpp \
  LogbookModifiedDialog.cpp \
  LogbookStatisticsDialog.cpp \
  LogEntryInformationDialog.cpp \
  LogEntryModel.cpp \
  MainFrame.cpp \
  Menu.cpp \
  NewAttachmentDialog.cpp \
  NewLogbookDialog.cpp \
  OpenAttachmentDialog.cpp \
  SearchPanel.cpp \
  SelectionFrame.cpp \
  SplashScreen.cpp \
  ViewHtmlEntryDialog.cpp \
  ViewHtmlLogbookDialog.cpp \
  elogbook.cpp

nodist_elogbook_SOURCES = \
  qrc_pixmaps.cpp \
  moc_AskForSaveDialog.cpp \
  moc_AttachmentFrame.cpp \
  moc_AttachmentList.cpp \
  moc_ConfigurationDialog.cpp \
  moc_EditFrame.cpp \
  moc_FormatBar.cpp \
  moc_KeywordList.cpp \
  moc_LogbookModifiedDialog.cpp \
  moc_MainFrame.cpp \
  moc_Menu.cpp \
  moc_NewAttachmentDialog.cpp \
  moc_SearchPanel.cpp \
  moc_SelectionFrame.cpp \
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
