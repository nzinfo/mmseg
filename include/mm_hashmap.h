#ifdef __APPLE__
#include <unordered_map>
using std::unordered_map;
#else
    #ifdef __GNUC__
    #include <tr1/unordered_map>
    using namespace std::tr1;
    //#include <ext/hash_map>
    //   #define unordered_map std::hash_map
    #endif
#endif

#if defined(_MSC_VER)  // Compat for vc 2008 ( python2.7 reply on it )
# include <hash_map>
using stdext::hash_map;
#define unordered_map stdext::hash_map
#endif
