%module(directors="1") mmseg

// Remove some warnings
#pragma SWIG nowarn=362,503,401,389,516,511

// Use STL support
%include <std_vector.i>
%include <std_string.i>
%include <std_map.i>
#if SWIGPYTHON || SWIGRUBY
%include <std_complex.i>
#endif
// Use C99 int support
%include <stdint.i>

// Use exceptions
%include "exception.i"
// Use pointer.
%include cpointer.i

%{
#include <csr_typedefs.h>
%}

// apply type mapping
%apply unsigned char { u1 }; 
%apply unsigned short { u2 }; 
%apply unsigned int { u4 }; 
%apply unsigned long long { u8 }; 

/* Create some functions for working with "int *" */
%pointer_functions(u2, ushortp);

// Global Tweaks to basedict
%ignore mm::LemmaPropertyEntry; 
%ignore mm::LemmaPropertyDefine; 
%ignore mm::LemmaPropertyType; 
%ignore mm::BaseDict::Init;
%ignore mm::CharMapper::Transform;
%ignore mm::BaseDictSchema::GetColumnType;
%ignore mm::DictMatchResult::ClearResult;
%ignore mm::DictMatchResult::AddResult;
%ignore mm::DictMatchResult::GetResultPtr;

// This section is copied verbatim into the generated source code.
// Any include files, definitions, etc. need to go here.
%{
#include <basedict.h>
%}

// Language Specific Sections
%include Java.i
%include Python.i

%newobject get_dict_property_string;
%newobject get_dict_property_string_by_value;

// Some C-Side Helper
%inline %{
  char* get_dict_property_string(mm::BaseDict* dict, unsigned int term_id, const char* key)
  {
  	int data_len = 0;
  	int rs = 0;
  	char* buf = NULL;
  	rs = dict->GetProp ( term_id, key, buf, &data_len);  // buf == NULL;
  	//printf("get data_len %d\n", data_len);
  	if(data_len) {
  		buf = (char*)malloc(data_len+1); // +1 for whitespace for string.
  		rs = dict->GetProp ( term_id, key, buf, &data_len);
  		if(rs == 0) {
  			buf[data_len] = 0;
  			return buf;
  		}
  	}
  	return NULL;
  }

  const char* get_dict_property_string_by_value(mm::BaseDict* dict, unsigned int value, const char* key)
  {
    int data_len = 0;
    int rs = 0;
    const char* buf_ptr = dict->GetEntryProperty ( value, key, &data_len);  // buf == NULL;
    //printf("get data_len %d\n", data_len);
    if(buf_ptr && data_len) {
      char * buf = (char*)malloc(data_len+1); // +1 for whitespace for string.
      memcpy(buf, buf_ptr, data_len);
      if(rs == 0) {
        buf[data_len] = 0;
        return buf;
      }
    }
    return NULL;
  }

  u8 get_dict_property_number(mm::BaseDict* dict, unsigned int term_id, const char* key) {
  	u8 v = 0;
  	int rs = dict->GetPropInteger ( term_id, key, &v);
  	if(rs == 0)
  		return v;
  	return 0;
  }

  void free_dict_property_string(char* p) {
  	free((void*)p);
  }

  /* Create any sort of [size] array */
  int *int_array(int size) {
     return (int *) malloc(size*sizeof(int));
  }
  /* Create a two-dimension array [size][10] */
  int (*int_array_10(int size))[10] {
     return (int (*)[10]) malloc(size*10*sizeof(int));
  }
%}

// Help SWIG handle std vectors
namespace std
{
  //%template(VectorBool) vector<bool>;
  //%template(VectorUInt8) vector<uint8_t>;
}

/* stl support */
%include stl.i
 
/* Wrapper并生成代码 */
%include "basedict.h"