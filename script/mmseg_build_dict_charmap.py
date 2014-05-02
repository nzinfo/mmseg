# -*- coding: utf-8 -*-
#!/usr/bin/env python
import os
import sys
import codecs

pwd = os.path.abspath(os.getcwd())
mmseg_so_path = os.path.join(pwd, 'binding')
sys.path.insert(0, mmseg_so_path)

class CharMapDict():
    import _mmseg as mm
    _mmseg = mm
    def __init__(self):
        # if char not exist in map, char will pass though, this behave change on each load.
        self._char_map = CharMapDict._mmseg.new_CharMapper(True)
        self._tag_ptr = CharMapDict._mmseg.new_ushortp()

    def __del__(self):
        CharMapDict._mmseg.delete_ushortp(self._tag_ptr)
        CharMapDict._mmseg.delete_CharMapper(self._char_map)

    def def_map(self, src, det):
        CharMapDict._mmseg.CharMapper_Mapping(self._char_map, src, det, 0)
        pass

    def def_map_range(self, src_begin, src_end, det_begin, det_end):
        CharMapDict._mmseg.CharMapper_MappingRange(self._char_map, src_begin, src_end, det_begin, det_end)

    def def_map_pass(self, src):
        CharMapDict._mmseg.CharMapper_MappingPass(self._char_map, src)

    def def_map_pass_range(self, src_begin, src_end):
        CharMapDict._mmseg.CharMapper_MappingRangePass(self._char_map, src_begin, src_end)

    def def_tag(self, src, tag):
        CharMapDict._mmseg.CharMapper_Tag(self._char_map, src, tag)

    def save(self, fname):
        CharMapDict._mmseg.CharMapper_Save(self._char_map, fname)

    def load(self, fname):
        CharMapDict._mmseg.CharMapper_Load(self._char_map, fname)

    def trans(self, src):
        iCode = ord(src)
        n = CharMapDict._mmseg.CharMapper_TransformScript(self._char_map, iCode, self._tag_ptr)
        return n, CharMapDict._mmseg.ushortp_value(self._tag_ptr)

g_max_icode = 0

def get_icode(s):
    global g_max_icode

    v = 0
    if s.find('U+') == -1:
        try:
            v = ord(s)
        except Exception as ex:
            print ex, s, '----'
    else:
        s = s.replace('U+','0x')
        v = eval(s)

    if v > g_max_icode:
        g_max_icode = v
    return v

def main(fname, unicode_script_fname = None):
    charmap = CharMapDict()
    # do transform
    with codecs.open(fname, "r", "UTF-8") as fh:
        for line in fh:
            line = line.strip()
            if line[0] == '#':
                continue
            parse = line.strip().split(',')
            for p in parse:
                p = p.strip()
                if not p:
                    continue

                keyIdx = p.find('->')
                if keyIdx != -1:
                    leftK = p[:keyIdx]
                    rightK = p[keyIdx+2:]
                    if p.find('..') == -1:
                        # A -> a
                        charmap.def_map(get_icode(leftK), get_icode(rightK))
                        #print get_icode(leftK), get_icode(rightK)
                    else:
                        # A..Z -> a..z
                        lbegin = leftK.find('..')
                        srcbegin = get_icode( leftK[:lbegin].strip() )
                        srcend = get_icode( leftK[lbegin+2:].strip() )
                        #print srcbegin, srcend
                        lbegin = rightK.find('..')
                        detbegin = get_icode( rightK[:lbegin].strip() )
                        detend = get_icode( rightK[lbegin+2:].strip() )
                        charmap.def_map_range(srcbegin, srcend, detbegin, detend)
                    continue

                # pass through chars
                if p.find('..') == -1:
                    charmap.def_map_pass( get_icode(p) )
                else:
                    # process single char
                    lbegin = p.find('..')
                    srcbegin = get_icode( p[:lbegin].strip() )
                    srcend = get_icode( p[lbegin+2:].strip() )
                    charmap.def_map_pass_range(srcbegin, srcend)
            # end for parse
        # end for
    # end with
    if True:
        # custom mapping.
        trans_table = {}
        trans_table[ord(u'／')] = ord('/')
        trans_table[ord(u'￥')] = ord('$')
        trans_table[ord(u'＃')] = ord('#')
        trans_table[ord(u'％')] = ord('%')
        trans_table[ord(u'！')] = ord('!')
        trans_table[ord(u'＊')] = ord('*')
        trans_table[ord(u'（')] = ord('(')
        trans_table[ord(u'）')] = ord(')')
        trans_table[ord(u'－')] = ord('-')
        trans_table[ord(u'＋')] = ord('+')
        trans_table[ord(u'＝')] = ord('=')
        trans_table[ord(u'｛')] = ord('{')
        trans_table[ord(u'｝')] = ord('}')
        trans_table[ord(u'［')] = ord('[')
        trans_table[ord(u'］')] = ord(']')
        trans_table[ord(u'、')] = ord(',')
        trans_table[ord(u'｜')] = ord('|')
        trans_table[ord(u'；')] = ord(';')
        trans_table[ord(u'：')] = ord(':')
        trans_table[ord(u'‘')] = ord('\'')
        trans_table[ord(u'“')] = ord('"')
        trans_table[ord(u'《')] = ord('<')
        trans_table[ord(u'》')] = ord('>')
        trans_table[ord(u'〉')] = ord('<')
        trans_table[ord(u'〈')] = ord('>')
        trans_table[ord(u'？')] = ord('?')
        trans_table[ord(u'～')] =ord('~')
        trans_table[ord(u'｀')] =ord('`')
        for k, v in  trans_table.items():
            charmap.def_map(k, v)

    # do tagger
    if True:
        from unidata_script import UniDataScript, generate_tag_define
        d = UniDataScript()
        d.load_text(unicode_script_fname)
        ct = generate_tag_define(d._ct, "chartag_def.h")
        d.reset()
        d.load_text(unicode_script_fname, ct)
        # pollute
        for c in d._chars:
            charmap.def_tag(c, d._chars[c])
            #print c, d._chars[c]


    charmap.save('t.lib')
    print charmap.trans(u'a')

if __name__ == "__main__":

    unicode_script_fname = sys.argv[2]
    main(sys.argv[1], unicode_script_fname)
    #print g_max_icode,'---'

    # test
    charmap = CharMapDict()
    charmap.load('t.lib')
    print charmap.trans(u'一')
    print 0xF900, charmap.trans(u'\uF900'), 0x8C48
    charmap = None
# -*- end of file -*-