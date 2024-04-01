//======================================================================
// Header File: bit_utilities.h
// Author: Bill Hallahan
// Date: June 14, 2019
//
// Copyright (C) 2019-2024 William Hallahan
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

#ifndef BIT_UTILITIES_H
#define BIT_UTILITIES_H

#include "platform_os.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

static const uint64_t SUBSEQUENCE_MASK = 63LL;

// Determine if the passed 52-bit value has unique subsequences.
// This routine will not work with full-64-bit values.
BOOL bit_has_unique_subsequences(uint64_t sequence);

// Generate an integer sequence from a binary text sequence.
uint64_t bit_get_integer_seq(const TCHAR * sequence_ptr);

// Display an integer sequence.
void bit_display_integer_sequence(TCHAR * sequence_name_ptr,
                                  uint64_t sequence,
                                  int first_bit_index,
                                  int n_bits_max);

// Count the number of bits set in a 32-bit unsigned integer.
int bit_count_set_bits(unsigned int value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BIT_UTILITIES_H
