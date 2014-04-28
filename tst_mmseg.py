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
	schema = "id:4;attr2:2;name:s;attr1:2;attr3:2"
	_mmseg.BaseDict_InitString( dt, schema, len(schema) )
	# add entrys
	_mmseg.BaseDict_Insert(dt, "a", 100, 10)
	_mmseg.BaseDict_Insert(dt, "abc", 101, 10)
	_mmseg.BaseDict_Insert(dt, "abcd", 102, 10)
	_mmseg.BaseDict_Insert(dt, "ab", 103, 10)
	_mmseg.BaseDict_Insert(dt, "abgaowei", 104, 10)
	_mmseg.BaseDict_Insert(dt, u"选".encode('utf-8'), 105, 10)
	_mmseg.BaseDict_Insert(dt, u"选择".encode('utf-8'), 106, 10)
	_mmseg.BaseDict_Insert(dt, u"选择题".encode('utf-8'), 107, 10)
	# build darts
	_mmseg.BaseDict_Build(dt)
	_mmseg.BaseDict_SaveRaw(dt, 'd.lib')
	# save
	# reload
	# query extract
	# query commonprefix

	_mmseg.delete_BaseDict(dt)

#end of file
