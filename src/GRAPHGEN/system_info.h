// Copyright (c) 2020, the GRAPHGEN contributors, as
// shown by the AUTHORS file. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef GRAPHGEN_SYSTEM_INFO_H_
#define GRAPHGEN_SYSTEM_INFO_H_

/** @file system_info.h

This file contains a bunch of defines to deal with OS dependent functionalities. 

 */

#if _WIN32 || _WIN64 || WIN32 || __WIN32__ || __WINDOWS__ || __TOS_WIN__
// #ifdef _MSC_VER
// #include <intrin.h>
// #endif
// #ifndef NOMINMAX
// #define NOMINMAX // Prevent <Windows.h> header file defines its own macros named max and min
// #endif
// #include <WINDOWS.h>
// #include <lm.h>
// #pragma comment(lib, "netapi32.lib")
#define GRAPHGEN_WINDOWS
#elif  __gnu_linux__ || __linux__
#define GRAPHGEN_LINUX
//#include <sys/utsname.h>
#elif  __unix || __unix__
#define GRAPHGEN_UNIX
//#include <sys/utsname.h>
#elif __APPLE__ || __MACH__ || macintosh || Macintosh || (__APPLE__ && __MACH__)
//#include <sys/types.h>
//#include <sys/sysctl.h>
#define GRAPHGEN_APPLE
#endif


#endif // !GRAPHGEN_SYSTEM_INFO_H_