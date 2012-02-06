# DpkgDeb : Create debian packages for your projects and sub projects.
# Was heavily inspired by UseDebian (Mathieu Malaterre) and UseRPMTools (TSP Team) modules
#
# Written by Mehdi Rabah
# Modified by Dizan Vasquez form SimpleCMake
#
# Requires /usr/bin/dpkg
#
# USAGE : add_debian_package( [CONTROL_FILE] )
# You need to set the control file either by setting these variables
# or by giving as second parameter the path to the control file you want to use
# If you choose to set the variables, you must set the mandatory variables:
# (see man 5 deb-control for more details)
#
# -- MANDATORY Variables
# Package: ${PACKAGE_NAME}. Must be a lowercase string
# Version: ${PACKAGE_VERSION}. Like 1.1.0
# Release: ${PACKAGE_RELEASE}. A number beetween 0 and 9
# Maintainer: ${PACKAGE_MAINTAINER_NAME} and ${PACKAGE_MAINTAINER_EMAIL}.
# Description summary: ${PACKAGE_DESCRIPTION_SUMMARY}. Project summary
# Description : ${PACKAGE_DESCRIPTION}. Warning : for now, use of special characters (>, (, ', ... ) is not allowed
#
# -- OPTIONAL Variables
# Architecture: ${DEBIAN_ARCHITECTURE}, by default i386 for intel on debian like
# Depends: ${PACKAGE_DEPENDS}
# Section: ${PACKAGE_SECTION}
# Priority: ${PACKAGE_PRIORITY}
# Essential: ${PACKAGE_ESSENTIAL}
# Source: ${PACKAGE_SOURCE}
# Pre-Depends: ${PACKAGE_PREDEPENDS}
# Recommends: ${PACKAGE_RECOMMENDS}
# Suggests: ${PACKAGE_SUGGESTS}
# Breaks: ${PACKAGE_BREAKS}
# Conflicts: ${PACKAGE_CONFLICTS}
# Replaces: ${PACKAGE_REPLACES}
# Provides: ${PACKAGE_PROVIDES}

# TODO: Make clean does not clean the DEBIAN_DIR folder
# TODO: use objdump -p to automatically generate depedencies (cf dh_makeshlibs)

set(DPKGDEB_VERSION 1)
set(DPKGDEB_FOUND 1)

find_program(DPKG NAMES dpkg-deb PATHS "/usr/bin" )

if ( DPKG )
    get_filename_component(DPKG_PATH ${DPKG} ABSOLUTE)
    message(STATUS "Found dpkg-deb : ${DPKG_PATH}")
    set(DPKG_FOUND "YES")
    add_custom_target( deb )
else ( DPKG )
    message(STATUS "dpkg-deb NOT found. deb generation will not be available")
    set(DPKG_FOUND "NO")
endif ( DPKG )

#-------------------------------------------------------------------------------

