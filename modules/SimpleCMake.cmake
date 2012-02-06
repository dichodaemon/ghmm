# A utility to simplify the creation of CMake projects.

set( SIMPLE_CMAKE_VERSION 1 )
set( SIMPLE_CMAKE_FOUND   1 )
cmake_policy( SET CMP0011 OLD )
cmake_policy( SET CMP0002 OLD )
cmake_policy( SET CMP0014 OLD )

#-------------------------------------------------------------------------------

#--
# Glob that filters out hidden files
#--
macro( SM_GLOB VARIABLE_NAME GLOB_EXPRESSION )
  file( GLOB ${VARIABLE_NAME} ${GLOB_EXPRESSION} )
  foreach( FILE_NAME ${${VARIABLE_NAME}} )
    string( REGEX MATCH "^.*/[.].*$" REGEX_MATCHED ${FILE_NAME} )
    if ( REGEX_MATCHED )
      list( REMOVE_ITEM ${VARIABLE_NAME} ${FILE_NAME} )
    endif ( REGEX_MATCHED )
  endforeach( FILE_NAME ${VARIABLE_NAME} )
endmacro( SM_GLOB VARIABLE_NAME GLOB_EXPRESSION )

#-------------------------------------------------------------------------------

#--
# Configures c/c++
#--
macro( SM_SRC )
  file( GLOB SM_SRC_EXISTS src )
  if ( SM_SRC_EXISTS )
    set( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )
    set( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )
    include_directories( ${PROJECT_SOURCE_DIR}/src )
    add_subdirectory( src )
  endif ( SM_SRC_EXISTS )
endmacro( SM_SRC )

#-------------------------------------------------------------------------------

#--
# Configures Doxygen Documentation
#--
macro( SM_DOCUMENTATION )
  file( GLOB SM_DOC_EXISTS doc )
  if ( SM_DOC_EXISTS )
    add_subdirectory( doc )
    find_package( Doxygen )
    if ( DOXYGEN_FOUND )
      message( STATUS "Added documentation task" )
      configure_file( doc/Doxyfile.in doc/Doxyfile )
      add_custom_target(
        doc ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY doc
      )
    else ( DOXYGEN_FOUND )
      message( STATUS "Doxygen not found, documentation task won't be available" )
    endif ( DOXYGEN_FOUND )
  endif ( SM_DOC_EXISTS )
endmacro( SM_DOCUMENTATION )

#-------------------------------------------------------------------------------

