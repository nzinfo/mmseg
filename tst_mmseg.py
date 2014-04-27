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

#end of file
