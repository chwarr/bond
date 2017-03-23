#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>

typedef boost::pool_allocator<
        char,
        boost::default_user_allocator_new_delete,
        boost::details::pool::null_mutex>
    PoolAlloc;
