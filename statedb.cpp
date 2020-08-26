#include <iostream>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "db_viewer.hpp"
#include "eos_db.hpp"

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

template<typename T>
void test() {
    std::cout << "test type id: " << boost::core::demangle( typeid( T ).name() ) << std::endl;
}


int main(int argc, char **argv)
{
    config cfg;

    try
    {

        chainbase::database db(cfg.database_dir, database::read_write, cfg.cache_size);
        eos_db _eosdb(db);
        // db.add_index<table_id_multi_index>();
        // db.add_index<key_value_index>();
        // db.add_index<index64_index>();
        // db.add_index<index128_index>();
        // db.add_index<index256_index>();
        db.add_index<table_key_multi_index>();
      auto table = name("table");
      auto user_public_key = fc::crypto::private_key::generate().get_public_key();
      auto scope = fc::sha256::hash<>(user_public_key);
        db.create<table_key_object>([&](table_key_object &t_id) {
            t_id.id = scope;
            t_id.value.assign("bob's info", strlen("bob's info"));
        });

        const auto *existing_tid = db.find<table_key_object, by_key>(scope);
        std::cout << existing_tid->value << '\n';
        std::cout << existing_tid->id << '\n';
        // db.create<table_key_object>([&](table_key_object &t_id) {
        //     t_id.id = scope;
        //     t_id.value.assign("bob's info", strlen("bob's info"));
        // });

        // db.create<table_key_object>([&](table_key_object &t_id) {
        //     t_id.id = scope;
        //     t_id.value.assign("bob's info", strlen("bob's info"));
        // });
        // db.create<table_key_object>([&](table_key_object &t_id) {
        //     t_id.id = 1;
        //     t_id.value.assign("bob's info", strlen("bob's info"));
        // });

        // const auto *existing_tid = db.find<table_key_object, by_key>(1);
        // std::cout << existing_tid->value << '\n';
        // std::cout << existing_tid->id << '\n';
    // std::vector<string> v = {"apple", "facebook", "microsoft", "google"};
    //     std::cout << "value type: " << boost::core::demangle(typeid(decltype(v)::value_type).name()) << std::endl;
        // db.add_index<table_key_multi_index>();

        // auto receiver = name("receiver");
        // auto table = name("table");
        // auto user_public_key = fc::crypto::private_key::generate().get_public_key();
        // db.add_index<table_key_multi_index>();
        // auto scope = fc::sha256::hash<std::string>("scope");
        // auto scope1 = fc::sha256::hash<std::string>("scope1");
        // test<table_key_multi_index>();

        // table_key_multi_index table_key;
        // std::string value = "alice's info";
    //   value.assign("alice's info", strlen("alice's info"));
    //   const shared_blob& b = shared_blob(value.begin(), value.end(), allocator<value>);
        // shared_blob value("alice's info");
        
        // table_key.insert({1});

    //     auto &table_index = table_key.get<0>();
    //     auto it1 = table_index.find(1);
    //     std::cout << it1->id << '\n';
    //     table_index.modify(it1, [&](table_key_object &a) { a.id = 2; });
    // //     std::cout << it1->scope << '\n';

        // table_key.emplace([&](table_key_object &t_id) {
        //     t_id.scope = scope;
        //     t_id.value.assign( "alice's info", strlen("alice's info"));
        // });

        //  _eosdb.db_store_by_scope(receiver, scope, table, receiver, "100.0000 EOS", strlen("100.0000 EOS"));

        // db_viewer<table_key_multi_index>::walk(db, [](const auto &row) {
        //     std::cout << row.id << ": " << row.value << std::endl;
        // });

        // const auto &itr = _eosdb.find_or_create_table(receiver, scope, table, receiver);
        //  std::cout << itr.value << std::endl;

        //  _eosdb.db_update_by_scope(itr, receiver, "20.0000 EOS", strlen("100.0000 EOS"));

        // db_viewer<table_id_multi_index>::walk(db, [](const auto &row) {
        //     std::cout << row.id << ": " << row.payer << ": " << row.value << std::endl;
        // });

        // const auto *existing_tid = db.find<table_key_object, by_key>(scope);
        // if (existing_tid == nullptr)
        // {
        //     auto const &object = db.create<table_key_object>([&](table_key_object &t_id) {
        //         t_id.key = scope;
        //         t_id.value.assign( "alice's info", strlen("alice's info"));
        //     });
        //     const auto *existing_tid = db.find<table_key_object, by_key>(scope);
        //     std::cout << existing_tid->key << object.value << std::endl;
        // }
        // std::cout << itr.value << std::endl;
        // db_viewer<table_key_multi_index>::walk(db, [](const auto &row) {
        //     std::cout << row.key << ": " << row.value << std::endl;
        // });
    }
    catch (...)
    {
        std::cout << "Error insert"
                  << "\n";
        throw;
    }
    bfs::remove_all(cfg.database_dir);
    return 0;
}