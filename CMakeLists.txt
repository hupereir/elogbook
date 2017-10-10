# $Id$
cmake_minimum_required(VERSION 2.8.12)

project(elogbook)
set(ELOGBOOK_VERSION_MAJOR 1)
SET(ELOGBOOK_VERSION_MINOR 12)
SET(ELOGBOOK_VERSION_PATCH 1)
set(ELOGBOOK_VERSION ${ELOGBOOK_VERSION_MAJOR}.${ELOGBOOK_VERSION_MINOR}.${ELOGBOOK_VERSION_PATCH})

########### modules #################
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/base-cmake")
include(Macros)
setup_cmake_settings()
setup_compiler_flags()

########### packages ###############
set(CPACK_PACKAGE_NAME "elogbook")
set(CPACK_PACKAGE_VENDOR "hugo.pereira@free.fr")
set(CPACK_PACKAGE_VERSION "${ELOGBOOK_VERSION}")
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
include(CPack)

########### configuration files ###############
configure_file(elogbook.spec.cmake ${CMAKE_SOURCE_DIR}/elogbook.spec)

########### aspell ###############
find_program(ASPELL aspell)
find_package(ASPELL)
if(ASPELL_FOUND)
  add_definitions(-DWITH_ASPELL=1)
else()
  add_definitions(-DWITH_ASPELL=0)
endif()

########### external applications ###############
find_program(XDG_OPEN xdg-open)

########### subdirectories ###############
if(ENABLE_SHARED)

  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  link_directories(${CMAKE_INSTALL_PREFIX}/lib)

else()

  add_subdirectory(base)
  add_subdirectory(base-qt)
  add_subdirectory(base-server)
  add_subdirectory(base-help)

  if(ASPELL_FOUND)
    add_subdirectory(base-spellcheck)
  endif(ASPELL_FOUND)

endif()

add_subdirectory(src)
