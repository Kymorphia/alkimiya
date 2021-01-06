
project (MarkdownBrowser C)
cmake_minimum_required (VERSION 3.0.2)
set (PACKAGE "markdown-browser")

# Options for the GNU C compiler only
if (CMAKE_COMPILER_IS_GNUCC)
  set (CMAKE_C_FLAGS_DEBUG "-Wall -g -DDEBUG")
  set (CMAKE_C_FLAGS_RELEASE "-Wall -O3")
endif (CMAKE_COMPILER_IS_GNUCC)

if (ENABLE_DEBUG)
  set (CMAKE_BUILD_TYPE "Debug" CACHE STRING
       "Choose the build type, options: Debug Release" FORCE)
  set (DEBUG 1)
else (ENABLE_DEBUG)
  set (CMAKE_BUILD_TYPE "Release" CACHE STRING
       "Choose the build type, options: Debug Release" FORCE)
  set (DEBUG 0)
endif (ENABLE_DEBUG)

# BIN_INSTALL_DIR - the directory where executables will be installed
set (BIN_INSTALL_DIR "bin" CACHE STRING "The install dir for executables")
mark_as_advanced (BIN_INSTALL_DIR)

include (FindPkgConfig)
pkg_check_modules (GTK REQUIRED gtk+-3.0>=3.22)

include_directories (
  .
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}
  ${GTK_INCLUDEDIR}
  ${GTK_INCLUDE_DIRS}
)

set (markdown_browser_HEADERS
  MarkdownBrowser.h
  MarkdownBrowserDialog.h
)

set (markdown_browser_SOURCES
  MarkdownBrowser.c
  MarkdownBrowserDialog.c
  main.c
)

link_directories (
  ${GTK_LIBDIR}
)

add_executable (markdown-browser
  ${markdown_browser_SOURCES}
  ${CMAKE_CURRENT_BINARY_DIR}/MarkdownBrowser-ui.h
)

target_link_libraries (markdown-browser
  ${GTK_LIBRARIES}
)

install (TARGETS markdown-browser
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
)

find_program (XXD xxd)

add_custom_command (
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MarkdownBrowser-ui.h
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${XXD} -i MarkdownBrowser.ui ${CMAKE_CURRENT_BINARY_DIR}/MarkdownBrowser-ui.h
    COMMENT "Converting Help.ui to C source code data"
)

