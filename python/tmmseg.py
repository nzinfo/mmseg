# -*- coding: utf-8 -*-
import os
import cmmseg
#cmmseg.init('F:\\deps\\mmseg\\src\\win32')
dict_path = os.path.abspath(os.path.join("..", "win32"))
print "load dictionary ", dict_path

seg = cmmseg.MMSeg(dict_path)
rs = seg.segment((u'中文分词').encode('utf-8'))
for i in rs:
    print i.decode('utf-8')

for i in seg.thesaurus((u'一丁点儿').encode('utf-8')):
	print i
print '-------'