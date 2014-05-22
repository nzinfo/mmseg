# -*- coding: utf-8 -*-
#!/usr/bin/env python
import sys
from .basedict import BaseDict, BaseReader
from cjklib.build.builder import CEDICTBuilder
from cjklib import dbconnector

def init_cedict():
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


class CEDictReader(BaseReader):
    _source_id = 0

    def __init__(self):
        self._terms = []

    def load(self, fname):
        """
            加载 CEDict ，此处需要特别 处理 繁简转换，此词库是目前唯一的包括繁体词条的。
        """
        db = init_cedict()
        builder = CEDICTBuilder(dbConnectInst=db, useCollation = False, filePath=fname)
        generator = builder.getGenerator()
        keys = {}
        for newEntry in generator:
            simp_key, trad_key, reading = newEntry['HeadwordSimplified'], newEntry['HeadwordTraditional'], newEntry['Reading']
            #print simp_key
            simp_key = simp_key.encode('utf-8').strip()
            trad_key = trad_key.encode('utf-8').strip()
            reading = reading.encode('utf-8').strip()
            keys[trad_key] = 1
            if trad_key != simp_key:
                self._terms.append(
                    (trad_key, 1, [reading, simp_key])
                )
            else:
                self._terms.append(
                    (trad_key, 1, [reading, None])
                )
            if simp_key not in keys:
                keys[simp_key] = 1
                self._terms.append(
                    (simp_key, 1, [reading, None])
                )

    def get_terms(self):
        """
            词条返回的格式
            ( term, freq, [pos, ] )
        """
        return self._terms


def basedict_cedict_main(dict_name, fsource, dict_fname):
    d = BaseDict()
    # schema = "id:4;freq:4"
    schema = "id:4;freq:4;simp:s;pinyin:s"   # keep id == 0 for future enhance.
    d.Init(dict_name, schema)

    reader = CEDictReader()
    reader.load(fsource)

    for term in reader.get_terms():
        term_txt, freq, props = term  # props is a vector of props value, each reader are diff.
        term_txt = term_txt.encode('utf-8')
        freq = int(freq)
        #print term_txt, type(term_txt)
        if props[1]:
            d.AddItem(term_txt, {"freq": freq, 'pinyin':props[0], 'simp':props[1]})
        else:
            d.AddItem(term_txt, {"freq": freq, 'pinyin':props[0] })

    d.Save(dict_fname, 1)  # rev: 1
    # check load
    #d = BaseDict()
    #d.Load(dict_fname)
    print d.Match(u"中国".encode('utf8'))

# -*- end of file -*-