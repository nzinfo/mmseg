md libcss
md libcss\include

copy src\*.h libcss\include\
copy src\css\*.h libcss\include\
copy src\utils\*.h libcss\include\

md libcss\lib
md libcss\lib\x64

md libcss\lib\debug
md libcss\lib\release

md libcss\lib\x64\debug
md libcss\lib\x64\release

copy win32\release\*.lib libcss\lib\release\
copy win32\debug\*.lib libcss\lib\debug\

copy win32\x64\release\*.lib libcss\lib\x64\release\
copy win32\x64\debug\*.lib libcss\lib\x64\debug\