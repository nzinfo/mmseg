# -*- coding: utf-8 -*-
#!/usr/bin/env python
import os
import sys
from .safe_mmseg import mmseg
#print dir(mmseg)

class DictionaryManager(object):
    _mmseg = mmseg

    def __init__(self):
        # if char not exist in map, char will pass though, this behave change on each load.
        self._dm = DictionaryManager._mmseg.new_DictMgr()
        self._ss = DictionaryManager._mmseg.new_SegScript()

    def __del__(self):
        DictionaryManager._mmseg.delete_DictMgr(self._dm)
        DictionaryManager._mmseg.delete_SegScript(self._ss)

    # load
    def load(self, dict_path, script_path = None):
        """
            在 C 风格的 API 中，区分 单字词典、 系统词典、 行业词典， 在 Python 中，不进行详细区分
        """
        # load all the dict
        DictionaryManager._mmseg.DictMgr_LoadTerm(self._dm, dict_path)
        DictionaryManager._mmseg.DictMgr_LoadPharse(self._dm, dict_path)
        DictionaryManager._mmseg.DictMgr_LoadSpecial(self._dm, dict_path)

        s_idx_cache_fname = os.path.join(dict_path, ".term_idx")

        rs = DictionaryManager._mmseg.DictMgr_LoadIndexCache(self._dm, s_idx_cache_fname)
        if rs < 0:
            DictionaryManager._mmseg.DictMgr_BuildIndex(self._dm, True)
            DictionaryManager._mmseg.DictMgr_SaveIndexCache(self._dm, s_idx_cache_fname)
        else:
            #print 'use cache'
            DictionaryManager._mmseg.DictMgr_BuildIndex(self._dm, False)

        # load script.
        if script_path:
            s_error = ""
            rs = DictionaryManager._mmseg.SegScript_LoadScripts(self._ss, script_path)
            # FIXME: check rs.
            print 'script load', rs, DictionaryManager._mmseg.SegScript_GetErrorMessage(self._ss)

    def match(self, term):
        """
            查询词的命中
        """
        utf8_term = term
        if type(term) == unicode:
            utf8_term = term.encode('utf-8')

        matches = DictionaryManager._mmseg.new_DictMatchResult()
        try:
            # do segment.
            rs = DictionaryManager._mmseg.DictMgr_ExactMatch(self._dm, utf8_term, len(utf8_term), matches)
            return rs
        finally:
            # free for all
            DictionaryManager._mmseg.delete_DictMatchResult(matches)

    def dictionary_names(self, category):
        if category in ["term", "pharse", "special"]:
            dict_names_str = DictionaryManager._mmseg.DictMgr_GetDictionaryNames(self._dm, category)
            return filter(lambda x: x, dict_names_str.split(';') )
        return []

    def dictionary_schema(self, dict_name):
        dict_obj = DictionaryManager._mmseg.DictMgr_GetDictionary(self._dm, dict_name)
        if dict_obj:
            schema_def_s = DictionaryManager._mmseg.DictBase_GetColumnDefine( dict_obj )
        return schema_def_s

class Tokenizer(object):
    """
        实际处理切分的类， 实现为一个迭代器。 
    """
    _mmseg = mmseg

    def __init__(self, dict_mgr, task_id, txt):
        # check txt
        uni_txt = txt
        utf8_txt = txt
        if type(txt) == str:
            uni_txt = txt.decode('utf-8')
        if type(txt) == unicode:
            utf8_txt = txt.encode('utf-8')
        utf8_txt_len = len(utf8_txt)

        self._task = task_id
        self._txt  = "  %s  " % uni_txt     # add the prefix 2 char.
        # fetch result.
        self._rs = DictionaryManager._mmseg.new_SegmentorResultReaderScript()
        self._seg_opt = DictionaryManager._mmseg.new_SegOptions("", "")     # annotes, special_dict_name
        self._seg_stat = DictionaryManager._mmseg.new_SegStatus(self._seg_opt)
        self._seg = DictionaryManager._mmseg.new_Segmentor(dict_mgr._dm, dict_mgr._ss)

        # do seg.1st pass
        rs = DictionaryManager._mmseg.Segmentor_Tokenizer(self._seg, task_id, utf8_txt, utf8_txt_len, self._seg_stat)
        DictionaryManager._mmseg.Segmentor_GetResult(self._seg, self._rs, self._seg_stat)

        # self._idx = 0  # skip the first 2 char.
        self._idx = 2
        self._seg_base = 0
        self._items_len = DictionaryManager._mmseg.SegmentorResultReaderScript_iCodeLength(self._rs)
        #self._items = [1, 2, 4, 6]

    def __del__(self):
        DictionaryManager._mmseg.delete_SegmentorResultReaderScript(self._rs)
        DictionaryManager._mmseg.delete_SegOptions(self._seg_opt)
        DictionaryManager._mmseg.delete_SegStatus(self._seg_stat)
        DictionaryManager._mmseg.delete_Segmentor(self._seg)

    def __iter__(self):
        return self

    def next(self):
        """
          实际可以额外输出 Annote，目前不处理。
        """
        token = ""
        annote_cnt = DictionaryManager._mmseg.new_ushortp()

        if self._idx >= self._items_len:
            # FIXME: move next block.
            raise StopIteration

        while self._idx < self._items_len:
            idx = self._idx
            char_tag = DictionaryManager._mmseg.SegmentorResultReaderScript_Char(self._rs, idx, annote_cnt)
            #print self._seg_base + self._idx, len(self._txt), self._items_len
            token += self._txt[self._seg_base + self._idx]
            self._idx += 1
            if char_tag == 'E' or char_tag == 'S':
                break

        #print char_tag, DictionaryManager._mmseg.ushortp_value(annote_cnt)
        DictionaryManager._mmseg.delete_ushortp(annote_cnt)

        return token

# -*- end of file -*-