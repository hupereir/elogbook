# $Id$
########### Qt configuration #########
if(USE_QT5)

  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Network REQUIRED)
  find_package(Qt5Xml REQUIRED)

else()

  find_package(Qt4 REQUIRED)
  set(QT_USE_QTXML TRUE)
  set(QT_USE_QTNETWORK TRUE)
  include(${QT_USE_FILE})
  add_definitions(${QT_DEFINITIONS})

endif()

########### includes #########
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/base)
include_directories(${CMAKE_SOURCE_DIR}/base-qt)
include_directories(${CMAKE_SOURCE_DIR}/base-server)
include_directories(${CMAKE_SOURCE_DIR}/base-help)

if(ASPELL_FOUND)
  include_directories(${ASPELL_INCLUDE_DIR})
  include_directories(${CMAKE_SOURCE_DIR}/base-spellcheck)
endif()

########### configuration files ###############
configure_file(Config.h.cmake Config.h)
configure_file(SystemOptions.h.cmake SystemOptions.h)

########### next target ###############
set(elogbook_lib_SOURCES
  Attachment.cpp
  AttachmentType.cpp
  Backup.cpp
  FileCheck.cpp
  Keyword.cpp
  Logbook.cpp
  LogEntry.cpp
)

########### next target ###############
set(elogbook_SOURCES
  Application.cpp
  AskForSaveDialog.cpp
  AttachmentFrame.cpp
  AttachmentModel.cpp
  AttachmentWindow.cpp
  BackupManagerDialog.cpp
  BackupManagerWidget.cpp
  BackupModel.cpp
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
  LogbookModel.cpp
  LogbookModifiedDialog.cpp
  LogbookPrintHelper.cpp
  LogbookPrintOptionWidget.cpp
  LogbookStatisticsDialog.cpp
  LogEntryHtmlHelper.cpp
  LogEntryInformationDialog.cpp
  LogEntryModel.cpp
  LogEntryPrintHelper.cpp
  LogEntryPrintOptionWidget.cpp
  LogEntryPrintSelectionDialog.cpp
  LogEntryPrintSelectionWidget.cpp
  MainWindow.cpp
  Menu.cpp
  NewAttachmentDialog.cpp
  NewLogbookDialog.cpp
  OpenAttachmentDialog.cpp
  ProgressBar.cpp
  SearchPanel.cpp
  OpenLinkDialog.cpp
  main.cpp
)

set(elogbook_RESOURCES pixmaps.qrc)

if(USE_QT5)

  qt5_add_resources(elogbook_RESOURCES_RCC ${elogbook_RESOURCES})

else()

  qt4_add_resources(elogbook_RESOURCES_RCC ${elogbook_RESOURCES})

endif()

add_application_icon(elogbook_SOURCES ${CMAKE_SOURCE_DIR}/elogbook)
add_desktop_file(${CMAKE_SOURCE_DIR}/elogbook)
add_platform_executable(elogbook
  ${ELOGBOOK_VERSION}
  ${elogbook_lib_SOURCES}
  ${elogbook_SOURCES}
  ${elogbook_RESOURCES_RCC}
)

target_link_libraries(elogbook ${QT_LIBRARIES})
target_link_libraries(elogbook
  base
  base-qt
  base-server
  base-help
)

if(ASPELL_FOUND)
  target_link_libraries(elogbook base-spellcheck)
endif()

install(TARGETS elogbook DESTINATION ${BIN_INSTALL_DIR})

if(USE_QT5)

  qt5_use_modules(elogbook Widgets Network PrintSupport Xml)

endif()


########### next target ###############
if(UNIX)

  set(copy_logbook_SOURCES copy-logbook.cpp)
  add_executable(copy-logbook
    ${elogbook_lib_SOURCES}
    ${copy_logbook_SOURCES})

  target_link_libraries(copy-logbook ${QT_LIBRARIES})
  target_link_libraries(copy-logbook
    base
    base-qt
    base-server)

  install(TARGETS copy-logbook DESTINATION ${BIN_INSTALL_DIR})

  if(USE_QT5)

    qt5_use_modules(copy-logbook Widgets Xml)

  endif()

endif()

########### next target ###############
if(UNIX)

  set(synchronize_logbook_SOURCES synchronize-logbook.cpp)
  add_executable(synchronize-logbook
    ${elogbook_lib_SOURCES}
    ${synchronize_logbook_SOURCES})

  target_link_libraries(synchronize-logbook ${QT_LIBRARIES})
  target_link_libraries(synchronize-logbook
    base
    base-qt
    base-server)

  install(TARGETS synchronize-logbook DESTINATION ${BIN_INSTALL_DIR})

  if(USE_QT5)

    qt5_use_modules(synchronize-logbook Widgets Xml)

  endif()

endif()
