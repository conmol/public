//======================================================================
// File: f64_seq.c
// Author: Bill Hallahan
// Date: Sep 14, 2019
// 
// Abstract:
//
//   This file contains the implementation for a 64-bit sequence
//   generator.
//
// Copyright (c) 2019-2024, William Hallahan.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "platform_os.h"
#include "f64_seq.h"
#include "../common/platform_utilities.h"

#define MAXIMUM_NUMBER_OF_FILES (16)
#define MAXIMUM_FILE_NAME_LENGTH (256)

//======================================================================
// f64_seq_s structure definition
//======================================================================

struct f64_seq_s
{
    uint64_t * sequence_ptr;
    size_t count;
    size_t seq_index;
    int file_name_index;
};

//======================================================================
// Cached sequence data structure definition
//======================================================================

struct f64_cached_seq_s
{
    uint64_t * sequence_ptr;
    size_t count;
} f64_cached_seq_t;

//======================================================================
// Static data declarations
//======================================================================

static uint32_t s_reference_count_array[MAXIMUM_NUMBER_OF_FILES];
static TCHAR s_path_file_name_array[MAXIMUM_NUMBER_OF_FILES][MAXIMUM_FILE_NAME_LENGTH];
static struct f64_cached_seq_s s_cache_array[MAXIMUM_NUMBER_OF_FILES];

//======================================================================
// Forward declaration of static functions.
//======================================================================

static int f64_load_file(f64_seq_handle_t f64_ptr,
                         const TCHAR * path_file_name_ptr);

//======================================================================
//  Function: f64_make
//
//  Abstract:
//
//    This function constructs an instance of xxxxx
//======================================================================

int f64_make(f64_seq_handle_t * f64_ptr_ptr,
             const TCHAR * path_file_name_ptr)
{
    ASSERT(f64_ptr_ptr != NULL);
    ASSERT(path_file_name_ptr != NULL);

    f64_seq_handle_t f64_ptr = NULL;
    TCHAR * cached_file_name_ptr = NULL;
    int i = 0;
    int status = F64_SUCCESS;

    // This do loop construct ends with "while (0)", so it is not a loop.
    // This allows using "break" to exit on errors and avoids deep-nesting
    // of code while still allowing a single "return" statement without
    // using a "goto" statement.
    do
    {
        // Check passed arguments.
        if ((f64_ptr_ptr == NULL)
            || (path_file_name_ptr == NULL))
        {
            status = F64_BAD_ARGUMENT;
            break;
        }

        // Pass back NULL on failure.
        *f64_ptr_ptr = NULL;

        //  Initialize the structure instance.
        f64_ptr = (f64_seq_handle_t)malloc(sizeof(*f64_ptr));

        if (f64_ptr == NULL)
        {
            status = F64_OUT_OF_MEMORY;
            break;
        }

        // Initialize the handle memory to zero.
        memset(f64_ptr, 0, sizeof(*f64_ptr));

        // Pass the handle back to the caller.
        *f64_ptr_ptr = f64_ptr;

        /* Determine if the file is already loaded. */
        f64_ptr->file_name_index = -1;

        for (i = 0; i < MAXIMUM_NUMBER_OF_FILES; ++i)
        {
            cached_file_name_ptr = s_path_file_name_array[i];

            if (cached_file_name_ptr[0] == '\0')
            {
                /* The file was not found. Cache the file name. */
                f64_ptr->file_name_index = i;
                _tcscpy_s(cached_file_name_ptr,
                          MAXIMUM_FILE_NAME_LENGTH,
                          path_file_name_ptr);
                break;
            }
            else
            {
                /* Is the cached file name the passed file name? */
                if (_tcscmp(path_file_name_ptr, cached_file_name_ptr) == 0)
                {
                    /* An object instance already exists with this file name.
                     * Use the same sequence pointer as that instance. */
                    f64_ptr->file_name_index = i;
                    f64_ptr->sequence_ptr = s_cache_array[f64_ptr->file_name_index].sequence_ptr;
                    f64_ptr->count = s_cache_array[f64_ptr->file_name_index].count;
                    break;
                }
            }
        }

        /* Was either the cached file name found, or there was space to cache
         * the current file name? */
        if (f64_ptr->file_name_index == -1)
        {
            status = F64_FAIL;
            break;
        }

        /* Is this the first instance with this file name? */
        if (s_reference_count_array[f64_ptr->file_name_index] == 0)
        {
            /* This is the first instance with this file name. Load the file
             * contents into a sequence buffer. */
            if (f64_load_file(f64_ptr, path_file_name_ptr) != F64_SUCCESS)
            {
                status = F64_FAIL;
                break;
            }
        }
        
        s_reference_count_array[f64_ptr->file_name_index] += 1;
    }
    while (0);

    // If a failure occurred, then free all memory.
    if (status != F64_SUCCESS)
    {
        f64_delete(f64_ptr_ptr);
    }

    return status;
}

