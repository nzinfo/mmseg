#include "mmseg_interface.h"

#include <fstream>
#include <string>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <map>
#include  <stdlib.h>

#include "SegmenterManager.h"
#include "Segmenter.h"
#include "csr_utils.h"

using namespace std;
using namespace css;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	PyObject_HEAD
		/* Type-specific fields go here. */
		SegmenterManager* m_segmgr; // only PySource is supported for the leak support of setField in other documents
		Segmenter* m_thesaurus_seg; // used query thesaurus.
} csfHelper_MMSegObject;

PyObject * PyMmseg_Segment(PyObject * self, PyObject* args);
PyObject * PyMmseg_Thesaurus(PyObject * self, PyObject* args);

static int PyMMSeg_init(csfHelper_MMSegObject *self, PyObject *args, PyObject *kwds);

static void PyMMSeg_dealloc(csfHelper_MMSegObject* self);

static PyMethodDef PyMMSeg_Helper_methods[] = {  
	{"segment", PyMmseg_Segment, METH_VARARGS},   
	{"thesaurus", PyMmseg_Thesaurus, METH_VARARGS},   
	{"build_dict", PyMmseg_BuildDict, METH_VARARGS},   
	{NULL, NULL}  
};  



static PyTypeObject csfHelper_MMSegType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"Coreseek.MMSeg", /*tp_name*/
	sizeof(csfHelper_MMSegObject), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor)PyMMSeg_dealloc, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	0, /*tp_compare*/
	0, /*tp_repr*/ 
	0, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	0, /*tp_getattro*/
	0, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT, /*tp_flags*/
	"Coreseek MMSeg", /* tp_doc */
	0, /*tp_traverse*/
	0, /*tp_clear*/
	0, /*tp_richcompare*/
	0, /*tp_weaklistoffset*/
	0, /*tp_iter*/
	0, /*tp_iternext*/
	PyMMSeg_Helper_methods, /*tp_methods*/
	0, /*tp_members*/
	0, /*tp_getset*/
	0, /*tp_base*/
	0, /*tp_dict*/
	0, /*tp_descr_get*/
	0, /*tp_descr_set*/
	0, /*tp_dictoffset*/
	(initproc)PyMMSeg_init, /*tp_init*/
	0, /*tp_alloc*/
	0, /*tp_new*/
	0, /*tp_free*/
	0, /*tp_is_gc*/
	0, /*tp_bases*/
	0, /*tp_mro*/
	0, /*tp_cache*/
	0, /*tp_subclasses*/
	0, /*tp_weaklist*/
};

static int PyMMSeg_init(csfHelper_MMSegObject *self, PyObject *args, PyObject *kwds) {
	const char* key = NULL;
	PyObject* pV = NULL;
	int ok = PyArg_ParseTuple( args, "s", &key);  //not inc the value refer
	if(!ok) return -1;  
	if(!self->m_segmgr) {
		self->m_segmgr = new SegmenterManager();
		//can init only once for each instance
		int nRet = self->m_segmgr->init(key);
		//printf("%d:%s\n",nRet, key);
		if(nRet != 0) {
			delete self->m_segmgr;
			PyErr_SetString(PyExc_ValueError, "invalid dict_path");
			return -1;
		}
	}
	// 初始化 查询同义词需要的 segment
	self->m_thesaurus_seg = self->m_segmgr->getSegmenter(false); 
	return 0;
}

static void PyMMSeg_dealloc(csfHelper_MMSegObject* self) {
	if(self->m_thesaurus_seg) 
	{
		delete self->m_thesaurus_seg;
		self->m_thesaurus_seg  = NULL;
	}

	if(self->m_segmgr)
	{
		delete self->m_segmgr;
		self->m_segmgr = NULL;
	}
}

PyObject * PyMmseg_Segment(PyObject * self, PyObject* args)
{
	csfHelper_MMSegObject *self2 = (csfHelper_MMSegObject *)self;
	char *fromPython; 

	if (!PyArg_Parse(args, "(s)", &fromPython))
		return NULL;
	else
	{
		Segmenter* seg = self2->m_segmgr->getSegmenter(false); 
		seg->setBuffer((u1*)fromPython, (u4)strlen(fromPython));

		PyObject* item;
        PyObject* seg_result = PyList_New(0);
		while(1)
		{
			u2 len = 0, symlen = 0;
			char* tok = (char*)seg->peekToken(len,symlen);
			if(!tok || !*tok || !len){
				break;
			}
			//append new item
			item = PyString_FromStringAndSize(tok,len);
            PyList_Append(seg_result, item);
            Py_DECREF(item);
			seg->popToken(len);
		}
		//FIXME: free the segmenter
		delete seg;

		return seg_result;
	}
}

PyObject * PyMmseg_Thesaurus(PyObject * self, PyObject* args)
{
	csfHelper_MMSegObject *self2 = (csfHelper_MMSegObject *)self;
	char *fromPython; 

	if (!PyArg_Parse(args, "(s)", &fromPython))
		return NULL;
	else
	{
		Segmenter* seg = self2->m_thesaurus_seg;
		unsigned short len = 0;

		const char* thesaurus_ptr = seg->thesaurus(fromPython, strlen(fromPython));
		PyObject* seg_result = PyList_New(0);
        PyObject* item = NULL;
		while(thesaurus_ptr && *thesaurus_ptr) {
			len = strlen(thesaurus_ptr);
			//printf("%*.*s/s ",len,len,thesaurus_ptr);
			item = PyString_FromStringAndSize(thesaurus_ptr,len);
            PyList_Append(seg_result, item);
			Py_DECREF(item);
            thesaurus_ptr += len + 1; //move next
		}
		return seg_result;
	}
}

PyObject * PyMmseg_BuildDict(PyObject * self, PyObject* args)
{
	csfHelper_MMSegObject *self2 = (csfHelper_MMSegObject *)self;
	char *type; 
	char *source_file;
	char *target_file;

	if (!PyArg_Parse(args, "(sss)", &type, &source_file, &target_file))
		return NULL;
	else
	{
		int ret = 0;
		if(strncmp("unigram", type, 7) == 0) {
			UnigramCorpusReader ur;
			ur.open(source_file, NULL);
			int ret = 0;
			{
				UnigramDict ud;
				ret = ud.import(ur);
				ud.save(target_file);		
			}
			return Py_BuildValue("i",ret);
		}

		if(strncmp("thesaurus", type, 9) == 0 ) {
			ThesaurusDict tdict;
			ret = tdict.import(source_file, target_file);
			return Py_BuildValue("i",ret);
		}

		return Py_None;
	}
}

 int init_cmmseg_module(PyObject *m)
 {
	 csfHelper_MMSegType.tp_new = PyType_GenericNew;
	 if (PyType_Ready(&csfHelper_MMSegType) < 0)
		 return -1;
	 return PyModule_AddObject(m, "MMSeg", (PyObject *)&csfHelper_MMSegType);
 }

#ifdef __cplusplus
}
#endif