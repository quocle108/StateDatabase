#ifndef INCLUDE_PERMISSION_OBJECT_HPP_
#define INCLUDE_PERMISSION_OBJECT_HPP_

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
struct permission_usage_object : public chainbase::object<permission_usage_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   permission_usage_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }
   id_type id;
   time_point last_used; ///< when this permission was last used
};

struct by_account_permission;
using permission_usage_index = chainbase::shared_multi_index_container<
    permission_usage_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<permission_usage_object, permission_usage_object::id_type, &permission_usage_object::id>>>>;

struct permission_object : public chainbase::object<permission_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   permission_object(Constructor &&c, Allocator &&a): auth(a)
   {
      c(*this);
   }

   id_type id;
   permission_usage_object::id_type usage_id;
   id_type parent;          ///< parent permission
   name owner;      ///< the account this permission belongs to (should not be changed within a chainbase modifier lambda)
   name permission_name;    ///< human-readable name for the permission (should not be changed within a chainbase modifier lambda)
   time_point last_updated; ///< the last time this authority was updated
   shared_blob                  auth;
};

/**
 * special cased to abstract the foreign keys for usage and the optimization of using OID for the parent
 */
struct snapshot_permission_object
{
   name parent;  ///< parent permission
   name owner;      ///< the account this permission belongs to
   name permission_name;    ///< human-readable name for the permission
   time_point last_updated; ///< the last time this authority was updated
   time_point last_used;    ///< when this permission was last used
   shared_blob         auth; ///< authority required to execute this permission
};

struct by_parent;
struct by_owner;
struct by_name;
struct by_id;
using permission_index = chainbase::shared_multi_index_container<
    permission_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<permission_object, permission_object::id_type, &permission_object::id>>,
        ordered_unique<tag<by_parent>,
                       composite_key<permission_object,
                                     member<permission_object, permission_object::id_type, &permission_object::parent>,
                                     member<permission_object, permission_object::id_type, &permission_object::id>>>,
        ordered_unique<tag<by_owner>,
                       composite_key<permission_object,
                                     member<permission_object, name, &permission_object::owner>,
                                     member<permission_object, name, &permission_object::permission_name>>>,
        ordered_unique<tag<by_name>,
                       composite_key<permission_object,
                                     member<permission_object, name, &permission_object::permission_name>,
                                     member<permission_object, permission_object::id_type, &permission_object::id>>>>>;

const permission_object &create_permission(chainbase::database &db, name account,
                                           name permission_name,
                                           permission_object::id_type parent
)
{


   const auto& permission_usage_idx = db.get_index<permission_usage_index>().indices();
   const auto &perm_usage = db.create<permission_usage_object>([&](auto &p)
                                                                { 
                                                                  p.id = permission_usage_idx.size();
                                                                  });

   const auto& permission_idx = db.get_index<permission_index>().indices();
   const std::string default_publickey = "PUB_K1_72XXv5CYHJQGBAvoWxKWKCkfuend9nccdiNXPE59G6EeYL1yD2";
   const auto &perm = db.create<permission_object>([&](auto &p)
                                                    {
         p.id = permission_idx.size();
         p.usage_id     = perm_usage.id;
         p.parent       = parent;
         p.owner        = account;
         p.permission_name         = permission_name;
         // p.last_updated = creation_time;
         p.auth.assign(default_publickey.c_str(), default_publickey.size());
         });
   return perm;
}

CHAINBASE_SET_INDEX_TYPE(permission_usage_object, permission_usage_index)
CHAINBASE_SET_INDEX_TYPE(permission_object, permission_index)

#endif // INCLUDE_PERMISSION_OBJECT_HPP_"