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

// This section is copied verbatim into the generated source code.
// Any include files, definitions, etc. need to go here.
%{
#include <basedict.h>
%}

// Language Specific Sections
%include Java.i
%include Python.i

/* stl support */
%include stl.i
 
/* Wrapper并生成代码 */
%include "basedict.h"