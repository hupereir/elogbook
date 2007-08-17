AUTOMAKE_OPTIONS = foreign

if WITH_ASPELL 
SPELLCHECK_SOURCE_DIR=base-spellcheck
else 
SPELLCHECK_SOURCE_DIR=
endif

SUBDIRS =  base base-qt base-server $(SPELLCHECK_SOURCE_DIR) pixmaps src
EXTRA_DIST = INSTALL COPYING autogen.sh eLogbook.spec.in eLogbook.spec eLogbook.pro
