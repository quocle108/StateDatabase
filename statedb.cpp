#include <iostream>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "db_viewer.hpp"
#include "eos_db.hpp"
#include <algorithm>
#include <random>

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

template <typename TimeT = std::chrono::milliseconds>
struct measure
{
    template <typename F, typename... Args>
    static typename TimeT::rep execution(F func, Args &&... args)
    {
        auto start = std::chrono::high_resolution_clock::now();
        func(std::forward<Args>(args)...);
        auto duration = std::chrono::duration_cast<TimeT>(std::chrono::high_resolution_clock::now() - start);
        return duration.count();
    }
};

void store_by_table_key_value(chainbase::database &db, const std::vector<int> &ids)
{
    eos_db _eosdb(db);
    auto scope = fc::sha256::hash<std::string>("scope");
    for (auto i = 0; i < ids.size(); i++)
    {
        _eosdb.db_store_by_key(scope, i, scope, "bob's info", strlen("bob's info"));
    }
}

void store_by_table_ordered_id(chainbase::database &db, const std::vector<int> &ids)
{
    eos_db _eosdb(db);
    auto receiver = name("receiver");
    auto table = name("table");
    auto scope = fc::sha256::hash<std::string>("scope");

    _eosdb.find_or_create_table(receiver, scope, table, receiver);
    for (auto i = 0; i < ids.size(); i++)
    {
        _eosdb.db_store_i64(receiver, scope, table, receiver, ids[i], "bob's info", strlen("bob's info"));
    }
}

void create_random_id(std::vector<int> &unique_ids, uint64_t num)
{
    unique_ids.reserve(num);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    for (int i = 0; i < num; ++i)
        unique_ids.push_back(i);
    std::shuffle(unique_ids.begin(), unique_ids.end(), std::default_random_engine(seed));
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
            db.add_index<index64_index>();
            db.add_index<index128_index>();
            db.add_index<index256_index>();
            db.add_index<table_key_multi_index>();
            std::vector<int> random_ids;
            const int test_num = atoi(argv[i]);
            create_random_id(random_ids, test_num);

            auto t = measure<std::chrono::nanoseconds>::execution(store_by_table_ordered_id, db, random_ids);
            std::cout << "ORDERED INDEX TABLE" << " - Total Elements:" << db_viewer<key_value_index>::size(db) << " - Duration: " << t << " - Execution Time Unit: " << t / test_num << std::endl;

            auto t1 = measure<std::chrono::nanoseconds>::execution(store_by_table_key_value, db, random_ids);
            std::cout << "KEY VALUE TABLE" << " - Total Elements:" << db_viewer<table_key_multi_index>::size(db) << " - Duration: " << t1 << " - Execution Time Unit: " << t1 / test_num << std::endl;
            
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