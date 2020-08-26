#pragma once
#include "name.hpp"

#include <chainbase/chainbase.hpp>

#include <fc/interprocess/container.hpp>
#include <fc/io/varint.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/optional.hpp>
#include <fc/safe.hpp>
#include <fc/container/flat.hpp>
#include <fc/string.hpp>
#include <fc/io/raw.hpp>
#include <fc/static_variant.hpp>
#include <fc/smart_ref_fwd.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/fixed_string.hpp>
#include <fc/crypto/private_key.hpp>

#include <memory>
#include <vector>
#include <deque>
#include <cstdint>

#define OBJECT_CTOR1(NAME)                                \
   NAME() = delete;                                       \
                                                          \
public:                                                   \
   template <typename Constructor, typename Allocator>    \
   NAME(Constructor &&c, chainbase::allocator<Allocator>) \
   {                                                      \
      c(*this);                                           \
   }
#define OBJECT_CTOR2_MACRO(x, y, field) , field(a)
#define OBJECT_CTOR2(NAME, FIELDS)                                  \
   NAME() = delete;                                                 \
                                                                    \
public:                                                             \
   template <typename Constructor, typename Allocator>              \
   NAME(Constructor &&c, chainbase::allocator<Allocator> a)         \
       : id(0) BOOST_PP_SEQ_FOR_EACH(OBJECT_CTOR2_MACRO, _, FIELDS) \
   {                                                                \
      c(*this);                                                     \
   }
#define OBJECT_CTOR(...) BOOST_PP_OVERLOAD(OBJECT_CTOR, __VA_ARGS__) \
(__VA_ARGS__)

#define _V(n, v) fc::mutable_variant_object(n, v)

using std::all_of;
using std::deque;
using std::enable_shared_from_this;
using std::forward;
using std::make_pair;
using std::map;
using std::move;
using std::pair;
using std::set;
using std::shared_ptr;
using std::string;
using std::tie;
using std::to_string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;
using std::weak_ptr;

using fc::enum_type;
using fc::flat_map;
using fc::flat_multimap;
using fc::flat_set;
using fc::optional;
using fc::path;
using fc::safe;
using fc::signed_int;
using fc::smart_ref;
using fc::static_variant;
using fc::time_point;
using fc::time_point_sec;
using fc::unsigned_int;
using fc::variant;
using fc::variant_object;
using fc::ecc::commitment_type;
using fc::ecc::range_proof_info;
using fc::ecc::range_proof_type;

using public_key_type = fc::crypto::public_key;
using private_key_type = fc::crypto::private_key;
using signature_type = fc::crypto::signature;

struct void_t
{
};

using chainbase::allocator;
using shared_string = boost::interprocess::basic_string<char, std::char_traits<char>, allocator<char>>;
template <typename T>
using shared_vector = boost::interprocess::vector<T, allocator<T>>;
template <typename T>
using shared_set = boost::interprocess::set<T, std::less<T>, allocator<T>>;
template <typename K, typename V>
using shared_flat_multimap = boost::interprocess::flat_multimap<K, V, std::less<K>, allocator<std::pair<K, V>>>;

/**
    * For bugs in boost interprocess we moved our blob data to shared_string
    * this wrapper allows us to continue that while also having a type-level distinction for
    * serialization and to/from variant
    */
class shared_blob : public shared_string
{
public:
   shared_blob() = delete;
   shared_blob(shared_blob &&) = default;

   shared_blob(const shared_blob &s)
       : shared_string(s.get_allocator())
   {
      assign(s.c_str(), s.size());
   }

   shared_blob &operator=(const shared_blob &s)
   {
      assign(s.c_str(), s.size());
      return *this;
   }

   shared_blob &operator=(shared_blob &&) = default;

   template <typename InputIterator>
   shared_blob(InputIterator f, InputIterator l, const allocator_type &a)
       : shared_string(f, l, a)
   {
   }

   shared_blob(const allocator_type &a)
       : shared_string(a)
   {
   }
};

using action_name = name;
using scope_name = name;
using account_name = name;
using permission_name = name;
using table_name = name;

   enum table_type
   {
      reversible_block_object_type,
      table_id_object_type,
      key_value_object_type,
      index64_object_type,
      index128_object_type,
      index256_object_type,
      table_key_value_object_type
   };

class account_object;

using block_id_type = fc::sha256;
using checksum_type = fc::sha256;
using checksum256_type = fc::sha256;
using checksum512_type = fc::sha512;
using checksum160_type = fc::ripemd160;
using transaction_id_type = checksum_type;
using digest_type = checksum_type;
using weight_type = uint16_t;
using block_num_type = uint32_t;
using share_type = int64_t;
using int128_t = __int128;
using uint128_t = unsigned __int128;
using bytes = vector<char>;

struct sha256_less
{
   bool operator()(const fc::sha256 &lhs, const fc::sha256 &rhs) const
   {
      return std::tie(lhs._hash[0], lhs._hash[1], lhs._hash[2], lhs._hash[3]) <
             std::tie(rhs._hash[0], rhs._hash[1], rhs._hash[2], rhs._hash[3]);
   }
};

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
typedef std::array<uint128_t, 2> key256_t;