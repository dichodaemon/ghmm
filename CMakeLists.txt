project( ghmm )

cmake_minimum_required( VERSION 2.6 )

set( PACKAGE_NAME ${PROJECT_NAME} )
set( PACKAGE_VERSION 0.0.1 )
set( PACKAGE_RELEASE 0 )
set( PACKAGE_MAINTAINER_NAME "Dizan Vasquez" )
set( PACKAGE_MAINTAINER_EMAIL "dichodaemon@gmail.com" )
set( PACKAGE_DESCRIPTION_SUMMARY "GHMM Library" )
set( PACKAGE_DESCRIPTION "Implementation of Growing Hidden Markov Models" )
set( PACKAGE_DEPENDS )
set( PACKAGE_BUILD_DEPENDS )

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/modules )
include( SimpleCMake )

sm_configure_project()
