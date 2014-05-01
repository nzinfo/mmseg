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

if False:
	char_map = _mmseg.new_CharMapper(True)
	_mmseg.CharMapper_Mapping(char_map, 655, 1, 8)
	rs = _mmseg.new_ushortp()
	print _mmseg.CharMapper_TransformScript(char_map, 655, rs)
	print _mmseg.ushortp_value(rs)      # Dereference
	_mmseg.delete_ushortp(rs)     # Delete

	# save 
	_mmseg.CharMapper_Save(char_map, "mm_cm.lib")
	_mmseg.delete_CharMapper(char_map)

	# reload
	char_map = _mmseg.new_CharMapper(True)
	_mmseg.CharMapper_Load(char_map, "mm_cm.lib")

	rs = _mmseg.new_ushortp()
	print _mmseg.CharMapper_TransformScript(char_map, 655, rs)
	print _mmseg.ushortp_value(rs)      # Dereference
	_mmseg.delete_ushortp(rs)     # Delete
	_mmseg.delete_CharMapper(char_map)

if False:	# Test property
	dt =  _mmseg.new_BaseDict()
	schema = "id:4;attr2:2;name:s;attr1:2;attr3:2"
	_mmseg.BaseDict_InitString( dt, schema, len(schema) )
	# add entry
	_mmseg.BaseDict_Insert(dt, "abcdef", 100, 10, None, 0)
	msg = "hello world, abceef"
	_mmseg.BaseDict_SetProp(dt, 100, "name", msg, len(msg))
	c = _mmseg.get_dict_property_string(dt, 100, "name")
	print c

	#_mmseg.BaseDict_SetPropInteger(dt, 100, "id", 9999)
	#print _mmseg.get_dict_property_number(dt, 100, "id")

	_mmseg.delete_BaseDict(dt)
	#pass

if True:	# Test Dart
	dt =  _mmseg.new_BaseDict()
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
	_mmseg.BaseDict_SaveRaw(dt, 'd.lib')
	print _mmseg.BaseDict_Save(dt, 'd1.lib', 100)  # rev = 100
	# save
	# reload
	# query extract
	# query commonprefix
	_mmseg.delete_BaseDict(dt)
	# load
	dt =  _mmseg.new_BaseDict()
	print _mmseg.BaseDict_Load(dt, 'd1.lib', 'r')
	rs = _mmseg.new_DictMatchResult()
	print _mmseg.BaseDict_ExactMatch(dt, "abc", 3, rs)
	length = 0
	value = 0
	r = _mmseg.DictMatchResult_GetResult(rs)
	print r & 0xFFFFFFFF, r >> 32

	n = _mmseg.BaseDict_PrefixMatch(dt, "abcefg", 3, rs)
	for i in range(0, n):
		r = _mmseg.DictMatchResult_GetResult(rs, i)
		print r & 0xFFFFFFFF, r >> 32, '---'

	_mmseg.delete_DictMatchResult(rs)
	_mmseg.delete_BaseDict(dt)

#end of file
