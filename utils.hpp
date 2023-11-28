#ifndef INCLUDE_UTILS_HPP_
#define INCLUDE_UTILS_HPP_

#include <boost/interprocess/managed_external_buffer.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/io_service.hpp>

#include <string>
#include <iostream>
#include <cinttypes>
#include <inttypes.h>

namespace bfs = boost::filesystem;

constexpr size_t DIM(const auto& x) {
    return sizeof(x) / sizeof(*x);
}

static const char* sizes[] = { "EiB", "PiB", "TiB", "GiB", "MiB", "KiB", "B" };
static const uint64_t exbibytes = 1024ULL * 1024ULL * 1024ULL *
                                  1024ULL * 1024ULL * 1024ULL;

std::string calculateSize(uint64_t size) {
    std::string result(20, '\0');
    uint64_t multiplier = exbibytes;

    for (int i = 0; i < DIM(sizes); ++i, multiplier /= 1024) {
        if (size < multiplier) continue;

        if (size % multiplier == 0)
            snprintf(&result[0], 20, "%" PRIu64 " %s", size / multiplier, sizes[i]);
        else
            snprintf(&result[0], 20, "%.1f %s", static_cast<float>(size) / multiplier, sizes[i]);
        
        return result;
    }

    strcpy(&result[0], "0");
    return result;
}

void printDatabase(const chainbase::database &db, boost::filesystem::path path){
   auto _data_file_path(bfs::absolute(path / "shared_memory.bin"));
   auto existing_file_size = bfs::file_size(_data_file_path);
   auto free_memory = db.get_segment_manager()->get_free_memory();
   std::cerr << "***********Total memory   :   " << calculateSize(existing_file_size) << " \n";
   std::cerr << "***********Free memory    :   " << calculateSize(free_memory) << " \n";
   std::cerr << "***********Used memory    :   " << calculateSize(existing_file_size - free_memory) << " \n";
}


#endif  // INCLUDE_UTILS_HPP_"