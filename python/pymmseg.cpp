#include <boost/python.hpp>

#include "csr_typedefs.h"
#include "SegmenterManager.h"
#include "Segmenter.h"
#include "csr_utils.h"
 
 char const* yay()
 {
   return "Yay!";
 }

/*
 * MMSeg Interface
 * */

typedef std::vector<u4> seglist;

class MMSegImpl
{
   public:
     int init(std::string sPath) {
         return 0;
     }
     seglist tokenizer(std::string sText) {
         seglist ret;
         ret.reserve(2048);
         ret.push_back(20);
         ret.push_back(25);
         ret.push_back(40);
         //seglist(ret).swap(ret);
         return ret;
     }
};

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

BOOST_PYTHON_MODULE(pymmseg)
{
   using namespace boost::python;

   //def("yay", yay);
   class_<std::vector<u4> >("position_vector")
            .def(vector_indexing_suite<std::vector<u4> >())
        ;

   class_<MMSegImpl>("mmseg")
       .def("init", &MMSegImpl::init)
       .def("split", &MMSegImpl::tokenizer)
       ;
}
