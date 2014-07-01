#if !defined(_MM_HASHMAP_H)

#define _MM_HASHMAP_H

#include "csr_typedefs.h"

#ifdef __APPLE__
#include <unordered_map>
using std::unordered_map;
#define HASH_MAP_C11	1
#else
    #ifdef __GNUC__  //FIXME: check GCC's version.
    #include <tr1/unordered_map>
    using namespace std::tr1;
namespace std{ namespace tr1 {
#define tr1_hashtable_define_trivial_hash(T)            \
  template<>                                            \
    struct hash<T>                                      \
    : public std::unary_function<T, std::size_t>        \
    {                                                   \
      std::size_t                                       \
      operator()(T val) const                           \
      { return static_cast<std::size_t>(val); }         \
    } 
    tr1_hashtable_define_trivial_hash(u8);
#undef tr1_hashtable_define_trivial_hash

#define HASH_MAP_C11	0
} } //inject std::tr1

//----------------------------------------------	
    // modern gcc
    //#include <ext/hash_map>
    //   #define unordered_map std::hash_map
    #endif
#endif


#if defined(_MSC_VER)  // Compat for vc 2008 ( python2.7 reply on it )
# include <hash_map>
using stdext::hash_map;
#define unordered_map stdext::hash_map
#define HASH_MAP_C11	1

#endif

#endif // _MM_HASHMAP_H
