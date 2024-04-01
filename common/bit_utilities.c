//======================================================================
// File: bit_utilities.c
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

#include "bit_utilities.h"

// This more general subsequence routine that handles up to a full 64-bit
// sequence with a 6-bit subsequence mask, is slower than the special-case
// routine below that handles only 52-bit sequences. 

//// Determine if the passed 64-bit value has unique subsequences.
//int bit_has_unique_subsequences(uint64_t sequence,
//                                unsigned int n_bits_max)
//{
//    static const int SUBSEQUENCE_MASK = 63LL;
//    uint64_t subsequence_bit_store = 0LL;
//    uint64_t temp = 0LL;
//    uint64_t subseq_bit = 0LL;
//    unsigned int subsequence_index = 0;
//    int i = 0;
//    int n_subseq_bit_count_minus_one = 5;  // For now, hardcode this
//    BOOL is_valid_flag = TRUE;
//
//    temp = sequence;
//
//    for (i = 0; is_valid_flag && (i < (int)n_bits_max - n_subseq_bit_count_minus_one); ++i)
//    {
//        subsequence_index = (unsigned int)(temp & SUBSEQUENCE_MASK);
//        subseq_bit = 1LL << subsequence_index;
//        is_valid_flag = (subsequence_bit_store & subseq_bit) == 0;
//        subsequence_bit_store |= subseq_bit;
//        temp = temp >> 1;
//    }
//
//    if (is_valid_flag)
//    {
//        temp = (sequence << n_subseq_bit_count_minus_one) | (sequence >> (n_bits_max - n_subseq_bit_count_minus_one));
//
//        for (i = 0; is_valid_flag && (i < (int)(n_subseq_bit_count_minus_one)); ++i)
//        {
//            subsequence_index = (unsigned int)(temp & SUBSEQUENCE_MASK);
//            subseq_bit = 1LL << subsequence_index;
//            is_valid_flag = (subsequence_bit_store & subseq_bit) == 0;
//            subsequence_bit_store |= subseq_bit;
//            temp = temp >> 1;
//        }
//    }
//
//    return is_valid_flag;
//}

// Determine if the passed 52-bit value has unique subsequences.
// This routine will not work with full-64-bit values.
BOOL bit_has_unique_subsequences(uint64_t sequence)
{
    static const int SEQUENCE_LENGTH = 52;
    static const int SUBSEQUENCE_MASK = 63LL;
    uint64_t seq = sequence | ((sequence & 31) << SEQUENCE_LENGTH);
    uint64_t subsequence_bit_store = 0LL;
    uint64_t subseq_bit = 0LL;
    unsigned int subsequence_index = 0;
    int i = 0;
    int is_valid = TRUE;

    for (i = 0; is_valid && (i < SEQUENCE_LENGTH); ++i)
    {
        subsequence_index = (unsigned int)(seq & SUBSEQUENCE_MASK);
        subseq_bit = 1LL << subsequence_index;
        is_valid = (subsequence_bit_store & subseq_bit) == 0;
        subsequence_bit_store |= subseq_bit;
        seq >>= 1;
    }

    return is_valid;
}

// Generate an integer sequence from a binary text sequence.
uint64_t bit_get_integer_seq(const TCHAR * sequence_ptr)
{
    ASSERT(sequence_ptr != NULL);

    uint64_t seq = 0ll;
    TCHAR digit = _T('\0');

    while (1)
    {
        digit = *sequence_ptr;
        ++sequence_ptr;

        if ((digit == _T(' ')) || (digit == _T('\t')))
        {
            continue;
        }

        if ((digit == _T('\0')) || (digit == _T('\n')))
        {
            break;
        }

        seq = seq << 1;

        if (digit == _T('1'))
        {
            seq |= 1;
        }
    }

    return seq;
}

// Display an integer sequence.
void bit_display_integer_sequence(TCHAR * sequence_name_ptr,
                                  uint64_t sequence,
                                  int first_bit_index,
                                  int n_bits_max)
{
    int i = 0;
    uint64_t bit_mask = 1LL;
    int start_length = 0;
    int end_length = 0;

    _tprintf(_T("%s sequence:  "), sequence_name_ptr);

    bit_mask = 1LL << (n_bits_max - (first_bit_index + 1));
    start_length = n_bits_max - first_bit_index;

    for (i = 0; i < start_length; ++i)
    {
        if ((bit_mask & sequence) == 0)
        {
            _tprintf(_T("0"));
        }
        else
        {
            _tprintf(_T("1"));
        }

        bit_mask = bit_mask >> 1;
    }

    bit_mask = 1LL << 51;
    end_length = n_bits_max - start_length;

    for (i = 0; i < end_length; ++i)
    {
        if ((bit_mask & sequence) == 0)
        {
            _tprintf(_T("0"));
        }
        else
        {
            _tprintf(_T("1"));
        }

        bit_mask = bit_mask >> 1;
    }

    _tprintf(_T("\n"));
}

// Count the number of bits set in a 32-bit unsigned integer.
int bit_count_set_bits(unsigned int value)
{
    unsigned int count = 0;

    while (value != 0LL)
    {
        value = value & (value - 1LL);
        count += 1;
    }

    return count;
}
