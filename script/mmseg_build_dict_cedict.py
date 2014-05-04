# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
    读取 CEDICT 文件， 创建 MMSeg 词库
    - 词库格式
        term, simp, pinyin  simp 仅当词库中词条为繁体， 且其与简体不同 出现
"""
import sys
import unicodedata
import re
import codecs
import os

from cjklib.build.builder import CEDICTBuilder
from cjklib import dbconnector

pwd = os.path.abspath(os.getcwd())
mmseg_so_path = os.path.join(pwd, 'binding')
sys.path.insert(0, mmseg_so_path)

def init():
    # get database connection
    options = {'databaseUrl':'sqlite:///./test.db'}
    configuration = dbconnector.getDefaultConfiguration()
    configuration['sqlalchemy.url'] = options.pop('databaseUrl',
        configuration['sqlalchemy.url'])
    configuration['attach'] = [attach for attach in
        options.pop('attach', configuration.get('attach', [])) if attach]
    if 'registerUnicode' in options:
        configuration['registerUnicode'] = options.pop('registerUnicode')
    try:
        db = dbconnector.DatabaseConnector(configuration)
    except ValueError, e:
        print >> sys.stderr, "Error: %s" % e
        return False
    return db

if __name__ == "__main__":
    import _mmseg
    import sys

    fname = sys.argv[1]
    dict_fname = sys.argv[2]
    db = init()

    dt = _mmseg.new_BaseDict()
    schema = "4:id;4:freq;s:simp;s:pinyin"
    _mmseg.BaseDict_InitString(dt, schema, len(schema))

    # pass db make cjklib based code happy, we don't needs database.
    builder = CEDICTBuilder(dbConnectInst=db, useCollation = False, filePath=fname)
    generator = builder.getGenerator()
    #reads = {}
    i = 10
    keys = {}
    for newEntry in generator:
        simp_key, trad_key, reading = newEntry['HeadwordSimplified'], newEntry['HeadwordTraditional'], newEntry['Reading']
        #print simp_key
        simp_key = simp_key.encode('utf-8').strip()
        trad_key = trad_key.encode('utf-8').strip()
        reading = reading.encode('utf-8').strip()

        keys[trad_key] = 1  #  FIXME: check dup in cext  ?
        _mmseg.BaseDict_Insert(dt, trad_key, len(trad_key), i)
        #_mmseg.BaseDict_SetPropInteger(dt, i, "freq", i)
        _mmseg.BaseDict_SetProp(dt, i, "pinyin", reading, len(reading))
        #print simp_key, trad_key, trad_key != simp_key
        if trad_key != simp_key:
            _mmseg.BaseDict_SetProp(dt, i, "simp", simp_key, len(simp_key))
            if simp_key not in keys:
                i += 1
                _mmseg.BaseDict_Insert(dt, simp_key, len(simp_key), i)
                _mmseg.BaseDict_SetProp(dt, i, "pinyin", reading, len(reading))
                keys[simp_key] = 1
        i += 1

    _mmseg.BaseDict_Save(dt, dict_fname, 1)  # rev: 1
    _mmseg.delete_BaseDict(dt)

    dt =  _mmseg.new_BaseDict()
    _mmseg.BaseDict_Load(dt, dict_fname, 'r')
    if True:
        term_txt = u"中国".encode('utf-8')
        v = _mmseg.BaseDict_ExactMatchScript( dt, term_txt, len(term_txt))
        print term_txt, v
        #print _mmseg.BaseDict_GetEntryPropertyU4(dt, v, "freq", 0)
        print _mmseg.get_dict_property_string_by_value(dt, v, "pinyin");
        print v

    _mmseg.delete_BaseDict(dt)
    #print reads
    #print len(reads)

# -*- end of file -*-