#--
# Configures the debian packages for this project
#--
macro( ADD_DEBIAN_PACKAGE COMPONENT )
  # Check if the mandatory variables are here
  if (NOT PACKAGE_NAME OR NOT PACKAGE_VERSION OR NOT DEFINED PACKAGE_RELEASE OR
      NOT PACKAGE_MAINTAINER_NAME OR NOT PACKAGE_MAINTAINER_EMAIL OR
      NOT PACKAGE_DESCRIPTION_SUMMARY OR NOT PACKAGE_DESCRIPTION )
      message ( FATAL_ERROR "ADD_DEBIAN_PACKAGE command was not correctly configured for ${PACKAGE_NAME}. See the documentation for more details" )
  endif (NOT PACKAGE_NAME OR NOT PACKAGE_VERSION OR NOT DEFINED PACKAGE_RELEASE OR
      NOT PACKAGE_MAINTAINER_NAME OR NOT PACKAGE_MAINTAINER_EMAIL OR
      NOT PACKAGE_DESCRIPTION_SUMMARY OR NOT PACKAGE_DESCRIPTION )

  if ( NOT ${COMPONENT} STREQUAL "default" )
    set( PACKAGE_NAME ${PACKAGE_NAME}-${COMPONENT} )
  endif ( NOT ${COMPONENT} STREQUAL "default" )
  set( CONTROL_FILE ${PROJECT_BINARY_DIR}/control_${PACKAGE_NAME} )

  if (NOT DEBIAN_ARCHITECTURE)
    set (DEBIAN_ARCHITECTURE i386) #dpkg --print-architecture is always i386 on intel
  endif (NOT DEBIAN_ARCHITECTURE)

  # Writing the control file
  # see man 5 deb-control for more details
  add_custom_command (
    OUTPUT ${CONTROL_FILE}
    COMMAND   ${CMAKE_COMMAND} -E echo
      "Package: ${PACKAGE_NAME}" > ${CONTROL_FILE}

    COMMAND   ${CMAKE_COMMAND} -E echo
      "Version: ${PACKAGE_VERSION}" >> ${CONTROL_FILE}

    COMMAND   ${CMAKE_COMMAND} -E echo
      "Maintainer: ${PACKAGE_MAINTAINER_NAME}<${PACKAGE_MAINTAINER_EMAIL}>" >> ${CONTROL_FILE}

    COMMAND   ${CMAKE_COMMAND}
    ARGS      -E echo "Architecture: ${DEBIAN_ARCHITECTURE}" >> ${CONTROL_FILE}
    VERBATIM
  )

  if ( DEFINED PACKAGE_DEPENDS )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Depends: ${PACKAGE_DEPENDS}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_DEPENDS )

  if ( DEFINED PACKAGE_SECTION )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Section: ${PACKAGE_SECTION}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_SECTION)

  if ( DEFINED PACKAGE_PRIORITY )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Priority: ${PACKAGE_PRIORITY}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_PRIORITY)

  if ( DEFINED PACKAGE_ESSENTIAL )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Essential: ${PACKAGE_ESSENTIAL}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_ESSENTIAL)

  if ( DEFINED PACKAGE_SOURCE )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Source: ${PACKAGE_SOURCE}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_SOURCE)

  if ( DEFINED PACKAGE_PREDEPENDS )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Pre-Depends: ${PACKAGE_PREDEPENDS}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_PREDEPENDS )

  if ( DEFINED PACKAGE_RECOMMENDS )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Recommends: ${PACKAGE_RECOMMENDS}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_RECOMMENDS)

  if ( DEFINED PACKAGE_SUGGESTS )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Suggests: ${PACKAGE_SUGGESTS}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_SUGGESTS )

  if ( DEFINED PACKAGE_BREAKS )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Breaks: ${PACKAGE_BREAKS}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_BREAKS )

  if ( DEFINED PACKAGE_CONFLICTS )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Conflicts: ${PACKAGE_CONFLICTS}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_CONFLICTS )

  if ( DEFINED PACKAGE_REPLACES )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Replaces: ${PACKAGE_REPLACES}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_REPLACES )

  if ( DEFINED PACKAGE_PROVIDES )
    add_custom_command( OUTPUT ${CONTROL_FILE}
      COMMAND   ${CMAKE_COMMAND} -E echo
        "Provides: ${PACKAGE_PROVIDES}" >> ${CONTROL_FILE}
      APPEND
      VERBATIM
    )
  endif ( DEFINED PACKAGE_PROVIDES )

  add_custom_command( OUTPUT ${CONTROL_FILE}
    COMMAND   ${CMAKE_COMMAND} -E echo
      "Description: ${PACKAGE_DESCRIPTION_SUMMARY}" >> ${CONTROL_FILE}
    COMMAND   ${CMAKE_COMMAND} -E echo
      " ." >> ${CONTROL_FILE}
    COMMAND   ${CMAKE_COMMAND} -E echo
      " ${PACKAGE_DESCRIPTION}" >> ${CONTROL_FILE}
    COMMAND   ${CMAKE_COMMAND} -E echo
      " ." >> ${CONTROL_FILE}
    COMMAND   ${CMAKE_COMMAND} -E echo
      "" >> ${CONTROL_FILE}
    COMMENT   "Generating control file"
    APPEND
    VERBATIM
  )
  generate_debian_package( ${COMPONENT} )
endmacro(ADD_DEBIAN_PACKAGE)

#-------------------------------------------------------------------------------

#--
# Generates a debian package for a given component
#--
macro( GENERATE_DEBIAN_PACKAGE COMPONENT )
#   if ( ${COMPONENT} STREQUAL default )
    set ( DEBIAN_PREFIX  ${PACKAGE_NAME}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}_${DEBIAN_ARCHITECTURE} )
#   else ( ${COMPONENT} STREQUAL default )
#     set ( DEBIAN_PREFIX  ${PACKAGE_NAME}-${COMPONENT}_${PACKAGE_VERSION}-${PACKAGE_RELEASE}_${DEBIAN_ARCHITECTURE} )
#   endif ( ${COMPONENT} STREQUAL default )

  message(STATUS "Adding package: ${DEBIAN_PREFIX}")

  set ( DEBIAN_DIR ${CMAKE_BINARY_DIR}/${DEBIAN_PREFIX})
  file ( REMOVE ${DEBIAN_DIR} )
  set ( DEBIAN_CLEAN_FILES ${DEBIAN_CLEAN_FILES} ${DEBIAN_PREFIX}.deb )
  set_directory_properties( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${DEBIAN_CLEAN_FILES}" )

  add_custom_target( remove_control_${COMPONENT}
    COMMAND ${CMAKE_COMMAND} -E remove ${CONTROL_FILE}
    COMMAND rm -fdr ${DEBIAN_DIR}
  )
  add_custom_target( deb_dir_${COMPONENT}
    COMMAND ${CMAKE_MAKE_PROGRAM}
    COMMAND ${CMAKE_MAKE_PROGRAM} preinstall
    COMMAND ${CMAKE_COMMAND} -DCOMPONENT=${COMPONENT} -DCMAKE_INSTALL_PREFIX=${DEBIAN_DIR}/${CMAKE_INSTALL_PREFIX} -P cmake_install.cmake
    COMMENT "Creating dir for ${COMPONENT} in = ${DEBIAN_DIR}"
  )
  add_custom_target( deb_${COMPONENT}
    COMMAND ${CMAKE_COMMAND} -E copy ${CONTROL_FILE} ${DEBIAN_DIR}/DEBIAN/control
    COMMAND fakeroot ${DPKG_PATH} --build ${DEBIAN_DIR}
    DEPENDS ${CONTROL_FILE}
    COMMENT   "Generating debian package"
  )
  add_dependencies( deb_dir_${COMPONENT} remove_control_${COMPONENT} )
  add_dependencies( deb_${COMPONENT} deb_dir_${COMPONENT} )
  add_dependencies( deb deb_${COMPONENT} )
endmacro( GENERATE_DEBIAN_PACKAGE COMPONENT )
