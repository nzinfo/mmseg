#/usr/bin/python
# -*- coding: utf-8 -*-

import os
import sys
import codecs
from flask import Flask
from flask.ext.script import Manager
import mmseg


app = Flask(__name__)
# configure your app
manager = Manager(app)

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

def charmap_main(fname, unicode_script_fname = None, fname_dictname = 'charmap.uni' ):
    charmap = mmseg.CharMapDict()
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


    charmap.save(fname_dictname)
    #print charmap.trans(u'a')

@manager.option('-t', '--tolower', dest='tolower', default='data/tolower.txt')
@manager.option('-s', '--script', dest='script', default='data/Unidata/Script.txt')
@manager.option('-d', '--dict', dest='dict', default='charmap.uni')
def charmap(tolower, script, dict):
    charmap_main(tolower, script, dict)

@manager.option('-m', '--mmseg_source', dest='mmsegsource', default='data/unigram.txt')
@manager.option('-d', '--dict', dest='dict', default='mmseg.term')
def mmdict(mmsegsource, dict):
    schema = "id:4;freq:4"
    mmseg.basedict_mmseg_main("com.coreseek.mmseg.base", mmsegsource, dict, schema)

@manager.option('-m', '--cedict_source', dest='cesource', default='data/cedict_1_0_ts_utf-8_mdbg.zip')
@manager.option('-d', '--dict', dest='dict', default='cedict.term')
def cedict(cesource, dict):
    mmseg.basedict_cedict_main("mmseg.cedict", cesource, dict)


if __name__ == "__main__":
    manager.run()
    #unicode_script_fname = sys.argv[2]
    #main(sys.argv[1], unicode_script_fname, 't.lib')
    #print g_max_icode,'---'

    # test charmpa
    if False:
        charmap = mmseg.CharMapDict()
        charmap.load('t.lib')
        print charmap.trans(u'一')
        # check trans.
        print 0xF900, charmap.trans(u'\uF900'), 0x8C48
        charmap = None
# -*- end of file -*-