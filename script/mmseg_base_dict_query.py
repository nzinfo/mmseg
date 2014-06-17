#/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import win32_unicode_argv  # only import on win32, should check ...
import mmseg

"""
    用于从 词典文件中查询 属性, 必须是字符串属性
"""
if __name__ == "__main__":
    dict_fname = sys.argv[1]
    key = sys.argv[2]
    prop = sys.argv[3]

    # check key
    if type(key) == unicode:
        key = key.encode('utf-8')
        dict_fname = dict_fname.encode('utf-8')

    if True:
        dict_obj = mmseg.BaseDict()
        dict_obj.Load(dict_fname)
        entry_offset = dict_obj.Match(key)
        print dict_obj.GetString(entry_offset, "thes")
        print entry_offset

# -*- end of file -*-