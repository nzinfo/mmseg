# -*- coding: utf-8 -*-
#!/usr/bin/env python
import os
import sys
import codecs

"""
    从原始文本中生成 ungram2.txt ，与之前版本，额外增加 公共子串（同义词）

    用法：
    build_unigram2.txt  term.txt char.stat.txt unigram2.txt
    if not exist arg3, output to stdout
"""


def read_terms(fname):
    terms = {}
    with codecs.open(fname, "r", "UTF-8") as fh:
        for l in fh:
            l = l.strip()
            if l not in terms:
                terms[l] = 1
    return terms

def read_char_freq(fname):
    uni_char = {}
    with codecs.open(fname, "r", "UTF-8") as fh:
        for l in fh:
            l = l.strip()
            toks = l.split('\t')
            k = toks[0]
            cnt = int(toks[1])

            if k not in uni_char:
                uni_char[k] = cnt
    return uni_char

def main():
    term_fname = sys.argv[1]
    char_freq_fname = sys.argv[2]
    
	fh = codecs.open(sys.argv[1],"r", "UTF-8")
	lines = fh.readlines()
	fh.close()
	uni_char = {}
	for l in lines:
		l = l.strip()
		toks = l.split('\t')
		k = toks[0]
		cnt = int(toks[1])
		if k not in uni_char:
			uni_char[k] = cnt
	fh = codecs.open(sys.argv[2],"r", "UTF-8")
	lines = fh.readlines()
	fh.close()

# -*- end of file -*-