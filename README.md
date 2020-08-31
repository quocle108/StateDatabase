# StateDatabase - a new version support key-value type
- Simple and faster table allow user able to store/get/update/remove data by key-value

## Dependencies

  - C++17
  - [Boost](http://www.boost.org/)
  - CMake Build Process


# Build

```
$ mkdir build; cd build/; cmake..
$ make

# run main program ./statedb test_elements test_elements test_elements ...
$ ./statedb 10000 50000 100000 500000 1000000

# run unit test
$ ./unit_test/db_test
```
