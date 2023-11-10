#ifndef INCLUDE_ACCOUNT_OBJECT_HPP_
#define INCLUDE_ACCOUNT_OBJECT_HPP_

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


struct account_object : public chainbase::object<account_object_type, int64_t>
{
   template <typename Constructor, typename Allocator>
   account_object(Constructor &&c, Allocator &&a) : abi(a)
   {
      c(*this);
   }

   id_type id;
   name account_name;
   fc::time_point creation_date;
   shared_blob abi;
};
using account_id_type = account_object::id_type;
struct by_id;
struct by_name;
using account_index = chainbase::shared_multi_index_container<
    account_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
        ordered_unique<tag<by_name>, member<account_object, name, &account_object::account_name>>>>;

struct account_metadata_object : public chainbase::object<account_metadata_object_type, int64_t>
{

   template <typename Constructor, typename Allocator>
   account_metadata_object(Constructor &&c, Allocator &&a)
   {
      c(*this);
   }
   id_type id;
   name account_name; //< name should not be changed within a chainbase modifier lambda
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

struct by_name;
using account_metadata_index = chainbase::shared_multi_index_container<
    account_metadata_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<account_metadata_object, account_metadata_object::id_type, &account_metadata_object::id>>,
        ordered_unique<tag<by_name>, member<account_metadata_object, name, &account_metadata_object::account_name>>>>;

struct account_ram_correction_object : public chainbase::object<account_ram_correction_object_type, int64_t>
{
   OBJECT_CTOR(account_ram_correction_object);

   id_type id;
   name account_name; //< name should not be changed within a chainbase modifier lambda
   uint64_t ram_correction = 0;
};

struct by_name;
using account_ram_correction_index = chainbase::shared_multi_index_container<
    account_ram_correction_object,
    indexed_by<
        ordered_unique<tag<by_id>, member<account_ram_correction_object, account_ram_correction_object::id_type, &account_ram_correction_object::id>>,
        ordered_unique<tag<by_name>, member<account_ram_correction_object, name, &account_ram_correction_object::account_name>>>>;

CHAINBASE_SET_INDEX_TYPE(account_object, account_index)
CHAINBASE_SET_INDEX_TYPE(account_metadata_object, account_metadata_index)
CHAINBASE_SET_INDEX_TYPE(account_ram_correction_object, account_ram_correction_index)

void create_native_account(chainbase::database &db, name account_name)
{
   const auto &account_object_idx = db.get_index<account_index>().indices();
   db.create<account_object>([&](auto &a)
                             {
                              a.id = account_object_idx.size();
                              a.account_name = account_name; });
   const auto &account_metadata_object_idx = db.get_index<account_index>().indices();
   db.create<account_metadata_object>([&](auto &a)
                                      { 
                                       a.id = account_metadata_object_idx.size();
                                       a.account_name = account_name; });

   int64_t ram_bytes = 123456;
   const auto &account_ram_correction_object_idx = db.get_index<account_index>().indices();
   auto ptr = db.find<account_ram_correction_object, by_name>(account_name);
   if (ptr)
   {
      db.modify<account_ram_correction_object>(*ptr, [&](auto &rco)
                                               { rco.ram_correction += ram_bytes; });
   }
   else
   {
      ptr = &db.create<account_ram_correction_object>([&](auto &rco)
                                                      {
         rco.id = account_ram_correction_object_idx.size();                                                
         rco.account_name = account_name;
         rco.ram_correction = ram_bytes; });
   }
}

#endif  // INCLUDE_ACCOUNT_OBJECT_HPP_"