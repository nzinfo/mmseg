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

def enmu_all_keys(key):
    """
        根据输入字符串，得到全部可用的组合
    """
    kl = []
    if len(key)==0:
        return kl
    prefix = ''
    for c in key:
        prefix = prefix + c
        if len(prefix) == 1:
            continue
        kl.append(prefix)
    kl2 = enmu_all_keys(key[1:])
    return kl + kl2

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
    chars = read_char_freq(char_freq_fname)
    terms = read_terms(term_fname)

    #print len(chars), len(terms)
    # 构造实际的词表
    ht = {}
    for c in chars:
        ht[c] = chars[c] # 单字，记录字的频率
    for t in terms:
        if t not in ht:
            ht[t] = 1

    # 处理 子串
    for k in ht:
        if len(k) == 1:
            continue
        subk = {}
        kl = enmu_all_keys(k)
        # 遍历全部的子串，检查是否在词库内。
        for sk in kl:
            if sk != k and ht.has_key(sk):
                subk[sk] = 1
        ht[k] = subk
    keys = sorted(ht.keys())
    # check output
    out_fname = None
    if len(sys.argv) == 4:
        out_fname = sys.argv[3]
        origin_stdout = sys.stdout
        sys.stdout = open(out_fname, 'w+')

    for k in keys:
        if len(k) == 1:
            cnt = ht[k]
            print (k+'\t'+str(cnt)).encode('UTF-8')
            print ('x:'+str(cnt)).encode('UTF-8')
            continue
        else:
            print (k+'\t1').encode('UTF-8') # 目前系统不考虑 term 的词频
            if type(ht[k]) == dict and len(ht[k]):
                v = ';'.join(sorted(ht[k].keys()))
                print "t:"+v.encode('UTF-8')
            else:
                print ('x:1').encode('UTF-8')
            #print (''+str(cnt)).encode('UTF-8')
    # end for


if __name__ == "__main__":
     main()

# -*- end of file -*-