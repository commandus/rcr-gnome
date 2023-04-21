#
# -------------------- Do not edit below this line --------------------
#

cmake_minimum_required(VERSION 3.9)
set(PROJECT_NAME "rcr-gnome")
project(${PROJECT_NAME} VERSION 1.0)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GEN ${CMAKE_CURRENT_SOURCE_DIR})
set(GEN_SRC ${GEN}/resources.c)

#
# Setting up i18l & l10n
# see https://stackoverflow.com/questions/10521635/using-intltool-with-cmake
#
find_package(Gettext REQUIRED)

find_package (Intl REQUIRED)
include_directories(${Intl_INCLUDE_DIRS})
link_directories(${Intl_LIBRARY_DIRS})
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
IF(NOT GETTEXT_MSGFMT_EXECUTABLE)
	MESSAGE("NOTE: msgfmt not found. Translations will *not* be installed")
ELSE()
	# Build the .po files
	SET(PO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/po)
	# Input files
	FILE(GLOB PO_FILES ${PO_PATH}/*.po)
	# Generated file list
	SET(MO_FILES)
	SET(PRG rcr-gnome)
	SET(LOCALE_INSTALL_DIR /usr/share/locale)
	FOREACH(_poFile ${PO_FILES})
		GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
		STRING(REGEX REPLACE "^${PRG}\\." "" _langCode ${_poFileName} )
		STRING(REGEX REPLACE "\\.UTF-8\\.po$" "" _langCode ${_langCode} )
		STRING(REGEX REPLACE "_..$" "" _langCode ${_langCode} )
		IF( _langCode )
			MESSAGE("** PO file ${_poFileName} LANG ${_langCode} LIBDIR ${INTL_LIBRARY_DIRS} INCDIR ${INTL_INCLUDE_DIRS}")
			GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)

			SET(_moPath ${CMAKE_CURRENT_SOURCE_DIR}/locale/${_langCode}/LC_MESSAGES)
			SET(_moFile ${_moPath}/${PRG}.mo)
			MESSAGE("** ** ${_moFile} install to ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/${PRG}.mo")
			#	mkdir -p ${_moPath} ; ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_moFile} ${_poFile}
			MESSAGE("** ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}")

			ADD_CUSTOM_COMMAND(OUTPUT ${_moFile}
					COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}
					DEPENDS ${_poFile})
			#			INSTALL(FILES ${_moFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${PRG}.mo)
			LIST(APPEND MO_FILES ${_moFile})
		ENDIF()
	ENDFOREACH(_poFile ${PO_FILES})
	ADD_CUSTOM_TARGET(translations ALL DEPENDS ${MO_FILES})
ENDIF()

#
# GTK3, icu, xlsx, thread, Protobuf, gRPC
#
add_compile_options(-Wdeprecated-declarations)
find_package(PkgConfig)
pkg_check_modules(GTK gtk+-3.0 gtkmm-3.0)
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
link_libraries(${GTK_LIBRARIES})

find_package(ICU 61.0 COMPONENTS uc i18n REQUIRED)
set(ICU_LIBS ICU::uc ICU::i18n)

find_package(Xlnt CONFIG REQUIRED)
set(XLSX_LIBS xlnt::xlnt)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(RCR_INCLUDE_DIR "third-party/rcr" )

set(GEN ${RCR_INCLUDE_DIR}/gen)
set(GEN_GRPC_SRC ${GEN}/rcr.grpc.pb.cc ${GEN}/rcr.pb.cc)

#
# rcr-gnome
#
set(RCR_GNOME_SRC rcr-gnome.cpp rcr-app.cpp top-window.cpp
	third-party/config-filename.cpp third-party/utilfile.cpp third-party/utilstring.cpp
	third-party/rcr/string-helper.cpp third-party/rcr/MeasureUnit.cpp
	third-party/rcr/StockOperation.cpp
	third-party/rcr/QueryProperties.cpp
	third-party/rcr/RCQuery.cpp
	${GEN_GRPC_SRC}
	${GEN_SRC}
	GRcrClient.cpp GRcrClient.h RcrSettings.cpp CardWindow.cpp CardWindow.h BoxConfirmDialog.cpp BoxConfirmDialog.h LoginDialog.cpp LoginDialog.h RegisterDialog.cpp RegisterDialog.h UserListDialog.cpp UserListDialog.h UserDialog.cpp UserDialog.h StatisticsDialog.cpp StatisticsDialog.h rcr-gnome.h ServiceStateIntf.cpp ServiceStateIntf.h)
add_executable(rcr-gnome ${RCR_GNOME_SRC})
target_link_libraries(rcr-gnome PRIVATE ${ICU_LIBS} gRPC::grpc++ ${Protobuf_LIBRARIES} ${ICU_LIBS} ${XLSX_LIBS}
		${Intl_LIBRARY})
target_include_directories(rcr-gnome PRIVATE "third-party" ${RCR_INCLUDE_DIR} ${GTK_INCLUDE_DIRS})
target_compile_definitions(rcr-gnome PUBLIC RCR_VERSION=${CMAKE_PROJECT_VERSION})

#
# Generate resource file
#
add_custom_command(
	OUTPUT ${GEN_SRC}
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/build-resources.sh ${CMAKE_CURRENT_SOURCE_DIR}
)

INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)
