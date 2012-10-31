#include <boost/python.hpp>
#include <assert.h>
#include <iostream>

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

class InvalidDictionaryPathException : public std::exception
{

private:
  int _code;
  std::string _message;

public:
  InvalidDictionaryPathException(int iCode, std::string message)
  {
    _code = iCode;
    _message = message;
  }
  const char *what() const throw()
  {
    return _message.c_str();
  }

  ~ InvalidDictionaryPathException() throw()
  {
  }

  std::string getMessage()
  {
    return _message;
  }
};

///////////////////////////////////////////////////
//  Main Interface
///////////////////////////////////////////////////

typedef std::vector<u4> seglist;
using namespace css;

class MMSegImpl
{

   public:
     MMSegImpl():_mgr(NULL) {};
     ~MMSegImpl(){
         if(_mgr) delete _mgr;
         _mgr = NULL;
     }

   public:
     int init(std::string sPath) {
	 /*
 	  * = only init segmenter once
 	  */
         if(_mgr == NULL) {
            _mgr = new SegmenterManager();
            int nRet = _mgr->init(sPath.c_str());
            if(nRet != 0) {
                delete _mgr;
                _mgr = NULL;
                throw InvalidDictionaryPathException(404, "Invalid dictionary path "+sPath+" .");
	    }
         }
         return 0;
     }
     seglist tokenizer(std::string sText) {
         if(_mgr == NULL)
		throw InvalidDictionaryPathException(500, "unset dictionary path, call init first.");

         Segmenter* seg = _mgr->getSegmenter(false); 
	 seg->setBuffer((u1*)sText.c_str(), sText.length());
	 
         // init result.
         seglist ret;
  	 ret.reserve(2048);
	 u4 offset = 0;
         while(true)
         {
		u2 len = 0, symlen = 0;
		char* tok = (char*)seg->peekToken(len,symlen);
		if(!tok || !*tok || !len)
			break;
		offset += len;
         	ret.push_back(offset);
		seg->popToken(len);
         }
         return ret;
     }

   protected:
     SegmenterManager* _mgr;
};


PyObject *myCPPExceptionType = NULL;

void translateMyCPPException(InvalidDictionaryPathException const &e)
{
  assert(myCPPExceptionType != NULL);
  boost::python::object pythonExceptionInstance(e);
  PyErr_SetObject(myCPPExceptionType, pythonExceptionInstance.ptr());
}

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

BOOST_PYTHON_MODULE(pymmseg)
{
   using namespace boost::python;
   
   boost::python::class_<InvalidDictionaryPathException>
      myCPPExceptionClass("InvalidDictionaryPathException",
            boost::python::init<int, std::string>());
   
   myCPPExceptionClass.add_property("message", &InvalidDictionaryPathException::getMessage);

   myCPPExceptionType = myCPPExceptionClass.ptr();
   boost::python::register_exception_translator<InvalidDictionaryPathException>
    	(&translateMyCPPException);

   //def("yay", yay);
   class_<std::vector<u4> >("position_vector")
            .def(vector_indexing_suite<std::vector<u4> >())
        ;

   class_<MMSegImpl>("mmseg")
       .def("init", &MMSegImpl::init)
       .def("split", &MMSegImpl::tokenizer)
       ;
}
