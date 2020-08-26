#include <chainbase/chainbase.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <string>
#include <iostream>
#include "types.hpp"

using namespace ::boost;
using namespace ::boost::multi_index;
using chainbase::allocator;

struct table_id_object : public chainbase::object<table_id_object_type, int64_t>
{
    //CHAINBASE_DEFAULT_CONSTRUCTOR(table_id_object)
    template <typename Constructor, typename Allocator>
    table_id_object(Constructor &&c, Allocator &&a) : value(a)
    {
        c(*this);
    }
    
    id_type id;
    name code;
    fc::sha256 scope;
    name table;
    name payer;
    uint32_t count = 0; /// the number of elements in the table
    shared_blob value;
};

struct by_id;
struct by_code_scope_table;

using table_id_multi_index = chainbase::shared_multi_index_container<
    table_id_object,
    indexed_by<
        ordered_unique<tag<by_id>,
                       member<table_id_object, table_id_object::id_type, &table_id_object::id>>,
        ordered_unique<tag<by_code_scope_table>,
                       composite_key<table_id_object,
                                     member<table_id_object, name, &table_id_object::code>,
                                     member<table_id_object, fc::sha256, &table_id_object::scope>,
                                     member<table_id_object, name, &table_id_object::table>>,
                                     composite_key_compare<std::less<name>, sha256_less, std::less<name>>>>>;
                                    

using table_id = table_id_object::id_type;

struct by_scope_primary;
struct by_scope_secondary;
struct by_scope_tertiary;

struct key_value_object : public chainbase::object<key_value_object_type, int64_t>
{
    // CHAINBASE_DEFAULT_CONSTRUCTOR( key_value_object )

    template <typename Constructor, typename Allocator>
    key_value_object(Constructor &&c, Allocator &&a) : value(a)
    {
        c(*this);
    }
    typedef uint64_t key_type;
    static const int number_of_keys = 1;

    id_type id;
    table_id t_id;        //< t_id should not be changed within a chainbase modifier lambda
    uint64_t primary_key; //< primary_key should not be changed within a chainbase modifier lambda
    name payer;
    shared_blob value;
};

using key_value_index = chainbase::shared_multi_index_container<
    key_value_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<key_value_object, key_value_object::id_type, &key_value_object::id>>,
        ordered_unique<tag<by_scope_primary>,
                       composite_key<key_value_object,
                                     member<key_value_object, table_id, &key_value_object::t_id>,
                                     member<key_value_object, uint64_t, &key_value_object::primary_key>>,
                       composite_key_compare<std::less<table_id>, std::less<uint64_t>>>>>;
struct by_primary;
struct by_secondary;

template <typename SecondaryKey, uint64_t ObjectTypeId, typename SecondaryKeyLess = std::less<SecondaryKey>>
struct secondary_index
{

    struct index_object : public chainbase::object<ObjectTypeId, int64_t>
    {
        CHAINBASE_DEFAULT_CONSTRUCTOR(index_object)

        typedef SecondaryKey secondary_key_type;
        typename chainbase::object<ObjectTypeId, int64_t>::id_type id;

        table_id t_id;        //< t_id should not be changed within a chainbase modifier lambda
        uint64_t primary_key; //< primary_key should not be changed within a chainbase modifier lambda
        name payer;
        SecondaryKey secondary_key; //< secondary_key should not be changed within a chainbase modifier lambda
    };

    typedef chainbase::shared_multi_index_container<
        index_object,
        indexed_by<
            ordered_unique<tag<by_id>, member<index_object, typename index_object::id_type, &index_object::id>>,
            ordered_unique<tag<by_primary>,
                           composite_key<index_object,
                                         member<index_object, table_id, &index_object::t_id>,
                                         member<index_object, uint64_t, &index_object::primary_key>>,
                           composite_key_compare<std::less<table_id>, std::less<uint64_t>>>,
            ordered_unique<tag<by_secondary>,
                           composite_key<index_object,
                                         member<index_object, table_id, &index_object::t_id>,
                                         member<index_object, SecondaryKey, &index_object::secondary_key>,
                                         member<index_object, uint64_t, &index_object::primary_key>>,
                           composite_key_compare<std::less<table_id>, SecondaryKeyLess, std::less<uint64_t>>>>>
        index_index;
};

typedef secondary_index<uint64_t, index64_object_type>::index_object index64_object;
typedef secondary_index<uint64_t, index64_object_type>::index_index index64_index;

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
typedef secondary_index<uint128_t, index128_object_type>::index_object index128_object;
typedef secondary_index<uint128_t, index128_object_type>::index_index index128_index;

typedef std::array<uint128_t, 2> key256_t;
typedef secondary_index<key256_t, index256_object_type>::index_object index256_object;
typedef secondary_index<key256_t, index256_object_type>::index_index index256_index;

struct table_key_object : public chainbase::object<table_key_value_object_type, fc::sha256>
{
    template <typename Constructor, typename Allocator>
    table_key_object(Constructor &&c, Allocator &&a) : value(a)
    {
        c(*this);
    }
    
    id_type id;
    fc::sha256 payer;
    shared_blob value;
};

struct by_id;
struct by_key;
struct by_composite_key;
using table_key_multi_index = chainbase::shared_multi_index_container<
    table_key_object,
    indexed_by<hashed_unique<tag<by_key>, member<table_key_object, fc::sha256, &table_key_object::id>>>>;


CHAINBASE_SET_INDEX_TYPE(table_id_object, table_id_multi_index)
CHAINBASE_SET_INDEX_TYPE(key_value_object, key_value_index)
CHAINBASE_SET_INDEX_TYPE(index64_object, index64_index)
CHAINBASE_SET_INDEX_TYPE(index128_object, index128_index)
CHAINBASE_SET_INDEX_TYPE(index256_object, index256_index)
CHAINBASE_SET_INDEX_TYPE(table_key_object, table_key_multi_index)