//======================================================================
// File: f64_seq.h
// Author: Bill Hallahan
// Date: Sep 14, 2019
// 
// Abstract:
//
//   This file contains the definition for a 64-bit sequence
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


#ifndef F64_SEQ_H
#define F64_SEQ_H

#ifdef __cplusplus
extern "C" {
#endif

// Status values
enum f64_status_t
{
    F64_SUCCESS,
    F64_FAIL,
    F64_OUT_OF_MEMORY,
    F64_BAD_ARGUMENT
};

//======================================================================
// f64_seq_handle_t handle definition
//======================================================================

typedef struct f64_seq_s * f64_seq_handle_t;

// Functions
int f64_make(f64_seq_handle_t * f64_ptr_ptr,
             const TCHAR * path_file_name_ptr);

void f64_delete(f64_seq_handle_t * f64_ptr_ptr);

void f64_reset(f64_seq_handle_t f64_ptr);

uint64_t f64_next(f64_seq_handle_t f64_ptr);

#ifdef __cplusplus
}
#endif

#endif // #ifdef F64_SEQ_H
