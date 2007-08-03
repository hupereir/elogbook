AUTOMAKE_OPTIONS = foreign

elogbookdir = $(prefix)/share/eLogbook
elogbook_DATA = icon.png

if WITH_ASPELL 
SPELLCHECK_SOURCE_DIR=base-spellcheck
else 
SPELLCHECK_SOURCE_DIR=
endif

SUBDIRS =  base base-qt base-server $(SPELLCHECK_SOURCE_DIR) src pixmaps
EXTRA_DIST = INSTALL COPYING autogen.sh icon.png eLogbook.spec.in eLogbook.spec eLogbook.pro
