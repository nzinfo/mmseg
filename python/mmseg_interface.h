#ifndef Py_CMMSEGMODULE_H
#define Py_CMMSEGMODULE_H

#include <Python.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

 int		init_cmmseg_module(PyObject *m);

 PyObject * PyMmseg_BuildDict(PyObject * self, PyObject* args);

#ifdef __cplusplus
}
#endif

#endif