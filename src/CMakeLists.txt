#-------------------------------------------------------------------------------
# Library
#-------------------------------------------------------------------------------

sm_cpp_install_headers( ghmm )

#-------------------------------------------------------------------------------
# Tests
#-------------------------------------------------------------------------------

file( GLOB TEST_SRC test/*.cpp )

add_executable( unit_tests ${TEST_SRC} ) 

target_link_libraries( unit_tests unittest++ )

add_test( unit_tests ${EXECUTABLE_OUTPUT_PATH}/unit_tests xml )

