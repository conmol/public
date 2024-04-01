//======================================================================
// Header File: platform_utilities.h
// Author: Bill Hallahan
// Date: March 3, 2024
//
// Copyright (C) 2024 William Hallahan
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

// If not on Linux, then make this file appear empty.

#ifndef PLATFORM_UTILITIES_H
#define PLATFORM_UTILITIES_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// The following function is only needed on Linux.
#ifdef __linux__

int _tfopen_s(FILE ** file_ptr_ptr,
              const char * fname_ptr,
              const char * mode_ptr);

#endif // __linux__

// Read environment variable "DBNPATH" to get the path
// to the 64-bit binary sequence files.
int plt_read_env_var(const TCHAR * env_var_name_ptr,
                     TCHAR * buffer_ptr,
                     int buffer_length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PLATFORM_UTILITIES_H

