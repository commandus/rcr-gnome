set(GTK3_INCLUDE_DIRS 
	"/usr/include/gtkmm-3.0/"
	"/usr/include/gtkmm-3.0/gtk"
	"/usr/include/gtkmm-3.0/gdk"
)
#
# -------------------- Do not edit below this line --------------------
#

cmake_minimum_required(VERSION 3.9)
set(PROJECT_NAME "rcr-gnome")
project(${PROJECT_NAME} VERSION 1.0)

set(GEN ${CMAKE_CURRENT_SOURCE_DIR})
set(GEN_SRC ${GEN}/resources.c)

#
# Unicode toUpper()
#
find_package(ICU 61.0 COMPONENTS uc i18n REQUIRED)
set(ICU_LIBS ICU::uc ICU::i18n)

#
# GTK3
#
add_compile_options(-Wdeprecated-declarations)
find_package(PkgConfig)
pkg_check_modules(GTK gtk+-3.0 gtkmm-3.0)
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
link_libraries(${GTK_LIBRARIES})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# xlsx load
#
find_package(Xlnt CONFIG REQUIRED)
set(XLSX_LIBS xlnt::xlnt)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(RCR_INCLUDE_DIR "third-party/rcr")

#
# rcr-gnome
#
set(RCR_GNOME_SRC rcr-gnome.cpp rcr-app.cpp top-window.cpp
	third-party/rcr/string-helper.cpp third-party/rcr/MeasureUnit.cpp third-party/rcr/StockOperation.cpp third-party/rcr/QueryProperties.cpp
	${GEN_SRC}
)
add_executable(rcr-gnome ${RCR_GNOME_SRC})
target_link_libraries(rcr-gnome PRIVATE ${ICU_LIBS} ${GTK_LIBRARY_DIRS})
target_include_directories(rcr-gnome PRIVATE ${RCR_INCLUDE_DIR} ${GTK_INCLUDE_DIRS})

#
# Generate resource file
#
add_custom_command(
	OUTPUT ${GEN_SRC}
	COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}; glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/rcr-gnome.resource.xml --target=${GEN_SRC} --generate-source
)

INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)
