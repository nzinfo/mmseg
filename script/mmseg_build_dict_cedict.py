# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
    读取 CEDICT 文件， 创建 MMSeg 词库
    - 词库格式
        term, simp, pinyin  simp 仅当词库中词条为繁体， 且其与简体不同 出现
"""
from cjklib.build.builder import CEDICTBuilder
from cjklib import dbconnector

def init():
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
        print configuration
        db = dbconnector.DatabaseConnector(configuration)
    except ValueError, e:
        print >> sys.stderr, "Error: %s" % e
        return False
    return db

if __name__ == "__main__":
    import sys
    fname = sys.argv[1]
    db = init()
    # pass db make cjklib based code happy, we don't needs database.
    builder = CEDICTBuilder(dbConnectInst=db, useCollation = False, filePath=fname)
    generator = builder.getGenerator()
    for newEntry in generator:
        print newEntry

# -*- end of file -*-