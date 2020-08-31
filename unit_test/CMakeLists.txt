file(GLOB UNIT_TESTS "*.cpp")
add_executable( test_db ${UNIT_TESTS} ../name.cpp ../eos_db.cpp )
target_link_libraries( test_db  fc chainbase Boost::unit_test_framework ${OPENSSL_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} )
