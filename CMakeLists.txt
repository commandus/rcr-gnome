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

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	find_program(POWERSHELL NAMES powershell)
endif()

#
# Setting up i18l & l10n
# see https://stackoverflow.com/questions/10521635/using-intltool-with-cmake
#
find_package(Gettext REQUIRED)

find_package (Intl REQUIRED)
include_directories(${Intl_INCLUDE_DIRS})
link_directories(${Intl_LIBRARY_DIRS})
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
	message("NOTE: msgfmt not found. Translations will *not* be installed")
else()
	# Build the .po files
	set(PO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/po)
	# Input files
	file(GLOB PO_FILES ${PO_PATH}/*.po)
	# Generated file list
	set(MO_FILES)
	set(PRG rcr-gnome)
	set(LOCALE_INSTALL_DIR /usr/share/locale)
	foreach(_poFile ${PO_FILES})
		get_filename_component(_poFileName ${_poFile} NAME)
		string(REGEX REPLACE "^${PRG}\\." "" _langCode ${_poFileName} )
		string(REGEX REPLACE "\\.UTF-8\\.po$" "" _langCode ${_langCode} )
		string(REGEX REPLACE "_..$" "" _langCode ${_langCode} )
		if( _langCode )
			message("** PO file ${_poFileName} LANG ${_langCode} LIBDIR ${INTL_LIBRARY_DIRS} INCDIR ${INTL_INCLUDE_DIRS}")
			get_filename_component(_lang ${_poFile} NAME_WE)

			set(_moPath ${CMAKE_CURRENT_SOURCE_DIR}/locale/${_langCode}/LC_MESSAGES)
			set(_moFile ${_moPath}/${PRG}.mo)
			#	mkdir -p ${_moPath} ; ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_moFile} ${_poFile}
			if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
				message("** powershell ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE} ${_moPath} ${_moFile} ${_poFile}")
				add_custom_command(OUTPUT ${_moFile}
					COMMAND ${POWERSHELL} ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.ps1 ${GETTEXT_MSGFMT_EXECUTABLE} ${_moPath} ${_moFile} ${_poFile}
					DEPENDS ${_poFile})
			else()
				message("** ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}")
				add_custom_command(OUTPUT ${_moFile}
					COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/mk_mo.sh ${_moPath} ${_moFile} ${_poFile}
					DEPENDS ${_poFile})
			endif()
			#			INSTALL(FILES ${_moFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${PRG}.mo)
			LIST(APPEND MO_FILES ${_moFile})
		endif ()
		endforeach()
	add_custom_target(translations ALL DEPENDS ${MO_FILES})
endif()

#
# GTK3, icu, xlsx, thread, Protobuf, gRPC
#

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(PKG_CONFIG_EXECUTABLE "/git/vcpkg/installed/x86-windows/tools/pkgconf/pkgconf.exe")
	find_package(PkgConfig)
	pkg_check_modules(GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-3.0)
	set(GTKMM_VARS PkgConfig::GTKMM_VARS)
else()	
	add_compile_options(-Wdeprecated-declarations)
	find_package(PkgConfig)
endif()


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
	GRcrClient.cpp RcrSettings.cpp CardWindow.cpp BoxConfirmDialog.cpp LoginDialog.cpp RegisterDialog.cpp
	UserListDialog.cpp UserDialog.cpp StatisticsDialog.cpp rcr-gnome.h ServiceStateIntf.cpp PropertyTypeDialog.cpp
	PropertyTypeEditDialog.cpp PropertyTypeEditDialog.cpp BoxDialog.cpp BoxDialog.h PropertyDialog.cpp PropertyDialog.h)
add_executable(rcr-gnome ${RCR_GNOME_SRC})
target_link_libraries(rcr-gnome PRIVATE ${ICU_LIBS} gRPC::grpc++ ${Protobuf_LIBRARIES} ${ICU_LIBS} ${XLSX_LIBS}
		${Intl_LIBRARY} ${GTKMM_VARS})
target_include_directories(rcr-gnome PRIVATE "third-party" ${RCR_INCLUDE_DIR} ${GTK_INCLUDE_DIRS})
target_compile_definitions(rcr-gnome PUBLIC RCR_VERSION=${CMAKE_PROJECT_VERSION})

#
# Generate resource file
#
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	find_program(GLIB_COMPILE_RESOURCES_EXECUTABLE msgfmt)
	if(NOT GLIB_COMPILE_RESOURCES_EXECUTABLE)
		message("NOTE: glib-compile-resources not found.")
	else()
		add_custom_command(OUTPUT ${GEN_SRC} 
			COMMAND ${POWERSHELL} ${CMAKE_CURRENT_SOURCE_DIR}/tools/build-resources.ps1 ${GLIB_COMPILE_RESOURCES_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}
		)
		message("** ${POWERSHELL} ${CMAKE_CURRENT_SOURCE_DIR}/tools/build-resources.ps1 ${GLIB_COMPILE_RESOURCES_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}")
	endif()
else()
	message("** ${CMAKE_CURRENT_SOURCE_DIR}/tools/build-resources.sh ${CMAKE_CURRENT_SOURCE_DIR}")
	add_custom_command(OUTPUT ${GEN_SRC}
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/build-resources.sh ${CMAKE_CURRENT_SOURCE_DIR}
	)
endif()


install(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)
