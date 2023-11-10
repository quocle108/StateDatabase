#ifndef INCLUDE_NEW_ACCOUNT_OBJECT_HPP_
#define INCLUDE_NEW_ACCOUNT_OBJECT_HPP_

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

struct new_account_object : public chainbase::object<new_account_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   new_account_object(Constructor &&c, Allocator &&a) : abi(a)
   {
      c(*this);
   }

   id_type id;
   fc::time_point creation_date;
   shared_blob abi;
};
using account_id_type = new_account_object::id_type;
struct by_id;
using new_account_index = chainbase::shared_multi_index_container<
    new_account_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<new_account_object, new_account_object::id_type, &new_account_object::id>>>>;

struct new_account_metadata_object : public chainbase::object<new_account_metadata_object_type, int64_t>
{

   template <typename Constructor, typename Allocator>
   new_account_metadata_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }
   id_type id;
   uint64_t recv_sequence = 0;
   uint64_t auth_sequence = 0;
   uint64_t code_sequence = 0;
   uint64_t abi_sequence = 0;
   digest_type code_hash;
   time_point last_code_update;
   uint32_t flags = 0;
   uint8_t vm_type = 0;
   uint8_t vm_version = 0;
};

using new_account_metadata_index = chainbase::shared_multi_index_container<
    new_account_metadata_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<new_account_metadata_object, new_account_metadata_object::id_type, &new_account_metadata_object::id>>>>;

struct new_account_ram_correction_object : public chainbase::object<new_account_ram_correction_object_type, int64_t>
{
   OBJECT_CTOR(new_account_ram_correction_object);

   id_type id;
   uint64_t ram_correction = 0;
};

using new_account_ram_correction_index = chainbase::shared_multi_index_container<
    new_account_ram_correction_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<new_account_ram_correction_object, new_account_ram_correction_object::id_type, &new_account_ram_correction_object::id>>>>;

CHAINBASE_SET_INDEX_TYPE(new_account_object, new_account_index)
CHAINBASE_SET_INDEX_TYPE(new_account_metadata_object, new_account_metadata_index)
CHAINBASE_SET_INDEX_TYPE(new_account_ram_correction_object, new_account_ram_correction_index)

void new_create_native_account(chainbase::database &db, name account_name)
{
   db.create<new_account_object>([&](auto &a)
                             {
                              a.id = account_name.to_uint64_t();
                              });
   db.create<new_account_metadata_object>([&](auto &a)
                                      { 
                                       a.id = account_name.to_uint64_t();
                                        });

   int64_t ram_bytes = 123456;
   auto ptr = db.find<new_account_ram_correction_object, by_id>(account_name.to_uint64_t());
   if (ptr)
   {
      db.modify<new_account_ram_correction_object>(*ptr, [&](auto &rco)
                                               { rco.ram_correction += ram_bytes; });
   }
   else
   {
      ptr = &db.create<new_account_ram_correction_object>([&](auto &rco)
                                                      {
         rco.id = account_name.to_uint64_t(); });
   }
}

#endif  // INCLUDE_NEW_ACCOUNT_OBJECT_HPP_"