#--
# Configures script installation (backwards compatibility, use SM_BIN instead)
#--
macro( SM_SCRIPTS )
  file( GLOB SM_SCRIPTS_EXISTS scripts )
  if ( SM_SCRIPTS_EXISTS )
    sm_glob( SCRIPTS scripts/* )
    install(
      PROGRAMS ${SCRIPTS}
      DESTINATION bin
      COMPONENT default
    )
  endif ( SM_SCRIPTS_EXISTS )
endmacro( SM_SCRIPTS )

#-------------------------------------------------------------------------------

#--
# Configures bin installation
#--
macro( SM_BIN )
  file( GLOB SM_BIN_EXISTS bin )
  if ( SM_BIN_EXISTS )
    sm_glob( BINS bin/* )
    install(
      PROGRAMS ${BINS}
      DESTINATION bin
      COMPONENT default
    )
  endif ( SM_BIN_EXISTS )
endmacro( SM_BIN )

#-------------------------------------------------------------------------------

#--
# Generates Debian Packages
#--
macro( SM_DEBIAN_PACKAGES )
  include( DpkgDeb OPTIONAL )
  if ( DPKG_FOUND )
    message( STATUS "Debian packages will install to '${CMAKE_INSTALL_PREFIX}'" )
    message( STATUS "To change this, modify CMAKE_INSTALL_PREFIX in the cache" )
  else ( DPKG_FOUND )
    message( STATUS "DpkgDeb is not installed. Cannot build Debian packages" )
  endif ( DPKG_FOUND )
endmacro( SM_DEBIAN_PACKAGES )

#-------------------------------------------------------------------------------

#--
# Configures the whole project according to conventions
#--
macro( SM_CONFIGURE_PROJECT )
  if( CMAKE_BUILD_TOOL MATCHES "make" )
    string( REGEX MATCH "arm-linux" IS_ARM_LINUX  ${CMAKE_CXX_COMPILER} )
    message( STATUS "Compiler: ${IS_ARM_LINUX}" )
    if( IS_ARM_LINUX )
      set( CMAKE_CXX_FLAGS "-Wall -Wnon-virtual-dtor" ) 
    else( IS_ARM_LINUX )
      set( CMAKE_CXX_FLAGS "-Wall -Wextra -Wnon-virtual-dtor" )     
      #set( CMAKE_CXX_FLAGS "-Wall -Wextra -Wnon-virtual-dtor -Weffc++" ) 
    endif( IS_ARM_LINUX )
  endif( CMAKE_BUILD_TOOL MATCHES "make" )
  
  enable_testing()

  if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    set( CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "SCMake install prefix" FORCE )
  endif( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

  if( NOT CMAKE_MODULE_PATH )
    set( CMAKE_MODULE_PATH $ENV{HOME}/cmake CACHE PATH "Path for extra cmake modules" )
  endif( NOT CMAKE_MODULE_PATH )
  string( REGEX REPLACE "[^./]+" ".." CMAKE_INSTALL_ROOT ${CMAKE_INSTALL_PREFIX} )
  set( CMAKE_INSTALL_ROOT ./${CMAKE_INSTALL_ROOT} )

  sm_debian_packages()

  sm_src()
  sm_documentation()
  sm_scripts()
  sm_bin()
  sm_python()

  if ( DPKGDEB_FOUND )
    add_debian_package( default )
  endif ( DPKGDEB_FOUND )
endmacro( SM_CONFIGURE_PROJECT )

#-------------------------------------------------------------------------------

#--
# Configures C++/C target installation
#--
macro( SM_CPP_INSTALL  )
  sm_cpp_install_binaries( ${ARGV} )
  sm_cpp_install_headers( ${ARGV} )
endmacro( SM_CPP_INSTALL INSTALL_TARGETS )

#-------------------------------------------------------------------------------

#--
# Install executables and library files
#--

macro( SM_CPP_INSTALL_BINARIES ) 
  message( STATUS "Library names:" ${ARGV} )
  install(
    TARGETS ${ARGV}
    RUNTIME DESTINATION bin COMPONENT default
    LIBRARY DESTINATION lib COMPONENT default
    ARCHIVE DESTINATION lib COMPONENT default
  )
endmacro( SM_CPP_INSTALL_BINARIES )

#-------------------------------------------------------------------------------

#--
# Install header files
#--

macro( SM_CPP_INSTALL_HEADERS  )
  message( STATUS "Header files:" ${ARGV} )
  foreach( DIR_NAME ${ARGV} )
    file( GLOB HEADER_FILES ${DIR_NAME}/*.h ${DIR_NAME}/*.hpp ${DIR_NAME}/*.tpp ${DIR_NAME}/*.I )

    if ( HEADER_FILES )
      install(
        FILES ${HEADER_FILES}
        DESTINATION include/${DIR_NAME}
        COMPONENT dev
      )
      if ( DPKGDEB_FOUND )
        if ( NOT DEFINED DEV_PACKAGE_ADDED )
          set( DEV_PACKAGE_ADDED 1 )
          add_debian_package( dev )
        endif ( NOT DEFINED DEV_PACKAGE_ADDED )
      endif ( DPKGDEB_FOUND )
    endif ( HEADER_FILES )
  endforeach( DIR_NAME ${INSTALL_TARGETS} )
endmacro( SM_CPP_INSTALL_HEADERS )

#-------------------------------------------------------------------------------

#--
# Finds external header files
#--

macro( SM_CPP_INCLUDE_DIR LIBRARY_NAME HEADER_DIR )
  string( TOUPPER ${LIBRARY_NAME} ${LIBRARY_NAME} )
  if( ${ARGC} GREATER 2 )
    set( PARENT "/../" )
  else( ${ARGC} GREATER 2 )
    set( PARENT "" )
  endif( ${ARGC} GREATER 2 )
  if ( NOT ${LIBRARY_NAME}_INCLUDE_PATH )
    find_file( ${LIBRARY_NAME}_INCLUDE_PATH ${HEADER_DIR}/  PATHS /usr/include / )
    if ( ${LIBRARY_NAME}_INCLUDE_PATH )
      set ( ${LIBRARY_NAME}_INCLUDE_PATH ${${LIBRARY_NAME}_INCLUDE_PATH}${PARENT} CACHE PATH "" FORCE )
    endif ( ${LIBRARY_NAME}_INCLUDE_PATH )
  endif ( NOT ${LIBRARY_NAME}_INCLUDE_PATH )
  include_directories( ${${LIBRARY_NAME}_INCLUDE_PATH} )
  message( STATUS  "${LIBRARY_NAME}_INCLUDE_PATH=${${LIBRARY_NAME}_INCLUDE_PATH}" )
endmacro( SM_CPP_INCLUDE_DIR LIBRARY_NAME HEADER_DIR )

#-------------------------------------------------------------------------------

#--
# Install python files
#--

macro( SM_PYTHON )
  find_path( 
    PYTHON_SITE_PACKAGES_PATH .
    PATHS /usr/lib
    PATH_SUFFIXES 
      python2.6/dist-packages
      python2.5/site-packages
      python2.4/site-packages
    NO_DEFAULT_PATH
  )  
  
  file( GLOB SM_PYTHON_EXISTS python )
  if ( SM_PYTHON_EXISTS )
    install(
      DIRECTORY python
      DESTINATION share/${PROJECT_NAME}/
      COMPONENT default
      PATTERN .* EXCLUDE
      PATTERN *.pyc EXCLUDE
    )

    set( PYTHON_PATH_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pth )
    
    file( WRITE ${PYTHON_PATH_FILE} /usr/share/${PROJECT_NAME}/lib\n ) 
    file( APPEND ${PYTHON_PATH_FILE} /usr/share/${PROJECT_NAME}/python ) 
    
    install(
      FILES ${PYTHON_PATH_FILE}
      DESTINATION ${CMAKE_INSTALL_ROOT}/${PYTHON_SITE_PACKAGES_PATH}
      COMPONENT default
    )
  endif ( SM_PYTHON_EXISTS )
endmacro( SM_PYTHON )
