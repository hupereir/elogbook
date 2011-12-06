ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign

if WITH_ASPELL
SPELLCHECK_SOURCE_DIR=base-spellcheck
else
SPELLCHECK_SOURCE_DIR=
endif

# only compile base directories in static mode
if ENABLE_SHARED
BASE_DIRS=
else
BASE_DIRS= \
 base \
 base-qt \
 base-help \
 base-server \
 base-pixmaps \
 $(SPELLCHECK_SOURCE_DIR)
endif

SUBDIRS = \
 $(BASE_DIRS) \
 pixmaps \
 extra-includes \
 src

EXTRA_DIST = \
 INSTALL \
 COPYING\
 icon.ico \
 icon.png \
 autogen.sh \
 elogbook.spec.in \
 elogbook.spec \
 elogbook.pro
