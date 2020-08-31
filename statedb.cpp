#include <iostream>
#include "perfomance_tests.hpp"

using namespace std;
using namespace chainbase;
using namespace boost::multi_index;

struct config
{
    boost::filesystem::path database_dir = "state";
    uint64_t cache_size = 340 * 1024 * 1024ll;
    chainbase::pinnable_mapped_file::map_mode db_map_mode = chainbase::pinnable_mapped_file::map_mode::mapped;
    vector<string> db_hugepage_paths;
};

void random_ids(std::vector<int> &ids)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(ids.begin(), ids.end(), std::default_random_engine(seed));
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

            const int test_num = atoi(argv[i]);
            std::vector<int> ids;
            ids.reserve(test_num);
            for (int i = 0; i < test_num; ++i) ids.push_back(i);

            std::cout << "ORDERED INDEX TABLE" << " - Total Elements: " << test_num << std::endl;

            random_ids(ids);
            auto t = measure<std::chrono::nanoseconds>::execution(db_tests::store_by_table_ordered_id, db, ids);
            std::cout << "STORE" << " - Elements:" << db_viewer<key_value_index>::size(db) << " - Duration: " << t << " - Execution Time Unit: " << t / test_num << std::endl;

            random_ids(ids);
            auto t1 = measure<std::chrono::nanoseconds>::execution(db_tests::update_by_table_ordered_id, db, ids);
            std::cout << "UPDATE" << " - Elements:" << db_viewer<key_value_index>::size(db) << " - Duration: " << t1 << " - Execution Time Unit: " << t1 / test_num << std::endl;

            random_ids(ids);
            auto t2 = measure<std::chrono::nanoseconds>::execution(db_tests::remove_by_table_ordered_id, db, ids);
            std::cout << "REMOVE" << " - Elements:" << db_viewer<key_value_index>::size(db) << " - Duration: " << t2 << " - Execution Time Unit: " << t2 / test_num << std::endl << std::endl;

            std::cout << "KEY VALUE TABLE" << " - Total Elements: " << test_num << std::endl;
            
            random_ids(ids);
            auto t3 = measure<std::chrono::nanoseconds>::execution(db_tests::store_by_table_key_value, db, ids);
            std::cout << "STORE" << " - Elements:" << db_viewer<table_key_multi_index>::size(db) << " - Duration: " << t3 << " - Execution Time Unit: " << t3 / test_num << std::endl;
            
            random_ids(ids);
            auto t4 = measure<std::chrono::nanoseconds>::execution(db_tests::update_by_table_key_value, db, ids);
            std::cout << "UPDATE" << " - Elements:" << db_viewer<table_key_multi_index>::size(db) << " - Duration: " << t4 << " - Execution Time Unit: " << t4 / test_num << std::endl;
            
            random_ids(ids);
            auto t5 = measure<std::chrono::nanoseconds>::execution(db_tests::remove_by_table_key_value, db, ids);
            std::cout << "REMOVE" << " - Elements:" << db_viewer<table_key_multi_index>::size(db) << " - Duration: " << t5 << " - Execution Time Unit: " << t5 / test_num << std::endl;

            std::cout << std::endl;
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