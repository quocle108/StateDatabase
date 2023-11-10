
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
#include <boost/interprocess/managed_external_buffer.hpp>
#include <boost/interprocess/anonymous_shared_memory.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio/io_service.hpp>

#include <cinttypes>
#include <inttypes.h>
#include "multi_index_includes.hpp"
#include "name.hpp"
#include "types.hpp"
#include "account_object.hpp"
#include "new_account_object.hpp"
#include "utils.hpp"

using namespace std;
using namespace chainbase;
using namespace boost::multi_index;
namespace bfs = boost::filesystem;

/**
    This simple program will open database_dir and add two new books every time
    it is run and then print out all of the books in the database.
 */
int main(int argc, char **argv)
{

   // boost::filesystem::path temp = boost::filesystem::unique_path();
   // std::cerr << temp << " \n";
   boost::filesystem::path database_dir = "state";
   chainbase::database db(database_dir, database::read_write, 1024 * 1024 * 1024);
   db.add_index<account_index>();
   db.add_index<account_metadata_index>();
   db.add_index<account_ram_correction_index>();
   for (auto i = 0; i < 1000000; i++)
   {
      create_native_account(db, name(i));
   }
   db.commit(0);
   printDatabase(db, database_dir);

   // boost::filesystem::path temp1 = boost::filesystem::unique_path();
   // std::cerr << temp1 << " \n";
   boost::filesystem::path new_database_dir = "state1";
   chainbase::database db1(new_database_dir, database::read_write, 1024 * 1024 * 1024);
   db1.add_index<new_account_index>();
   db1.add_index<new_account_metadata_index>();
   db1.add_index<new_account_ram_correction_index>();
   for (auto i = 0; i < 1000000; i++)
   {
      new_create_native_account(db1, name(i));
   }
   db1.commit(0);
   printDatabase(db1, new_database_dir);

   //  const auto& new_account_index = db1.get_index<new_account_index>().indices().get<by_id>();
   const name test_account = name(123);
   const auto *account_idx = db.find<account_object, by_name>(test_account);
   cout << "account name in current table is: " << name(account_idx->id).to_string() << endl;
   const auto *new_account_idx = db1.find<new_account_object, by_id>(test_account.to_uint64_t());
   cout << "account name in new table is: " << name(new_account_idx->id).to_string() << endl;

   return 0;
}
