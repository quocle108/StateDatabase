#define BOOST_TEST_MODULE db test

#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>
#include <chainbase/chainbase.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include <iostream>

#include "../db_viewer.hpp"
#include "../eos_db.hpp"

using namespace chainbase;
using namespace boost::multi_index;

BOOST_AUTO_TEST_CASE(store_by_scope)
{

   boost::filesystem::path temp = boost::filesystem::unique_path();
   try
   {
      chainbase::database db(temp, database::read_write, 1024 * 1024 * 8);

      eos_db _eosdb(db);
      db.add_index<table_id_multi_index>();
      db.add_index<key_value_index>();

      auto receiver = name("receiver");
      auto payer1 = name("payer1");
      auto payer2 = name("payer2");
      auto table = name("table");
      auto user_public_key = fc::crypto::private_key::generate().get_public_key();
      auto scope = fc::sha256::hash<>(user_public_key);

      _eosdb.db_store_by_scope(receiver, scope, table, payer1, "100.0000 EOS", strlen("100.0000 EOS"));

      BOOST_TEST_MESSAGE("Store");
      {
         const auto &itr = _eosdb.find_or_create_table(receiver, scope, table, payer1);
         BOOST_REQUIRE(itr.value == "100.0000 EOS");
      }

      BOOST_TEST_MESSAGE("Update");
      {
         const auto &itr = _eosdb.find_or_create_table(receiver, scope, table, payer2);
         _eosdb.db_update_by_scope(itr, receiver, "20.0000 EOS", strlen("20.0000 EOS"));
         BOOST_REQUIRE(itr.value == "20.0000 EOS" && itr.payer == receiver);
      }
   }
   catch (...)
   {
      bfs::remove_all(temp);
      throw;
   }
   bfs::remove_all(temp);
}

BOOST_AUTO_TEST_CASE(store_by_key_value)
{

   boost::filesystem::path temp = boost::filesystem::unique_path();
   try
   {
      chainbase::database db(temp, database::read_write, 1024 * 1024 * 8);

      eos_db _eosdb(db);
      db.add_index<table_key_multi_index>();

      auto scope = fc::sha256::hash<std::string>("scope");

      BOOST_TEST_MESSAGE("Store");
      {
         const auto &itr = _eosdb.db_store_by_key(scope, "key", scope, "100.0000 EOS", strlen("100.0000 EOS"));
         BOOST_REQUIRE(itr.value == "100.0000 EOS");
         // BOOST_REQUIRE_THROW(_eosdb.db_store_by_key(scope, "key", scope, "100.0000 EOS", strlen("100.0000 EOS"),boost::execution_exception);
      }

      BOOST_TEST_MESSAGE("Update");
      {
         _eosdb.db_update_by_key(scope, "key", scope, "20.0000 EOS", strlen("20.0000 EOS"));
         const auto &itr = _eosdb.db_find_by_key(scope, "key");
         BOOST_REQUIRE(itr->value == "20.0000 EOS");
      }

      BOOST_TEST_MESSAGE("remove");
      {
         _eosdb.db_remove_by_key(scope, "key");
         auto itr = _eosdb.db_find_by_key(scope, "key");
         BOOST_REQUIRE(itr == nullptr);
      }
   }
   catch (...)
   {
      bfs::remove_all(temp);
      throw;
   }
   bfs::remove_all(temp);
}

