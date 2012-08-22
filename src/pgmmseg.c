/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
* Version: GPL 2.0
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License. You should have
* received a copy of the GPL license along with this program; if you
* did not, you can find it at http://www.gnu.org/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is Coreseek.com code.
*
* Copyright (C) 2007-2008. All Rights Reserved.
*
* Author:
*	Li monan <li.monan@gmail.com>
*
* ***** END LICENSE BLOCK ***** */

#include <postgres.h>

#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

#include "catalog/pg_type.h"
#include "executor/spi.h"
#include "lib/stringinfo.h"
#include "mb/pg_wchar.h"
#include "parser/parse_coerce.h"
#include "parser/parse_type.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/typcache.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif //declare as the interface of pg extension

// define functions
Datum		mmseg_build_unigram (PG_FUNCTION_ARGS);
Datum		mmseg_segment (PG_FUNCTION_ARGS);
Datum		mmseg_term_expand (PG_FUNCTION_ARGS);

/** 
 * convert C string to text pointer.
 */
#define GET_TEXT(cstrp) \
   DatumGetTextP(DirectFunctionCall1(textin, CStringGetDatum(cstrp)))

/** 
 * convert text pointer to C string.
 */

#define GET_STR(textp)  \
   DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(textp)))
   
// impl
PG_FUNCTION_INFO_V1(mmseg_build_unigram);
Datum
mmseg_build_unigram(PG_FUNCTION_ARGS)
{
	/*
		= init mmseg env ( c++ side )
		= get a relation by name
		= build the object in memory -> mmseg build dict side
		= save the darts' data in memory.
	*/
	char *name;
	
	if ( PG_ARGISNULL(0) ) 
		PG_RETURN_NULL();
	name  = GET_STR(PG_GETARG_CSTRING(0));
	
	PG_RETURN_BOOL(true);
}

PG_FUNCTION_INFO_V1(mmseg_segment);
Datum
mmseg_segment(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(true);
}

PG_FUNCTION_INFO_V1(mmseg_term_expand);
Datum
mmseg_term_expand(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(true);
}

/* end of file */
