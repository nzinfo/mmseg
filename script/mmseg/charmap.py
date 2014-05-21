from .safe_mmseg import mmseg

class CharMapDict():
    _mmseg = mmseg
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

# -*- end of file -*-