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
                line = line.strip()
                if line[:2] == 'x:':
                    continue
                if line[:2] == 't:':
                    # 需要处理同义词
                    self._terms[-1][2]['thes'] = line[2:].encode('utf8')
                    continue
                #print line
                term, freq = line.strip().split('\t')
                self._terms.append( \
                    (term, freq, {})
                    )
        return True


class BaseDict():
    _mmseg = mmseg

    def __init__(self):
        # if char not exist in map, char will pass though, this behave change on each load.
        self._dt = BaseDict._mmseg.new_DictBase()
        self._entry_helper = BaseDict._mmseg.new_EntryDataWrap()
        #self._tag_ptr = BaseDict._mmseg.new_ushortp()
        self._set_prop_funcs  = {
            '2': BaseDict._mmseg.EntryDataWrap_SetU2,
            '4': BaseDict._mmseg.EntryDataWrap_SetU4,
            '8': BaseDict._mmseg.EntryDataWrap_SetU8,
        }
        #print dir(BaseDict._mmseg)
        self.i = 0
        self._find = 0

    def __del__(self):
        #BaseDict._mmseg.delete_ushortp(self._tag_ptr)
        BaseDict._mmseg.delete_DictBase(self._dt)
        BaseDict._mmseg.delete_EntryDataWrap(self._entry_helper)

    def Init(self, dict_name, schema = None):
        if not schema:
            schema = "id:4;freq:4;thes:s"
        BaseDict._mmseg.DictBase_Init(self._dt, dict_name, schema)
        #dict_schema = BaseDict._mmseg.DictBase_GetSchema(self._dt)
        #print dir(dict_schema)

    def GetSchemaDefine(self):
        return BaseDict._mmseg.DictBase_GetColumnDefine(self._dt)

    def AddItem(self, term, props):
        """
            props is
        """
        self.i += 1
        entry = BaseDict._mmseg.DictBase_Insert(self._dt, term, len(term))
        if entry == None:
            print term, entry , "term dup."
            return

        if False:
            if term == u"中国".encode('utf8') :
                props['freq'] = 2008

        for k, v in props.items():
            column_type = BaseDict._mmseg.DictBase_SchemaColumnType(self._dt, k)
            if column_type == 's':
                BaseDict._mmseg.EntryDataWrap_SetData(self._entry_helper, entry, BaseDict._mmseg.DictBase_GetSchema(self._dt),
                                                        BaseDict._mmseg.DictBase_GetStringPool(self._dt), k, v, len(v) )
            elif column_type in self._set_prop_funcs:
                self._set_prop_funcs[column_type](self._entry_helper, entry, BaseDict._mmseg.DictBase_GetSchema(self._dt), k, v)
            else:
                print k, v, 'column type not found.'
        # end for
        if False:
            if term == u"中国".encode('utf8') or (self._find and self.i % 10000 == 0):
                print props
                s = u"中国".encode('utf8')
                self._find = 1
                #entry = BaseDict._mmseg.DictBase_GetEntryDataByOffset(self._dt, entry_offset)
                print BaseDict._mmseg.EntryDataWrap_GetU4(self._entry_helper, entry, BaseDict._mmseg.DictBase_GetSchema(self._dt), "freq", 0)
                entry_offset =  BaseDict._mmseg.DictBase_GetEntryOffset(self._dt, s, len(s))
                entry = BaseDict._mmseg.DictBase_GetEntryDataByOffset(self._dt, entry_offset)
                print entry_offset, BaseDict._mmseg.EntryDataWrap_GetU4(self._entry_helper, entry, BaseDict._mmseg.DictBase_GetSchema(self._dt), "freq", 0)

    def Save(self, dict_fname, rev):
        BaseDict._mmseg.DictBase_Save(self._dt, dict_fname, rev)

    def Load(self, dict_fname):
        BaseDict._mmseg.DictBase_Load(self._dt, dict_fname)

    def Match(self, s):
        entry_offset = BaseDict._mmseg.DictBase_ExactMatch(self._dt, s, len(s))
        return entry_offset

        if False:
            print entry_offset
            entry = BaseDict._mmseg.DictBase_GetEntryDataByOffset(self._dt, entry_offset)
            print BaseDict._mmseg.EntryDataWrap_GetU4(self._entry_helper, entry, BaseDict._mmseg.DictBase_GetSchema(self._dt), "freq", 0)

    def GetString(self, entry_offset, prop):
        entry = BaseDict._mmseg.DictBase_GetEntryDataByOffset(self._dt, entry_offset)
        return BaseDict._mmseg.get_dict_property_string(self._dt, entry, prop);

    def GetU4(self, entry_offset, prop):
        entry = BaseDict._mmseg.DictBase_GetEntryDataByOffset(self._dt, entry_offset)
        return BaseDict._mmseg.EntryDataWrap_GetU4(self._entry_helper, entry, BaseDict._mmseg.DictBase_GetSchema(self._dt), prop, 0)

    def SetU4(self, entry_offset, prop, u4_v):
        entry = BaseDict._mmseg.DictBase_GetEntryDataByOffset(self._dt, entry_offset)
        return BaseDict._mmseg.EntryDataWrap_SetU4(self._entry_helper, entry, BaseDict._mmseg.DictBase_GetSchema(self._dt), prop, u4_v)


def basedict_mmseg_main(dict_name, fsource, dict_fname, schema):
    d = BaseDict()
    #schema = "4:id;4:freq"
    d.Init(dict_name, schema )

    reader = MMSegTermReader()
    reader.load(fsource)
    i = 10

    for term in reader.get_terms():
        term_txt, freq, term_props = term
        term_txt = term_txt.encode('utf-8')
        freq = int(freq)
        #print term_txt, type(term_txt)
        props = {"freq": freq}
        if 'thes' in term_props:
            props['thes'] = term_props['thes']
        d.AddItem(term_txt, props )

    d.Save(dict_fname, 1)  # rev: 1

    # check load
    d = BaseDict()
    d.Load(dict_fname)
    d.Match(u"中国".encode('utf8'))

# -*- end of file -*-