//======================================================================
//  Function: f64_delete
//
//  Abstract:
//
//    This function deletes an instance of xxxxx.
//======================================================================

void f64_delete(f64_seq_handle_t * f64_ptr_ptr)
{
    f64_seq_handle_t f64_ptr = NULL;

    if (f64_ptr_ptr != NULL)
    {
        f64_ptr = *f64_ptr_ptr;

        if (f64_ptr != NULL)
        {
            // Free structure element data.
            s_reference_count_array[f64_ptr->file_name_index] -= 1;
        
            if ((s_reference_count_array[f64_ptr->file_name_index] == 0)
                && (s_cache_array[f64_ptr->file_name_index].sequence_ptr != NULL))
            {
                free(s_cache_array[f64_ptr->file_name_index].sequence_ptr);
            }

            // Free this object.
            free (f64_ptr);
            *f64_ptr_ptr = NULL;
        }
    }
}

//======================================================================
//  Static function: f64_load_file
//======================================================================

int f64_load_file(f64_seq_handle_t f64_ptr,
                  const TCHAR * path_file_name_ptr)
{
    ASSERT(f64_ptr != NULL);
    ASSERT(path_file_name_ptr != NULL);

    struct _stat64 file_data;
    FILE * file_ptr = NULL;
    uint64_t * seq_ptr = NULL;
    size_t read_count = 0;
    unsigned int i = 0;
    int estatus = 0;
    int status = F64_SUCCESS;

    // This do loop construct ends with "while (0)", so it is not a loop.
    // This allows using "break" to exit on errors and avoids deep-nesting
    // of code while still allowing a single "return" statement without
    // using a "goto" statement.
    do
    {
        /* Get the size of the file in bytes. */
        if (_tstat64(path_file_name_ptr, &file_data) != 0)
        {
            status = F64_FAIL;
            break;
        }

        /* Convert the size in bytes to 64-bit words. */
        f64_ptr->count = file_data.st_size / sizeof(uint64_t);

        /* Allocate a sequence buffer for the 64-bit sequence values. */
        f64_ptr->sequence_ptr = (uint64_t *)malloc(f64_ptr->count * sizeof(uint64_t));

        /* Was the sequence buffer successfully allocated? */
        if (f64_ptr->sequence_ptr == NULL)
        {
            status = F64_OUT_OF_MEMORY;
            break;
        }

        /* Save the sequence pointer so other object instances that used the
         * same file name can use the same array. */
        s_cache_array[f64_ptr->file_name_index].sequence_ptr = f64_ptr->sequence_ptr;
        s_cache_array[f64_ptr->file_name_index].count = f64_ptr->count;

        /* Open the file. */
        estatus = _tfopen_s(&file_ptr,
                            (CHAR_TYPE *)path_file_name_ptr,
                            (CHAR_TYPE *)_T("rb"));

        if ((estatus != 0) || (file_ptr == NULL))
        {
            status = F64_FAIL;
            break;
        }
        
        /* Read the data into the sequence buffer. */
        seq_ptr = f64_ptr->sequence_ptr;

        for (i = 0; i < f64_ptr->count; ++i)
        {
            read_count = fread(seq_ptr, sizeof(*seq_ptr), 1, file_ptr);

            if (read_count != 1)
            {
                break;
            }

            ++seq_ptr;
        }
    }
    while (0);

    if (file_ptr != NULL)
    {
        fclose(file_ptr);
    }

    return status;
}

//======================================================================
//  Function: f64_reset
//======================================================================

void f64_reset(f64_seq_handle_t f64_ptr)
{
    ASSERT(f64_ptr != NULL);

    f64_ptr->seq_index = 0;
}

//======================================================================
//  Function: f64_next
//======================================================================

uint64_t f64_next(f64_seq_handle_t f64_ptr)
{
    ASSERT(f64_ptr != NULL);

    uint64_t seq_value = 0;

    if (f64_ptr->seq_index < f64_ptr->count)
    {
        seq_value = f64_ptr->sequence_ptr[f64_ptr->seq_index];
        ++f64_ptr->seq_index;
    }

    return seq_value;
}
