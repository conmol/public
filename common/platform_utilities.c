//======================================================================
// File: platform_utilities.c
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

#ifdef _WIN64
#include <windows.h>
#endif

#include "platform_os.h"
#include "platform_utilities.h"

// Linux functions.
#ifdef __linux__

// The "char" version of _tfopen_s, which is fopen_s
// does not exist on Linux platforms.
int _tfopen_s(FILE ** file_ptr_ptr,
              const char * fname_ptr,
              const char * mode_ptr)
{
    ASSERT(file_ptr_ptr != NULL);
    ASSERT(fname_ptr != NULL);
    ASSERT(mode_ptr != NULL)

    int status = 0;
    errno = 0;

    *file_ptr_ptr = fopen(fname_ptr, mode_ptr);

    if (*file_ptr_ptr == NULL)
    {
        status = errno;
        errno = 0;
    }

    return status;
}

// Read the contents of an environment variable.
int plt_read_env_var(const TCHAR * env_var_name_ptr,
                     TCHAR * buffer_ptr,
                     int buffer_length)
{
    UNUSED_VARIABLE(buffer_length);

    ASSERT(env_var_name_ptr != NULL);
    ASSERT(buffer_ptr != NULL);

    TCHAR * env_text_ptr = NULL;
    int text_length = 0;

    env_text_ptr = getenv(env_var_name_ptr);

    if (env_text_ptr != NULL)
    {
        // Get the text length.
        text_length = strlen(env_text_ptr);

        // Copy the text to the passed buffer.
        strncpy(buffer_ptr, env_text_ptr, buffer_length);

        // Make sure the string in the passed buffer is null-terminated.
        if (text_length >= buffer_length)
        {
            text_length = buffer_length - 1;
        }

        buffer_ptr[text_length] = '\0';
    }

    return text_length;
}

#endif // __linux__

// Windows(TM) functions.
#ifdef _WIN64

// The _tfopen_s function is already defined on Windows(TM).

// Read the contents of an environment variable.
int plt_read_env_var(const TCHAR * env_var_name_ptr,
                     TCHAR * buffer_ptr,
                     int buffer_length)
{
    ASSERT(env_var_name_ptr != NULL);
    ASSERT(buffer_ptr != NULL);

    int text_length = 0;

    text_length = GetEnvironmentVariable(env_var_name_ptr,
                                         buffer_ptr,
                                         buffer_length);

    return text_length;
}

#endif // _WIN64
