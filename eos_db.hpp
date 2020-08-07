
#include "qos_contract_table.hpp"
#include <iostream>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

using namespace std;

template <typename T>
class iterator_cache
{
public:
    iterator_cache()
    {
        _end_iterator_to_table.reserve(8);
        _iterator_to_object.reserve(32);
    }

    /// Returns end iterator of the table.
    int cache_table(const table_id_object &tobj)
    {
        auto itr = _table_cache.find(tobj.id);
        if (itr != _table_cache.end())
            return itr->second.second;

        auto ei = index_to_end_iterator(_end_iterator_to_table.size());
        _end_iterator_to_table.push_back(&tobj);
        _table_cache.emplace(tobj.id, make_pair(&tobj, ei));
        return ei;
    }

    const table_id_object &get_table(table_id_object::id_type i) const
    {
        auto itr = _table_cache.find(i);
        assert(itr != _table_cache.end() && "an invariant was broken, table should be in cache");
        return *itr->second.first;
    }

    int get_end_iterator_by_table_id(table_id_object::id_type i) const
    {
        auto itr = _table_cache.find(i);
        assert(itr != _table_cache.end() && "an invariant was broken, table should be in cache");
        return itr->second.second;
    }

    const table_id_object *find_table_by_end_iterator(int ei) const
    {
        assert(ei < -1 && "not an end iterator");
        auto indx = end_iterator_to_index(ei);
        if (indx >= _end_iterator_to_table.size())
            return nullptr;
        return _end_iterator_to_table[indx];
    }

    const T &get(int iterator)
    {
        assert(iterator != -1 && "invalid iterator");
        assert(iterator >= 0 && "dereference of end iterator");
        assert((size_t)iterator < _iterator_to_object.size() && "iterator out of range");
        auto result = _iterator_to_object[iterator];
        assert(result && "dereference of deleted object");
        return *result;
    }

    void remove(int iterator)
    {
        assert(iterator != -1 && "invalid iterator");
        assert(iterator >= 0 && "cannot call remove on end iterators");
        assert((size_t)iterator < _iterator_to_object.size() && "iterator out of range");

        auto obj_ptr = _iterator_to_object[iterator];
        if (!obj_ptr)
            return;
        _iterator_to_object[iterator] = nullptr;
        _object_to_iterator.erase(obj_ptr);
    }

    int add(const T &obj)
    {
        auto itr = _object_to_iterator.find(&obj);
        if (itr != _object_to_iterator.end())
            return itr->second;

        _iterator_to_object.push_back(&obj);
        _object_to_iterator[&obj] = _iterator_to_object.size() - 1;

        return _iterator_to_object.size() - 1;
    }

private:
    map<table_id_object::id_type, pair<const table_id_object *, int>> _table_cache;
    vector<const table_id_object *> _end_iterator_to_table;
    vector<const T *> _iterator_to_object;
    map<const T *, int> _object_to_iterator;

    /// Precondition: std::numeric_limits<int>::min() < ei < -1
    /// Iterator of -1 is reserved for invalid iterators (i.e. when the appropriate table has not yet been created).
    inline size_t end_iterator_to_index(int ei) const { return (-ei - 2); }
    /// Precondition: indx < _end_iterator_to_table.size() <= std::numeric_limits<int>::max()
    inline int index_to_end_iterator(size_t indx) const { return -(indx + 2); }
}; /// class iterator_cache

class eos_db
{
public:
    eos_db() = delete;
    eos_db(chainbase::database &mutable_db);

    const table_id_object &find_or_create_table(name code, fc::sha256 scope, name table, name payer);

    const table_id_object &db_store_by_scope(name code, fc::sha256 scope, name table, name payer, const char *buffer, size_t buffer_size);
    
    void db_update_by_scope(const table_id_object& iterator, name payer, const char *buffer, size_t buffer_size);

    int db_store_i64(name code, fc::sha256 scope, name table, name payer, uint64_t id, const char *buffer, size_t buffer_size);

    const table_id_object *find_table(name code, fc::sha256 scope, name table);

    void db_update_i64(int iterator, name payer, const char *buffer, size_t buffer_size);

    void db_remove_i64(int iterator);

    int db_get_i64(int iterator, char *buffer, size_t buffer_size);

    int db_next_i64(int iterator, uint64_t &primary);

    int db_previous_i64(int iterator, uint64_t &primary);

    int db_find_i64(name code, fc::sha256 scope, name table, uint64_t id);

    int db_lowerbound_i64(name code, fc::sha256 scope, name table, uint64_t id);

    int db_upperbound_i64(name code, fc::sha256 scope, name table, uint64_t id);

    int db_end_i64(name code, fc::sha256 scope, name table);

    template <typename ObjectType, typename SecondaryType>
    class generic_index{
        public:

            generic_index() = delete;
            generic_index(eos_db& x): parent(x) {}
            
            int store(name code, fc::sha256 scope, name table, name payer, uint64_t id, const SecondaryType &value)
            {
                const auto &tab = parent.find_or_create_table(code, scope, table, payer);

                const auto &obj = parent.db.create<ObjectType>([&](auto &o) {
                    o.t_id = tab.id;
                    o.primary_key = id;
                    o.secondary_key = value;
                    o.payer = payer;
                });

                parent.db.modify(tab, [&](auto &t) {
                    ++t.count;
                });
                itr_cache.cache_table( tab );
               return itr_cache.add( obj );
            }


            void remove( int iterator ) {
               const auto& obj = itr_cache.get( iterator );

               const auto& table_obj = itr_cache.get_table( obj.t_id );

               parent.db.modify( table_obj, [&]( auto& t ) {
                  --t.count;
               });
               parent.db.remove( obj );

               itr_cache.remove( iterator );
            }
            void update( int iterator, name payer, SecondaryType secondary ) {
               const auto& obj = itr_cache.get( iterator );

               const auto& table_obj = itr_cache.get_table( obj.t_id );
               parent.db.modify( obj, [&]( auto& o ) {
                 o.secondary_key= secondary;
                 o.payer = payer;
               });
            }
            int find_secondary( uint64_t code, fc::sha256 scope, uint64_t table, SecondaryType secondary, uint64_t& primary ) {
               auto tab = parent.find_table( name(code), scope, name(table) );
               if( !tab ) return -1;

               auto table_end_itr = itr_cache.cache_table( *tab );

               const auto* obj = parent.db.find<ObjectType, by_secondary>( boost::make_tuple(*tab, secondary));
               if( !obj ) return table_end_itr;

               primary = obj->primary_key;

               return itr_cache.add( *obj );
            }

        private:
        iterator_cache<ObjectType>  itr_cache;
        eos_db&  parent;
    };
private:
    chainbase::database &db; ///< database where state is stored
    iterator_cache<key_value_object> keyval_cache;
    
   public:
      generic_index<index64_object, uint64_t>   idx64;
      generic_index<index128_object, uint128_t> idx128;
      generic_index<index256_object, key256_t> idx256;
};
