
#include "eos_db.hpp"

eos_db::eos_db(chainbase::database &mutable_db) : db(mutable_db),idx64(*this),idx128(*this),idx256(*this)
{
}

const table_id_object &eos_db::find_or_create_table(name code, fc::sha256 scope, name table, name payer)
{
    const auto *existing_tid = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
    if (existing_tid != nullptr)
    {
        return *existing_tid;
    }

    return db.create<table_id_object>([&](table_id_object &t_id) {
        t_id.code = code;
        t_id.scope = scope;
        t_id.table = table;
        t_id.payer = payer;
    });
}

const table_id_object &eos_db::db_store_by_scope(name code, fc::sha256 scope, name table, name payer, const char *buffer, size_t buffer_size)
{   
    return db.create<table_id_object>([&](table_id_object &t_id) {
        t_id.code = code;
        t_id.scope = scope;
        t_id.table = table;
        t_id.payer = payer;
        t_id.value.assign(buffer, buffer_size);
    });
}

void eos_db::db_update_by_scope(const table_id_object& iterator, name payer, const char *buffer, size_t buffer_size){
    
    db.modify(iterator, [&](auto &t) {
        t.value.assign(buffer, buffer_size);
        t.payer = payer;
    });      
}

int eos_db::db_store_i64(name code, fc::sha256 scope, name table, name payer, uint64_t id, const char *buffer, size_t buffer_size)
{

    const auto &tab = find_or_create_table(code, scope, table, payer);
    auto tableid = tab.id;
    const auto &obj = db.create<key_value_object>([&](auto &o) {
        o.t_id = tableid;
        o.primary_key = id;
        o.value.assign(buffer, buffer_size);
        o.payer = payer;
    });

    db.modify(tab, [&](auto &t) {
        ++t.count;
    });
    keyval_cache.cache_table(tab);
    return keyval_cache.add(obj);
}

const table_id_object *eos_db::find_table(name code, fc::sha256 scope, name table)
{
    return db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
}

void eos_db::db_update_i64(int iterator, name payer, const char *buffer, size_t buffer_size)
{
    
    const key_value_object &obj = keyval_cache.get(iterator);

    // const auto &table_obj = keyval_cache.get_table(obj.t_id);

    db.modify(obj, [&](auto &o) {
         o.value.assign( buffer, buffer_size );
        o.payer = payer;
    });
}

void eos_db::db_remove_i64(int iterator)
{
    const key_value_object &obj = keyval_cache.get(iterator);

    const auto &table_obj = keyval_cache.get_table(obj.t_id);

    db.modify(table_obj, [&](auto &t) {
        --t.count;
    });
    db.remove(obj);

    if (table_obj.count == 0)
    {
        db.remove(table_obj);
    }

    keyval_cache.remove(iterator);
}

int eos_db::db_get_i64( int iterator, char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );

   auto s = obj.value.size();
   if( buffer_size == 0 ) return s;

   auto copy_size = std::min( buffer_size, s );
   memcpy( buffer, obj.value.data(), copy_size );

   return copy_size;
}

int eos_db::db_next_i64(int iterator, uint64_t &primary)
{
    if (iterator < -1)
        return -1; // cannot increment past end iterator of table

    const auto &obj = keyval_cache.get(iterator); // Check for iterator != -1 happens in this call
    const auto &idx = db.get_index<key_value_index, by_scope_primary>();

    auto itr = idx.iterator_to(obj);
    ++itr;

    if (itr == idx.end() || itr->t_id != obj.t_id)
        return keyval_cache.get_end_iterator_by_table_id(obj.t_id);

    primary = itr->primary_key;
    return keyval_cache.add(*itr);
}

int eos_db::db_previous_i64(int iterator, uint64_t &primary)
{
    const auto &idx = db.get_index<key_value_index, by_scope_primary>();

    if (iterator < -1) // is end iterator
    {
        auto tab = keyval_cache.find_table_by_end_iterator(iterator);
        assert(tab && "not a valid end iterator");

        auto itr = idx.upper_bound(tab->id);
        if (idx.begin() == idx.end() || itr == idx.begin())
            return -1; // Empty table

        --itr;

        if (itr->t_id != tab->id)
            return -1; // Empty table

        primary = itr->primary_key;
        return keyval_cache.add(*itr);
    }

    const auto &obj = keyval_cache.get(iterator); // Check for iterator != -1 happens in this call

    auto itr = idx.iterator_to(obj);
    if (itr == idx.begin())
        return -1; // cannot decrement past beginning iterator of table

    --itr;

    if (itr->t_id != obj.t_id)
        return -1; // cannot decrement past beginning iterator of table

    primary = itr->primary_key;
    return keyval_cache.add(*itr);
}

int eos_db::db_find_i64(name code, fc::sha256 scope, name table, uint64_t id)
{
    //require_read_lock( code, scope ); // redundant?

    const auto *tab = find_table(code, scope, table);
    if (!tab)
        return -1;

    auto table_end_itr = keyval_cache.cache_table(*tab);

    const key_value_object *obj = db.find<key_value_object, by_scope_primary>(boost::make_tuple(tab->id, id));
    if (!obj)
        return table_end_itr;

    return keyval_cache.add(*obj);
}

int eos_db::db_lowerbound_i64(name code, fc::sha256 scope, name table, uint64_t id)
{
    //require_read_lock( code, scope ); // redundant?

    const auto *tab = find_table(code, scope, table);
    if (!tab)
        return -1;

    auto table_end_itr = keyval_cache.cache_table(*tab);

    const auto &idx = db.get_index<key_value_index, by_scope_primary>();
    auto itr = idx.lower_bound(boost::make_tuple(tab->id, id));
    if (itr == idx.end())
        return table_end_itr;
    if (itr->t_id != tab->id)
        return table_end_itr;

    return keyval_cache.add(*itr);
}

int eos_db::db_upperbound_i64(name code, fc::sha256 scope, name table, uint64_t id)
{
    //require_read_lock( code, scope ); // redundant?

    const auto *tab = find_table(code, scope, table);
    if (!tab)
        return -1;

    auto table_end_itr = keyval_cache.cache_table(*tab);

    const auto &idx = db.get_index<key_value_index, by_scope_primary>();
    auto itr = idx.upper_bound(boost::make_tuple(tab->id, id));
    if (itr == idx.end())
        return table_end_itr;
    if (itr->t_id != tab->id)
        return table_end_itr;

    return keyval_cache.add(*itr);
}

int eos_db::db_end_i64(name code, fc::sha256 scope, name table)
{
    //require_read_lock( code, scope ); // redundant?

    const auto *tab = find_table(code, scope, table);
    if (!tab)
        return -1;

    return keyval_cache.cache_table(*tab);
}

