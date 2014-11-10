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

copy win32\release\libcss.lib libcss\lib\libcss.lib
copy win32\debug\libcss.lib libcss\lib\libcss_d.lib

copy win32\x64\release\*.lib libcss\lib\x64\release\
copy win32\x64\debug\*.lib libcss\lib\x64\debug\