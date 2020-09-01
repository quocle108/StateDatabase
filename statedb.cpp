#include <iostream>
#include <random>
#include "performance_test.hpp"

using namespace std;
using namespace chainbase;
using namespace boost::multi_index;

struct config
{
    boost::filesystem::path database_dir = "state";
    uint64_t cache_size = 720 * 1024 * 1024ll;
    chainbase::pinnable_mapped_file::map_mode db_map_mode = chainbase::pinnable_mapped_file::map_mode::mapped;
    vector<string> db_hugepage_paths;
};

void random_ids(std::vector<int> &ids)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(ids.begin(), ids.end(), std::default_random_engine(seed));
}

void printout(std::string func_name, uint64_t num_elements, measure<std::chrono::microseconds>::result result){
    std::cout << func_name << " - Elements:" << num_elements << " - Duration(us): " << result.duration.count() << " - Execution Time Unit(us): " << result.duration.count() / num_elements << " - Comsumed memmory(bytes): " << result.size << " - Comsumed memmory Unit(bytes): " << (double)result.size / num_elements << std::endl;
}

int main(int argc, char **argv)
{
    if (argc == 1)
        std::cout << "Please enter number elements" << std::endl;
    config cfg;    
    try
    {
        for (int i = 1; i < argc; i++)
        {
            chainbase::database db(cfg.database_dir, database::read_write, cfg.cache_size);
            db.add_index<table_id_multi_index>();
            db.add_index<key_value_index>();
            db.add_index<table_key_multi_index>();

            const uint64_t test_num = atoi(argv[i]);
            std::vector<int> ids;
            ids.reserve(test_num);
            for (int i = 0; i < test_num; ++i) ids.push_back(i);

            random_ids(ids);
            printout("STORE(ORDERED INDEX TABLE)", test_num, measure<std::chrono::microseconds>::execution(db_tests::store_by_table_ordered_id, db, ids));

            random_ids(ids);
            printout("UPDATE(ORDERED INDEX TABLE)", test_num, measure<std::chrono::microseconds>::execution(db_tests::update_by_table_ordered_id, db, ids));

            random_ids(ids);
            printout("REMOVE(ORDERED INDEX TABLE)", test_num, measure<std::chrono::microseconds>::execution(db_tests::remove_by_table_ordered_id, db, ids));
            
            random_ids(ids);
            printout("STORE(KEY VALUE TABLE)", test_num, measure<std::chrono::microseconds>::execution(db_tests::store_by_table_key_value, db, ids));

            random_ids(ids);
            printout("UPDATE(KEY VALUE TABLE)", test_num, measure<std::chrono::microseconds>::execution(db_tests::update_by_table_key_value, db, ids));

            random_ids(ids);
            printout("REMOVE(KEY VALUE TABLE)", test_num, measure<std::chrono::microseconds>::execution(db_tests::remove_by_table_key_value, db, ids));
            bfs::remove_all(cfg.database_dir);
        }
    }
    catch (...)
    {
        std::cout << "Error insert" << std::endl;
        bfs::remove_all(cfg.database_dir);
        throw;
    }
    bfs::remove_all(cfg.database_dir);
    return 0;
}