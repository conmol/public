//======================================================================
// File: dbn_de_bruijn.h
// Author: Bill Hallahan
// Date: Aug 05, 2018
// 
// Abstract:
//
//   This file contains the definitions for functions to create
//   de Bruijn sequences.
//
// Copyright (c) 2018-2024, William Hallahan.
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

#ifndef DBN_DE_BRUIJN_H
#define DBN_DE_BRUIJN_H

#ifdef __cplusplus
extern "C" {
#endif

// Status values
enum dbn_status_t
{
    DBN_SUCCESS,
    DBN_OUT_OF_MEMORY,
    DBN_BAD_ARGUMENT
};

// Value for the 'm_subseq_not_all_the_same_bits' argument
// of the dbn_make function.
enum seq_bit_uniqueness_t
{
    DBN_ALLOW_ALL_BITS_THE_SAME,
    DBN_REQUIRE_BITS_NOT_ALL_THE_SAME
};

//======================================================================
// dbn_de_bruijn_handle_t handle definition
//======================================================================

typedef struct dbn_de_bruijn_s * dbn_de_bruijn_handle_t;

// Functions
int dbn_make(dbn_de_bruijn_handle_t * dbn_ptr_ptr,
             unsigned int n_bits_max,
             unsigned int n_one_bits,
             int subseq_not_all_the_same_bits);

void dbn_delete(dbn_de_bruijn_handle_t * dbn_ptr_ptr);

uint64_t dbn_next(dbn_de_bruijn_handle_t dbn_ptr);

void dbn_reset(dbn_de_bruijn_handle_t dbn_ptr);

#ifdef __cplusplus
}
#endif

#endif // #ifdef DBN_DE_BRUIJN_H
