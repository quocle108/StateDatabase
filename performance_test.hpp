
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include "db_viewer.hpp"
#include "eos_db.hpp"
#include <algorithm>

inline constexpr char stored_data[]  = "store my msg to db";
inline constexpr char updated_data[]  = "update my stored msg to db";

template <typename TimeT = std::chrono::milliseconds>
struct measure
{
    struct result{
        int64_t size;
        TimeT duration;
    };

    template <typename F, typename... Args>
    static typename measure::result execution(F func, Args &&... args)
    {
        result _result;
        auto start = std::chrono::high_resolution_clock::now();
        _result.size = func(std::forward<Args>(args)...);
        _result.duration = std::chrono::duration_cast<TimeT>(std::chrono::high_resolution_clock::now() - start);
        return _result;
    }
};

class db_tests
{
    public:
    static int64_t store_by_table_key_value(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto scope = fc::sha256::hash<std::string>("scope");
        int64_t start_size = db.get_free_memory();
        for (auto i = 0; i < ids.size(); i++)
        {
            _eosdb.db_store_by_key(scope, i, scope, stored_data, strlen(stored_data));
        }
        return start_size - db.get_free_memory();
    }

    static int64_t update_by_table_key_value(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto scope = fc::sha256::hash<std::string>("scope");

        int64_t start_size = db.get_free_memory();
        for (auto i = 0; i < ids.size(); i++)
        {
            _eosdb.db_update_by_key(scope, i, scope, updated_data, strlen(updated_data));
        }
        return start_size - db.get_free_memory();
    }

    static int64_t remove_by_table_key_value(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto scope = fc::sha256::hash<std::string>("scope");

        int64_t start_size = db.get_free_memory();
        for (auto i = 0; i < ids.size(); i++)
        {
            _eosdb.db_remove_by_key(scope, i);
        }
        return start_size - db.get_free_memory();
    }

    static int64_t store_by_table_ordered_id(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto receiver = name("receiver");
        auto table = name("table");
        auto scope = fc::sha256::hash<std::string>("scope");

        int64_t start_size = db.get_free_memory();
        _eosdb.find_or_create_table(receiver, scope, table, receiver);
        for (auto i = 0; i < ids.size(); i++)
        {
            _eosdb.db_store_i64(receiver, scope, table, receiver, ids[i], stored_data, strlen(stored_data));
        }
        return start_size - db.get_free_memory();
    }

    static int64_t update_by_table_ordered_id(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto receiver = name("receiver");
        auto table = name("table");
        auto scope = fc::sha256::hash<std::string>("scope");

        int64_t start_size = db.get_free_memory();
        for (auto i = 0; i < ids.size(); i++)
        {
            auto itr = _eosdb.db_find_i64(receiver, scope, table, ids[i]);
            _eosdb.db_update_i64(itr, receiver, updated_data, strlen(updated_data));
        }
        return start_size - db.get_free_memory();
    }

    static int64_t remove_by_table_ordered_id(chainbase::database &db, const std::vector<int> &ids)
    {
        eos_db _eosdb(db);
        auto receiver = name("receiver");
        auto table = name("table");
        auto scope = fc::sha256::hash<std::string>("scope");

        int64_t start_size = db.get_free_memory();
        for (auto i = 0; i < ids.size(); i++)
        {
            auto itr = _eosdb.db_find_i64(receiver, scope, table, ids[i]);
            _eosdb.db_remove_i64(itr);
        }
        return start_size - db.get_free_memory();
    }
};