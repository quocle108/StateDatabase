file(GLOB UNIT_TESTS "*.cpp")
add_executable( db_test ${UNIT_TESTS} ../name.cpp ../eos_db.cpp )
target_link_libraries( db_test  fc chainbase Boost::unit_test_framework ${OPENSSL_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} )
