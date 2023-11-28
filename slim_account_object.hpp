#ifndef INCLUDE_SLIM_ACCOUNT_OBJECT_HPP_
#define INCLUDE_SLIM_ACCOUNT_OBJECT_HPP_

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

#include "name.hpp"
#include "types.hpp"
#include "slim_permission_object.hpp"
struct slim_account_object : public chainbase::object<slim_account_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   slim_account_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }

   id_type id;
   uint64_t recv_sequence = 0;
   uint64_t auth_sequence = 0;
   uint64_t ram_correction = 0;
};
using account_id_type = slim_account_object::id_type;
struct by_id;
using slim_account_index = chainbase::shared_multi_index_container<
    slim_account_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<slim_account_object, slim_account_object::id_type, &slim_account_object::id>>>>;


struct slim_resource_object : public chainbase::object<slim_resource_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   slim_resource_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }

   id_type id;
   bool pending = false; //< pending should not be changed within a chainbase modifier lambda

   int64_t net_weight = -1;
   int64_t cpu_weight = -1;
   int64_t ram_bytes = -1;

   usage_accumulator net_usage;
   usage_accumulator cpu_usage;

   uint64_t ram_usage = 0;
};

struct by_owner;
struct by_dirty;

using slim_resource_index = chainbase::shared_multi_index_container<
    slim_resource_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<slim_resource_object, slim_resource_object::id_type, &slim_resource_object::id>>,
        ordered_unique<tag<by_name>,
                       composite_key<slim_resource_object,
                                     member<slim_resource_object, bool, &slim_resource_object::pending>,
                                     member<slim_resource_object, slim_resource_object::id_type, &slim_resource_object::id>>>>>;

CHAINBASE_SET_INDEX_TYPE(slim_account_object, slim_account_index)
CHAINBASE_SET_INDEX_TYPE(slim_resource_object, slim_resource_index)


void initialize_slim_account(chainbase::database &db, const account_name& account_name) {
   const auto& limits = db.create<slim_resource_object>([&]( slim_resource_object& bl ) {
      bl.id = account_name.to_uint64_t();  
   });
}

void create_slim_account(chainbase::database &db, name account_name)
{
   db.create<slim_account_object>([&](auto &a)
                             {
                              a.id = account_name.to_uint64_t();
                              });

   initialize_slim_account(db, account_name);

   // create permisison
    const auto& active_permission  = create_slim_permission(db, account_name, name('active'), 0 );
}

#endif  // INCLUDE_SLIM_ACCOUNT_OBJECT_HPP_"