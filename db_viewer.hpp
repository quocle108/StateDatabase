
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <chainbase/chainbase.hpp>

template <typename Index>
class db_viewer
{
public:
    using index_t = Index;

    template <typename F>
    static void walk(const chainbase::database &db, F function)
    {
        auto const &index = db.get_index<Index>().indices();
        const auto &first = index.begin();
        const auto &last = index.end();
        for (auto itr = first; itr != last; ++itr)
        {
            function(*itr);
        }
    }

    template <typename Secondary, typename Key, typename F>
    static void walk_range(const chainbase::database &db, const Key &begin_key, const Key &end_key, F function)
    {
        const auto &idx = db.get_index<Index, Secondary>();
        auto begin_itr = idx.lower_bound(begin_key);
        auto end_itr = idx.lower_bound(end_key);
        for (auto itr = begin_itr; itr != end_itr; ++itr)
        {
            function(*itr);
        }
    }

    template <typename Secondary, typename Key>
    static size_t size_range(const chainbase::database &db, const Key &begin_key, const Key &end_key)
    {
        const auto &idx = db.get_index<Index, Secondary>();
        auto begin_itr = idx.lower_bound(begin_key);
        auto end_itr = idx.lower_bound(end_key);
        size_t res = 0;
        while (begin_itr != end_itr)
        {
            res++;
            ++begin_itr;
        }
        return res;
    }

    template <typename F>
    static void create(chainbase::database &db, F cons)
    {
        db.create<typename index_t::value_type>(cons);
    }
};