CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(setting-myaccount-efl C)

SET(APPDIR /usr/apps/setting-myaccount-efl)
SET(LIBDIR ${APPDIR}/lib/ug)
SET(BINDIR ${APPDIR}/bin)
SET(RESDIR ${APPDIR}/res)
SET(DATADIR ${APPDIR}/shared/trusted)



SET(ACCOUNT_LIST_UG_SRCS
src/myaccount_ug_main.c
src/myaccount_ug_common.c
src/myaccount_ug_account_list.c
src/myaccount_ug_addaccount.c
)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(/usr/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	appcore-efl
	ui-gadget-1
	accounts-svc
	capi-appfw-application
	vconf
	pkgmgr
	efl-extension
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Wall -Werror -fpie")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-D_A_PROJECT_")

IF(_FEATURE_SMEMO_ENABLE)
ADD_DEFINITIONS(-D_FEATURE_SMEMO_ENABLE)
ENDIF(_FEATURE_SMEMO_ENABLE)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${ACCOUNT_LIST_UG_SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.0.1)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} accounts-svc)

ADD_CUSTOM_TARGET(myaccount.edj
		COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/data/myaccount_photo
		${CMAKE_SOURCE_DIR}/data/edc/myaccount.edc ${CMAKE_BINARY_DIR}/myaccount.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/data/edc/myaccount.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} myaccount.edj)

SET(COLORTABLEDIR "${CMAKE_BINARY_DIR}/")

ADD_CUSTOM_TARGET(myaccount_edc_images.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/myaccount_photo
		${CMAKE_SOURCE_DIR}/data/edc/myaccount_edc_images.edc ${CMAKE_BINARY_DIR}/myaccount_edc_images.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/data/edc/myaccount_edc_image_macro.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} myaccount_edc_images.edj)

INSTALL(FILES ${CMAKE_BINARY_DIR}/setting-myaccount-efl.xml DESTINATION /usr/share/packages)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/myaccount.edj DESTINATION ${RESDIR}/edje/${PROJECT_NAME})
INSTALL(FILES ${CMAKE_BINARY_DIR}/myaccount_edc_images.edj DESTINATION ${RESDIR}/edje/${PROJECT_NAME})

# i18n
ADD_SUBDIRECTORY(po)
