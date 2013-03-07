# $Id$

########### X11 #########
IF( UNIX )

  FIND_PACKAGE( X11 REQUIRED )
  INCLUDE_DIRECTORIES( ${X11_X11_INCLUDE_PATH} )

ENDIF()

########### Qt4 configuration #########
IF( USE_QT5 )

  FIND_PACKAGE( Qt5Widgets REQUIRED )

ELSE()

  FIND_PACKAGE( Qt4 REQUIRED )
  SET( QT_USE_QTXML TRUE )
  SET( QT_USE_QTNETWORK TRUE )
  INCLUDE( ${QT_USE_FILE} )
  ADD_DEFINITIONS( ${QT_DEFINITIONS} )

ENDIF()

########### includes #########
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-qt)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-server)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-help)

IF( ASPELL_FOUND )
  INCLUDE_DIRECTORIES(${ASPELL_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/base-spellcheck)
ENDIF()

########### configuration files ###############
CONFIGURE_FILE( Config.h.cmake Config.h )
CONFIGURE_FILE( SystemOptions.h.cmake SystemOptions.h )

########### next target ###############
SET( elogbook_lib_SOURCES
  Attachment.cpp
  AttachmentType.cpp
  FileCheck.cpp
  Keyword.cpp
  Logbook.cpp
  LogbookThread.cpp
  LogEntry.cpp
)

SET( elogbook_lib_HEADERS
  FileCheck.h
  Logbook.h
)

IF( USE_QT5 )

  QT5_WRAP_CPP( elogbook_lib_HEADERS_MOC ${elogbook_lib_HEADERS} )

ELSE()

  QT4_WRAP_CPP( elogbook_lib_HEADERS_MOC ${elogbook_lib_HEADERS} )

ENDIF()

ADD_LIBRARY( elogbook_lib STATIC
  ${elogbook_lib_SOURCES}
  ${elogbook_lib_HEADERS_MOC}
)

TARGET_LINK_LIBRARIES( elogbook_lib ${QT_LIBRARIES} )

IF( USE_QT5 )

  QT5_USE_MODULES( elogbook_lib Widgets Xml )

ENDIF()

########### next target ###############
SET( elogbook_SOURCES
  Application.cpp
  AskForSaveDialog.cpp
  AttachmentFrame.cpp
  AttachmentModel.cpp
  AttachmentWindow.cpp
  BackupManagerDialog.cpp
  BackupManagerWidget.cpp
  ConfigurationDialog.cpp
  DeleteAttachmentDialog.cpp
  DeleteKeywordDialog.cpp
  EditAttachmentDialog.cpp
  EditionWindow.cpp
  EditKeywordDialog.cpp
  FileCheckDialog.cpp
  FormatBar.cpp
  HtmlHeaderNode.cpp
  InsertLinkDialog.cpp
  KeywordModel.cpp
  LogbookHtmlHelper.cpp
  LogbookInformationDialog.cpp
  LogbookModifiedDialog.cpp
  LogbookPrintHelper.cpp
  LogbookPrintOptionWidget.cpp
  LogbookStatisticsDialog.cpp
  LogEntryHtmlHelper.cpp
  LogEntryInformationDialog.cpp
  LogEntryModel.cpp
  LogEntryPrintHelper.cpp
  LogEntryPrintOptionWidget.cpp
  LogEntryPrintSelectionWidget.cpp
  MainWindow.cpp
  Menu.cpp
  NewAttachmentDialog.cpp
  NewLogbookDialog.cpp
  OpenAttachmentDialog.cpp
  SearchPanel.cpp
  OpenLinkDialog.cpp
  main.cpp
)

SET( elogbook_HEADERS
  Application.h
  AskForSaveDialog.h
  AttachmentFrame.h
  AttachmentModel.h
  AttachmentWindow.h
  BackupManagerWidget.h
  EditionWindow.h
  FormatBar.h
  KeywordModel.h
  LogbookHtmlHelper.h
  LogbookModifiedDialog.h
  LogbookPrintHelper.h
  LogbookPrintOptionWidget.h
  LogEntryHtmlHelper.h
  LogEntryModel.h
  LogEntryPrintHelper.h
  LogEntryPrintSelectionWidget.h
  LogEntryPrintOptionWidget.h
  MainWindow.h
  Menu.h
  NewAttachmentDialog.h
  ProgressBar.h
  SearchPanel.h
  OpenLinkDialog.h
)

SET( elogbook_RESOURCES pixmaps.qrc)

IF( USE_QT5 )

  QT5_WRAP_CPP( elogbook_HEADERS_MOC ${elogbook_HEADERS} )
  QT5_ADD_RESOURCES( elogbook_RESOURCES_RCC ${elogbook_RESOURCES})

ELSE()

  QT4_WRAP_CPP( elogbook_HEADERS_MOC ${elogbook_HEADERS} )
  QT4_ADD_RESOURCES( elogbook_RESOURCES_RCC ${elogbook_RESOURCES})

ENDIF()

ADD_APPLICATION_ICON( elogbook_SOURCES ${CMAKE_SOURCE_DIR}/icon )
ADD_PLATFORM_EXECUTABLE( elogbook
  ${ELOGBOOK_VERSION}
  ${elogbook_SOURCES}
  ${elogbook_HEADERS_MOC}
  ${elogbook_RESOURCES_RCC}
)

TARGET_LINK_LIBRARIES( elogbook ${QT_LIBRARIES} )
TARGET_LINK_LIBRARIES( elogbook
  base
  base-qt
  base-server
  base-help
)

IF( ASPELL_FOUND )
  TARGET_LINK_LIBRARIES( elogbook base-spellcheck )
ENDIF()

TARGET_LINK_LIBRARIES( elogbook elogbook_lib )
INSTALL( TARGETS elogbook DESTINATION ${BIN_INSTALL_DIR} )

IF( USE_QT5 )

  QT5_USE_MODULES( elogbook Widgets Network PrintSupport Xml )

ENDIF()


########### next target ###############
IF( UNIX )

  SET( copy_logbook_SOURCES copy-logbook.cpp )
  ADD_EXECUTABLE( copy-logbook ${copy_logbook_SOURCES} )
  TARGET_LINK_LIBRARIES( copy-logbook ${QT_LIBRARIES} )
  TARGET_LINK_LIBRARIES( copy-logbook
    base
    base-qt
    base-server
    elogbook_lib
  )

  INSTALL( TARGETS copy-logbook DESTINATION ${BIN_INSTALL_DIR} )

  IF( USE_QT5 )

    QT5_USE_MODULES( copy-logbook Widgets Xml )

  ENDIF()

ENDIF()

########### next target ###############
IF( UNIX )

  SET( synchronize_logbook_SOURCES synchronize-logbook.cpp )
  ADD_EXECUTABLE( synchronize-logbook ${synchronize_logbook_SOURCES} )
  TARGET_LINK_LIBRARIES( synchronize-logbook ${X11_X11_LIB} )
  TARGET_LINK_LIBRARIES( synchronize-logbook ${QT_LIBRARIES} )
  TARGET_LINK_LIBRARIES( synchronize-logbook
    base
    base-qt
    base-server
    elogbook_lib
  )

  INSTALL( TARGETS synchronize-logbook DESTINATION ${BIN_INSTALL_DIR} )

  IF( USE_QT5 )

    QT5_USE_MODULES( synchronize-logbook Widgets Xml )

  ENDIF()

ENDIF()
