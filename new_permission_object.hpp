#ifndef INCLUDE_NEW_PERMISSION_OBJECT_HPP_
#define INCLUDE_NEW_PERMISSION_OBJECT_HPP_

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

using namespace std;
using namespace boost::multi_index;

namespace bfs = boost::filesystem;
struct new_permission_usage_object : public chainbase::object<new_permission_usage_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   new_permission_usage_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }
   id_type id;
   time_point last_used; ///< when this permission was last used
};

struct by_account_permission;
using new_permission_usage_index = chainbase::shared_multi_index_container<
    new_permission_usage_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<new_permission_usage_object, new_permission_usage_object::id_type, &new_permission_usage_object::id>>>>;

struct new_permission_object : public chainbase::object<new_permission_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   new_permission_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }

   id_type id;
   new_permission_usage_object::id_type usage_id;
   id_type parent;          ///< parent permission
   name owner;      ///< the account this permission belongs to (should not be changed within a chainbase modifier lambda)
   name permission_name;    ///< human-readable name for the permission (should not be changed within a chainbase modifier lambda)
   time_point last_updated; ///< the last time this authority was updated
};

/**
 * special cased to abstract the foreign keys for usage and the optimization of using OID for the parent
 */
struct new_snapshot_permission_object
{
   name parent;  ///< parent permission
   name owner;      ///< the account this permission belongs to
   name permission_name;    ///< human-readable name for the permission
   time_point last_updated; ///< the last time this authority was updated
   time_point last_used;    ///< when this permission was last used
};

struct by_parent;
struct by_owner;
struct by_name;
struct by_id;
using new_permission_index = chainbase::shared_multi_index_container<
    new_permission_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<new_permission_object, new_permission_object::id_type, &new_permission_object::id>>,
        ordered_unique<tag<by_parent>,
                       composite_key<new_permission_object,
                                     member<new_permission_object, new_permission_object::id_type, &new_permission_object::parent>,
                                     member<new_permission_object, new_permission_object::id_type, &new_permission_object::id>>>,
        ordered_unique<tag<by_owner>,
                       composite_key<new_permission_object,
                                     member<new_permission_object, name, &new_permission_object::owner>,
                                     member<new_permission_object, name, &new_permission_object::permission_name>>>,
        ordered_unique<tag<by_name>,
                       composite_key<new_permission_object,
                                     member<new_permission_object, name, &new_permission_object::permission_name>,
                                     member<new_permission_object, new_permission_object::id_type, &new_permission_object::id>>>>>;

const new_permission_object &new_create_permission(chainbase::database &db, name account,
                                           name permission_name,
                                           new_permission_object::id_type parent
)
{


   const auto& permission_usage_idx = db.get_index<new_permission_usage_index>().indices();
   
   const auto &perm_usage = db.create<new_permission_usage_object>([&](auto &p)
                                                                { 
                                                                  p.id = permission_usage_idx.size();});

   const auto& permission_idx = db.get_index<new_permission_index>().indices();
   const auto &perm = db.create<new_permission_object>([&](auto &p)
                                                    {
         p.id = permission_idx.size();
         p.usage_id     = perm_usage.id;
         p.parent       = parent;
         p.owner        = account;
         p.permission_name         = permission_name;
         // p.last_updated = creation_time;
         // p.auth         = std::move(auth);
         });
   return perm;
}

CHAINBASE_SET_INDEX_TYPE(new_permission_object, new_permission_index)
CHAINBASE_SET_INDEX_TYPE(new_permission_usage_object, new_permission_usage_index)

#endif // INCLUDE_NEW_PERMISSION_OBJECT_HPP_"