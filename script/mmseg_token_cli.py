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

if __name__ == "__main__":
    dict_mgr = mmseg.DictionaryManager()
    dict_mgr.load(".", "../lua")
    #print(dir(mmseg))
    if False:
        for item in mmseg.Tokenizer(dict_mgr, 1, u"清理完最后一堆垃圾电子书，我算对整个出版行业在电子书上的投入和态度"
                                                 u"有了全面的了解。不想说太多，太坑爹了，说出来全是泪。"
                                                 u"但有一条：未来只会对真正愿意在数字出版道路上一起前行的优质资源提供者，投以回报。"):
            print item,
        print ''

    for item in mmseg.Tokenizer(dict_mgr, 1, u"AK-47是一款被广泛使用的Russian枪支，发明于1946年2月28日。正品CASTEL酒庄酒的标识如图，hello."):
        print item,

    # check exatcmatch.
    if True:
        key = u"AK-47"
        print dict_mgr.match(key)

    for dict_name in dict_mgr.dictionary_names("term"):
        print dict_mgr.dictionary_schema(dict_name)

# -*- end of file -*-