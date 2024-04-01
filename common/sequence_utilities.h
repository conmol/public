//======================================================================
// Header File: sequence_utilities.h
// Author: Bill Hallahan
// Date: June 14, 2019
//
// Copyright (C) 2013-2024 William Hallahan
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

#ifndef SEQUENCE_UTILITIES_H
#define SEQUENCE_UTILITIES_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// Definitions for sequence type bits.
#define SEQ_A6         (0x1)
#define SEQ_A7         (0x2)
#define SEQ_27         (0x4)
#define SEQ_28         (0x8)
#define SEQ_38        (0x10)
#define SEQ_39        (0x20)
#define SEQ_49        (0x40)
#define SEQ_4T        (0x80)
#define SEQ_5T       (0x100)
#define SEQ_5J       (0x200)
#define SEQ_6J       (0x400)
#define SEQ_6Q       (0x800)
#define SEQ_7Q      (0x1000)
#define SEQ_EV      (0x2000)
#define SEQ_HD      (0x4000)
#define SEQ_CD      (0x8000)
#define SEQ_HC     (0x10000)
#define SEQ_M34    (0x20000)
#define SEQ_M46    (0x40000)
#define SEQ_M47    (0x80000)
#define SEQ_M58   (0x100000)
#define SEQ_M59   (0x200000)
#define SEQ_M6Q   (0x400000)
#define SEQ_PR    (0x800000)
#define SEQ_FI   (0x1000000)
#define SEQ_LU   (0x2000000)

// Display sequence matches
void seq_display_sequence_matches(int sequence_type_bits);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SEQUENCE_UTILITIES_H

