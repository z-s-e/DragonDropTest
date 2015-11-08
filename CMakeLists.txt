cmake_minimum_required(VERSION 3.1)
project(DragonDropTest)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(DRAGONDROPTEST_VERSION_MAJOR "0")
set(DRAGONDROPTEST_VERSION_MINOR "1")
set(DRAGONDROPTEST_VERSION_PATCH "0")
set(DRAGONDROPTEST_VERSION_STRING "${DRAGONDROPTEST_VERSION_MAJOR}.${DRAGONDROPTEST_VERSION_MINOR}.${DRAGONDROPTEST_VERSION_PATCH}")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

set(dragondroptest_src
    src/dndaction.cpp
    src/dragsource.cpp
    src/droparea.cpp
    src/main.cpp
    src/widget.cpp
    src/widget.ui
)


option(
  USE_BUNDLED_FORMGENWIDGETS
  "Use bundled FormGenWidgets-Qt"
  ON
)

if(USE_BUNDLED_FORMGENWIDGETS)
  add_subdirectory(lib/FormGenWidgets-Qt)
else()
  find_package(FormGenWidgets-Qt)
endif()
find_package(Qt5 COMPONENTS Widgets NO_MODULE REQUIRED)


add_executable(DragonDropTest ${dragondroptest_src})
set(CXX_STANDARD_REQUIRED ON)
set_property(TARGET DragonDropTest PROPERTY CXX_STANDARD 11)
target_include_directories(DragonDropTest PRIVATE src)
target_link_libraries(DragonDropTest FormGenWidgets-Qt Qt5::Widgets)

