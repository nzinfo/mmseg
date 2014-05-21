import os
import sys

try:
    import _mmseg
except:
    # debug mode
    pwd = os.path.abspath(os.getcwd())
    mmseg_so_path = os.path.join(pwd, 'bin')
    sys.path.insert(0, mmseg_so_path)
    mmseg_so_path = os.path.join(pwd, 'bin', 'RelWithDebInfo')
    sys.path.insert(0, mmseg_so_path)
    print mmseg_so_path
    import _mmseg

mmseg = _mmseg