BOOST_AUTO_TEST_CASE(store_primary_index)
{

   boost::filesystem::path temp = boost::filesystem::unique_path();
   try
   {
      chainbase::database db(temp, database::read_write, 1024 * 1024 * 8);
      // chainbase::database db2(temp); /// open an already created db

      eos_db _eosdb(db);
      db.add_index<table_id_multi_index>();
      db.add_index<key_value_index>();
      db.add_index<index64_index>();
      db.add_index<index128_index>();
      db.add_index<index256_index>();

      auto receiver = name("receiver");
      auto table = name("table");
      auto scope = fc::sha256::hash<std::string>("scope");

      int alice_itr = _eosdb.db_store_i64(receiver, scope, table, receiver, name("alice").to_uint64_t(), "alice's info", strlen("alice's info"));
      _eosdb.db_store_i64(receiver, scope, table, receiver, name("bob").to_uint64_t(), "bob's info", strlen("bob's info"));
      _eosdb.db_store_i64(receiver, scope, table, receiver, name("charlie").to_uint64_t(), "charlie's info", strlen("charlies's info"));
      _eosdb.db_store_i64(receiver, scope, table, receiver, name("allyson").to_uint64_t(), "allyson's info", strlen("allyson's info"));

      BOOST_TEST_MESSAGE("Find");
      {
         uint64_t prim = 0;
         int itr_next = _eosdb.db_next_i64(alice_itr, prim);
         int itr_next_expected = _eosdb.db_find_i64(receiver, scope, table, name("allyson").to_uint64_t());
         BOOST_REQUIRE(itr_next == itr_next_expected && prim == name("allyson").to_uint64_t());
         itr_next = _eosdb.db_next_i64(itr_next, prim);
         itr_next_expected = _eosdb.db_find_i64(receiver, scope, table, name("bob").to_uint64_t());
         BOOST_REQUIRE(itr_next == itr_next_expected && prim == name("bob").to_uint64_t());
      }

       BOOST_TEST_MESSAGE("Next");
      {
         int charlie_itr = _eosdb.db_find_i64(receiver, scope, table, name("charlie").to_uint64_t());
         // nothing after charlie
         uint64_t prim = 0;
         int end_itr = _eosdb.db_next_i64(charlie_itr, prim);
         BOOST_REQUIRE(end_itr < 0);
         // prim didn't change
         BOOST_REQUIRE_EQUAL(prim, 0);
      }

       BOOST_TEST_MESSAGE("Previous");
      {
         int charlie_itr = _eosdb.db_find_i64(receiver, scope, table, name("charlie").to_uint64_t());
         uint64_t prim = 0;
         int itr_prev = _eosdb.db_previous_i64(charlie_itr, prim);
         int itr_prev_expected = _eosdb.db_find_i64(receiver, scope, table, name("bob").to_uint64_t());
         BOOST_REQUIRE(itr_prev == itr_prev_expected && prim == name("bob").to_uint64_t());

         itr_prev = _eosdb.db_previous_i64(itr_prev, prim);
         itr_prev_expected = _eosdb.db_find_i64(receiver, scope, table, name("allyson").to_uint64_t());
         BOOST_REQUIRE(itr_prev == itr_prev_expected && prim == name("allyson").to_uint64_t());

         itr_prev = _eosdb.db_previous_i64(itr_prev, prim);
         itr_prev_expected = _eosdb.db_find_i64(receiver, scope, table, name("alice").to_uint64_t());
         BOOST_REQUIRE(itr_prev == itr_prev_expected && prim == name("alice").to_uint64_t());

         itr_prev = _eosdb.db_previous_i64(itr_prev, prim);
         BOOST_REQUIRE(itr_prev < 0 && prim == name("alice").to_uint64_t());
      }

      BOOST_TEST_MESSAGE("Remove");
      {
         int itr = _eosdb.db_find_i64(receiver, scope, table, name("alice").to_uint64_t());
         BOOST_REQUIRE(itr >= 0);
         _eosdb.db_remove_i64(itr);
         itr = _eosdb.db_find_i64(receiver, scope, table, name("alice").to_uint64_t());
         BOOST_REQUIRE(itr < 0);
      }

      BOOST_TEST_MESSAGE("Get");
      {
         int itr = _eosdb.db_find_i64(receiver, scope, table, name("bob").to_uint64_t());
         BOOST_REQUIRE(itr >= 0);
         uint32_t buffer_len = 5;
         char value[50];
         auto len = _eosdb.db_get_i64(itr, value, buffer_len);
         value[buffer_len] = '\0';
         std::string s(value);
         BOOST_REQUIRE(uint32_t(len) == buffer_len);
         BOOST_REQUIRE(s == "bob's");

         buffer_len = 20;
         len = _eosdb.db_get_i64(itr, value, 0);
         len = _eosdb.db_get_i64(itr, value, (uint32_t)len);
         value[len] = '\0';
         std::string sfull(value);
         BOOST_REQUIRE(sfull == "bob's info");
      }

      BOOST_TEST_MESSAGE("Update");
      {
         int itr = _eosdb.db_find_i64(receiver, scope, table, name("bob").to_uint64_t());
         BOOST_REQUIRE(itr >= 0);
         const char *new_value = "bob's new info";
         uint32_t new_value_len = strlen(new_value);
         _eosdb.db_update_i64(itr, receiver, new_value, new_value_len);
         char ret_value[50];
         _eosdb.db_get_i64(itr, ret_value, new_value_len);
         ret_value[new_value_len] = '\0';
         std::string sret(ret_value);
         BOOST_REQUIRE(sret == "bob's new info");
      }
   }
   catch (...)
   {
      bfs::remove_all(temp);
      throw;
   }
   bfs::remove_all(temp);
}
