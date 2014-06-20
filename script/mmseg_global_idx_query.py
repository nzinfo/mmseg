#/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import win32_unicode_argv  # only import on win32, should check ...
import mmseg

"""
   用于从全局的词库索引中查询数据
   - 返回：
        词在那些词库中存在， 对应的 偏移量
        (目前，只返回词库编号 | 偏移量)
   - 可以直接根据偏移量查询具体的字段的值。
"""
if __name__ == "__main__":
    global_idxdict_fname = sys.argv[1]
    key = sys.argv[2]
    prop = sys.argv[3]

    # check key
    if type(key) == unicode:
        key = key.encode('utf-8')
        global_idxdict_fname = global_idxdict_fname.encode('utf-8')

    if True:
        dict_obj = mmseg.BaseDict()
        dict_obj.Load(global_idxdict_fname)
        entry_offset = dict_obj.Match(key)
        entries_data = dict_obj.GetString(entry_offset, "entries")

        print entries_data
        if entries_data:
            print entry_offset, mmseg.decode_global_entries(entries_data, len(key))
        else:
            print entry_offset
        # try set & get dag & seg
        print dict_obj.GetSchemaDefine()
        print dict_obj.SetU4(entry_offset, "dag", 1000)
        print dict_obj.GetU4(entry_offset, "dag")
        #print dict_obj.GetString(entry_offset, "thes")
        #print entry_offset

# -*- end of file -*-