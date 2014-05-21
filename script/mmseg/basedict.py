#/usr/bin/python
# -*- coding: utf-8 -*-
"""
    构造基本词典的接口 delete_DictBase
"""
import codecs
from .safe_mmseg import mmseg

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


class BaseDict():
    _mmseg = mmseg

    def __init__(self):
        # if char not exist in map, char will pass though, this behave change on each load.
        self._dt = BaseDict._mmseg.new_DictBase()
        #self._tag_ptr = BaseDict._mmseg.new_ushortp()

    def __del__(self):
        #BaseDict._mmseg.delete_ushortp(self._tag_ptr)
        BaseDict._mmseg.delete_DictBase(self._dt)

    def Init(self, dict_name, schema = None):
        if not schema:
            schema = "4:id;4:freq"
        BaseDict._mmseg.DictBase_Init(self._dt, dict_name, schema )

    def AddItem(self, term , props):
        pass

    def Save(self, dict_fname, rev):
        pass

def basedict_main(dict_name, fsource, dict_fname, schema):
    d = BaseDict()
    #schema = "4:id;4:freq"
    d.Init(dict_name, schema )

    reader = MMSegTermReader()
    reader.load(fsource)
    i = 10

    for term in reader.get_terms():
        term_txt, freq , _ = term
        term_txt = term_txt.encode('utf-8')
        freq = int(freq)
        d.AddItem(term_txt, {})

    d.Save(dict_fname, 1)  # rev: 1

# -*- end of file -*-