# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
	用于测试 swig 版本的 MMSEG 接口
"""
import os
import sys

pwd = os.path.abspath(os.getcwd())
mmseg_so_path = os.path.join(pwd, 'bin')
#print mmseg_so_path
sys.path.insert(0, mmseg_so_path)

# try import
import _mmseg

print dir(_mmseg)


#end of file
