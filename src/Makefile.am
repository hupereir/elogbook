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
  Attachment.h \
  AttachmentType.h \
  DeleteAttachmentDialog.h \
  DeleteKeywordDialog.h \
  EditAttachmentDialog.h \
  EditKeywordDialog.h \
  HtmlUtil.h \
  Logbook.h \
  LogbookInfoDialog.h \
  LogbookStatisticsDialog.h \
  LogEntry.h \
  NewAttachmentDialog.h \
  NewLogbookDialog.h \
  OpenAttachmentDialog.h \
  ViewHtmlEntryDialog.h \
  ViewHtmlLogbookDialog.h \
  XmlDef.h

eLogbook_SOURCES = eLogbook.cc
copy_logbook_SOURCES = copy_logbook.cc
	
libeLogbook_a_SOURCES = \
  AskForSaveDialog.cc \
  Attachment.cc \
  AttachmentType.cc \
  copy_logbook.cc \
  DeleteAttachmentDialog.cc \
  DeleteKeywordDialog.cc \
  EditAttachmentDialog.cc \
  EditKeywordDialog.cc \
  HtmlUtil.cc \
  Logbook.cc \
  LogbookInfoDialog.cc \
  LogbookStatisticsDialog.cc \
  LogEntry.cc \
  NewAttachmentDialog.cc \
  NewLogbookDialog.cc \
  OpenAttachmentDialog.cc \
  ViewHtmlEntryDialog.cc \
  ViewHtmlLogbookDialog.cc
 	
nodist_libeLogbook_a_SOURCES = \
  moc_Logbook.cc

### rule to generate qt meta objects
moc_%.cc: %.h
	$(qt_moc) $^ -o $@
