//======================================================================
// Header File: platform_os.h
// Author: Bill Hallahan
// Date: September 26, 2013
//
// Abstract:
//
//    Platform and operating system specific definitions.
//
// Copyright (c) 2013-2024, William Hallahan.
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
//======================================================================

#ifndef PLATFORM_OS_H
#define PLATFORM_OS_H

// Common header files.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// More Windows definitions.
#ifdef _WIN64
#include <tchar.h>

#define ASSERT(EXPR) assert(EXPR)

#ifdef UNICODE
#include <wctype.h>
#define CHAR_TYPE wchar_t
#define ISALPHA(P) iswalpha(P)
#define ISDIGIT(P) iswdigit(P)
#else  /* Not UNICODE */
#include <ctype.h>
#define CHAR_TYPE TCHAR
#define FOPEN(NM, MD) fopen(NM, MD)
#define ISALPHA(P) isalpha(P)
#define ISDIGIT(P) isdigit(P)
#endif /* UNICODE */

#define PLT_PATH_SEP_CHAR _T('\\');

#endif /* _WIN64 */

#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>

#define TCHAR char
#define CHAR_TYPE char

#define ASSERT(EXP)

#define SET_STDOUT_MODE
#define STD_COUT std::cout
#define _T(X) X
#define _tmain main
#define _tprintf printf
#define _tcslen strlen
#define _tcscpy strcpy
#define _tcscmp strcmp
#define _tcschr strchr
#define _tcsstr strstr
#define _tcstol strtol
#define _tcstod strtod
#define _stat64 stat
#define _tstat64 stat
#define _ftprintf fprintf
#define _istdigit isdigit

#define _tcscpy_s(DEST, SIZE, SRC) strcpy(DEST, SRC)
#define _tcscat_s(DEST, SIZE, SRC) strcat(DEST, SRC)

#define _fgetts(AR, SZ, FS) fgets(AR, SZ, FS)

#define ISALPHA(P) isalpha(P)
#define ISDIGIT(P) isdigit(P)

#define PLT_PATH_SEP_CHAR '/';

#endif /* __linux__ */

// Some common definitions.
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#define UNUSED_VARIABLE(VARNAME) (void)(VARNAME)

typedef int BOOL;

#endif /* #ifndef PLATFORM_OS_H */
