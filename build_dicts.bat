python ..\script\mmseg_cli.py charmap -t ..\data\tolower.txt -s ..\data\unidata\Scripts.txt -d charmap.uni
python ..\script\mmseg_cli.py mmdict -m ..\data\unigram.txt -d mmseg.term
python ..\script\mmseg_cli.py cedict -m ..\data\cedict_1_0_ts_utf-8_mdbg.zip -d cedict.term

REM \script\mmseg_cli.py charmap -t