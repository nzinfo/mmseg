# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
	用于测试 swig 版本的 MMSEG 接口
"""
import os
import sys

pwd = os.path.abspath(os.getcwd())
mmseg_so_path = os.path.join(pwd, 'binding')
#print mmseg_so_path
sys.path.insert(0, mmseg_so_path)

# try import
import _mmseg

print dir(_mmseg)

if True:	# Test Dart
	dt =  _mmseg.new_PharseDict()
	schema = "2:attr2;s:name;2:attr1;2:attr3"
	_mmseg.BaseDict_InitString( dt, schema, len(schema) )
	_mmseg.BaseDict_SetDictName(dt, "com.coreseek.mm.test")
	# add entrys
	terms = [("acc", 10), ("abc", 101), ("ebcd", 102), ("ab", 103), ("abgaowei", 104), ]
	_mmseg.BaseDict_Insert(dt, "a", 1, 100)
	msg = "hello world, abceef"
	_mmseg.BaseDict_SetProp(dt, 100, "name", msg, len(msg))
	_mmseg.BaseDict_SetPropInteger(dt, 100, "attr1", 9999)
	for key, kid, in terms:
		_mmseg.BaseDict_Insert(dt, key, len(key), kid)
	# build darts
	_mmseg.BaseDict_Build(dt)
	print _mmseg.BaseDict_Save(dt, 'd1.lib', 100)  # rev = 100
	# save
	# reload
	# query extract
	# query commonprefix
	_mmseg.delete_PharseDict(dt)
	

#end of file
