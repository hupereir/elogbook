AUTOMAKE_OPTIONS = foreign

if WITH_ASPELL 
SPELLCHECK_SOURCE_DIR=base-spellcheck
else 
SPELLCHECK_SOURCE_DIR=
endif

SUBDIRS = \
 pixmaps \
 base-pixmaps \
 base \
 base-qt \
 base-help \
 base-server \
 base-svg \
 $(SPELLCHECK_SOURCE_DIR) \
 base-transparency \
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
