########### Qt configuration #########
find_package(Qt5 COMPONENTS Widgets Network Xml PrintSupport REQUIRED)

if( WIN32 AND NOT USE_SHARED_LIBS )
  fix_win32_static_compilation()
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
  ColorMenu.cpp
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
  KeywordList.cpp
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
  LogEntryList.cpp
  LogEntryModel.cpp
  LogEntryPrintHelper.cpp
  LogEntryPrintOptionWidget.cpp
  LogEntryPrintSelectionDialog.cpp
  LogEntryPrintSelectionWidget.cpp
  MainWindow.cpp
  MenuBar.cpp
  NewAttachmentDialog.cpp
  NewLogbookDialog.cpp
  OpenAttachmentDialog.cpp
  ProgressBar.cpp
  SearchWidget.cpp
  ToolTipWidget.cpp
  main.cpp
)

set(elogbook_RESOURCES pixmaps.qrc)

add_application_icon(elogbook_SOURCES ${CMAKE_SOURCE_DIR}/elogbook)
add_desktop_file(${CMAKE_SOURCE_DIR}/elogbook)
add_platform_executable(elogbook
  ${ELOGBOOK_VERSION}
  ${elogbook_lib_SOURCES}
  ${elogbook_SOURCES}
  ${elogbook_RESOURCES}
)

target_link_libraries(elogbook
  base
  base-qt
  base-server
  base-help
)

if(ASPELL_FOUND)
  target_link_libraries(elogbook base-spellcheck)
endif()

target_link_libraries(elogbook Qt::Widgets Qt::Network Qt::PrintSupport Qt::Xml)
install(TARGETS elogbook DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############
if(UNIX)
  set(copy_logbook_SOURCES copy-logbook.cpp)
  add_executable(copy-logbook
    ${elogbook_lib_SOURCES}
    ${copy_logbook_SOURCES})

  target_link_libraries(copy-logbook
    base
    base-qt
    base-server)
  target_link_libraries(copy-logbook Qt::Widgets Qt::Xml)
  install(TARGETS copy-logbook DESTINATION ${BIN_INSTALL_DIR})
endif()

########### next target ###############
if(UNIX)
  set(compress_logbook_SOURCES compress-logbook.cpp)
  add_executable(compress-logbook
    ${elogbook_lib_SOURCES}
    ${compress_logbook_SOURCES})

  target_link_libraries(compress-logbook
    base
    base-qt
    base-server)
  target_link_libraries(compress-logbook Qt::Widgets Qt::Xml)
  install(TARGETS compress-logbook DESTINATION ${BIN_INSTALL_DIR})
endif()

########### next target ###############
if(UNIX)
  set(uncompress_logbook_SOURCES uncompress-logbook.cpp)
  add_executable(uncompress-logbook
    ${elogbook_lib_SOURCES}
    ${uncompress_logbook_SOURCES})

  target_link_libraries(uncompress-logbook
    base
    base-qt
    base-server)
  target_link_libraries(uncompress-logbook Qt::Widgets Qt::Xml)
  install(TARGETS uncompress-logbook DESTINATION ${BIN_INSTALL_DIR})
endif()

########### next target ###############
if(UNIX)
  set(synchronize_logbook_SOURCES synchronize-logbook.cpp)
  add_executable(synchronize-logbook
    ${elogbook_lib_SOURCES}
    ${synchronize_logbook_SOURCES})

  target_link_libraries(synchronize-logbook
    base
    base-qt
    base-server)
  target_link_libraries(synchronize-logbook Qt::Widgets Qt::Xml)
  install(TARGETS synchronize-logbook DESTINATION ${BIN_INSTALL_DIR})
endif()
