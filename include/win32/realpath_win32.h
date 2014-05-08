#ifndef REALPATH_WIN32_H
#define REALPATH_WIN32_H

#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)

#endif // REALPATH_WIN32_H
