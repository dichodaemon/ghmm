#-------------------------------------------------------------------------------
# Library
#-------------------------------------------------------------------------------

#file( GLOB GHMM_SRC ghmm/*.cpp )

#add_library( ghmm SHARED ${GHMM_SRC} )

#sm_cpp_install( ghmm )

#-------------------------------------------------------------------------------
# Tests
#-------------------------------------------------------------------------------

file( GLOB TEST_SRC test/*.cpp )

add_executable( unit_tests ${TEST_SRC} ) 

target_link_libraries( unit_tests unittest++ )
#target_link_libraries( unit_tests ghmm unittest++ )

add_test( unit_tests ${EXECUTABLE_OUTPUT_PATH}/unit_tests xml )

file( GLOB ERASEME_SRC eraseme/*.cpp )

add_executable( eraseme ${ERASEME_SRC} ) 
