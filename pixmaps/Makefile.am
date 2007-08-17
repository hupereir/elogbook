AUTOMAKE_OPTIONS = foreign

noinst_LIBRARIES = libpixmaps.a
libpixmaps_a_SOURCES = qrc_pixmaps.cpp

INCLUDES =  \
  -I$(qt_includes) \
  -I$(qt_includes)/Qt \
  -I$(qt_includes)/QtCore 

EXTRA_DIST = \
  pixmaps.qrc \
  pixmaps.pro \
  1leftarrow.png \
  1rightarrow.png \
  attach.png \
  color.png \
  editcopy.png \
  edit.png \
  exit.png \
  filenew.png \
  fileopen.png \
  filesave.png \
  filesaveas.png \
  fonts.png \
  homeB.png \
  html.png \
  icon.png \
  info.png \
  encrypted.png \
  redo.png \
  reload.png \
  text_bold.png \
  text_italic.png \
  text_strike.png \
  text_under.png \
  trashcan_empty.png \
  undo.png

clean-local:
	rm -f qrc_*.cpp

### rule to generate qt meta objects
qrc_%.cpp: %.qrc
	$(qt_rcc) -name $* $^ -o $@
