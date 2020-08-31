
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "db_viewer.hpp"
#include "eos_db.hpp"
#include <algorithm>
#include <random>

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

class db_tests
{
    public:
    static void store_by_table_key_value(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto scope = fc::sha256::hash<std::string>("scope");
        for (auto i = 0; i < ids.size(); i++)
        {
            _eosdb.db_store_by_key(scope, i, scope, "bob's info", strlen("bob's info"));
        }
    }

    static void update_by_table_key_value(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto scope = fc::sha256::hash<std::string>("scope");
        for (auto i = 0; i < ids.size(); i++)
        {
            _eosdb.db_update_by_key(scope, i, scope, "updated bob's info", strlen("updated bob's info"));
        }
    }

    static void remove_by_table_key_value(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto scope = fc::sha256::hash<std::string>("scope");
        for (auto i = 0; i < ids.size(); i++)
        {
            _eosdb.db_remove_by_key(scope, i);
        }
    }

    static void store_by_table_ordered_id(chainbase::database &db, const std::vector<int> &ids)
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

    static void update_by_table_ordered_id(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto receiver = name("receiver");
        auto table = name("table");
        auto scope = fc::sha256::hash<std::string>("scope");

        for (auto i = 0; i < ids.size(); i++)
        {
            auto itr = _eosdb.db_find_i64(receiver, scope, table, ids[i]);
            _eosdb.db_update_i64(itr, receiver, "updated bob's info", strlen("updated bob's info"));
        }
    }

    static void remove_by_table_ordered_id(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto receiver = name("receiver");
        auto table = name("table");
        auto scope = fc::sha256::hash<std::string>("scope");

        for (auto i = 0; i < ids.size(); i++)
        {
            auto itr = _eosdb.db_find_i64(receiver, scope, table, ids[i]);
            _eosdb.db_remove_i64(itr);
        }
    }
};