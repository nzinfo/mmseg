# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
    处理 MMSEG 的文本词库到 二进制词库的转化
"""

import sys
import unicodedata
import re
import codecs
import os

pwd = os.path.abspath(os.getcwd())
mmseg_so_path = os.path.join(pwd, 'binding')
sys.path.insert(0, mmseg_so_path)


class BaseReader(object):
    _source_id = 0

    def __init__(self):
        self._terms = []

    def load(self, fname):
        pass

    def get_terms(self):
        """
            词条返回的格式
            ( term, freq, [pos, ] )
        """
        return self._terms

class MMSegTermReader(BaseReader):
    _source_id = 120
    """
        从MMSEG文本格式的词库中导入
    """
    def load(self, fname):
        self._terms = []
        #FIXME: check input file format.
        with codecs.open(fname, "r", "UTF-8") as fh:
            for line in fh:
                if line[:2] == 'x:':
                    continue
                term, freq = line.strip().split('\t')
                self._terms.append( \
                    (term, freq, [])
                    )
        return True


if __name__ == "__main__":
    import _mmseg

    fname = sys.argv[1]
    dict_fname = sys.argv[2]

    dt =  _mmseg.new_BaseDict()
    schema = "4:id;4:freq"
    _mmseg.BaseDict_InitString( dt, schema, len(schema) )

    reader = MMSegTermReader()
    reader.load(fname)
    i = 10
    for term in reader.get_terms():
        term_txt, freq , _ = term
        term_txt = term_txt.encode('utf-8')
        freq = int(freq)
        _mmseg.BaseDict_Insert(dt, term_txt, len(term_txt), i)
        # FIXME  set freq
        _mmseg.BaseDict_SetPropInteger(dt, i, "freq", freq)
        i += 1

    # FIXME:  if check file directly, will get a segment fault
    _mmseg.BaseDict_Save(dt, dict_fname, 1)  # rev: 1
    _mmseg.delete_BaseDict(dt)

    dt =  _mmseg.new_BaseDict()
    _mmseg.BaseDict_Load(dt, dict_fname, 'r')

    if True:
        term_txt = u"中国".encode('utf-8')
        v = _mmseg.BaseDict_ExactMatchScript( dt, term_txt, len(term_txt))
        print _mmseg.BaseDict_GetEntryPropertyU4(dt, v, "freq", 0)

    i = 10
    for term in reader.get_terms():
        # Check prooperty
        term_txt, freq , _ = term
        term_txt = term_txt.encode('utf-8')

        #v = _mmseg.BaseDict_ExactMatchScript( dt, term_txt, len(term_txt))
        #print term_txt, v
        #print _mmseg.BaseDict_GetEntryPropertyU4(dt, v, "freq", 0)
        i += 1

    term_txt = "lkdfljas;fjkl;dsajfks;"
    #v = _mmseg.BaseDict_ExactMatchScript( dt, term_txt, len(term_txt))
    #print term_txt, v
    _mmseg.delete_BaseDict(dt)

# -*- end of file -*-