//======================================================================
// File: sequence_utilities.c
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

#include "platform_os.h"
#include "sequence_utilities.h"

// Display sequence matches
void seq_display_sequence_matches(int sequence_type_bits)
{
    static const TCHAR * sequence_text_ptr_array[26] = \
    {
        _T(" A6"), _T(" A7"), _T(" 27"), _T(" 28"),
        _T(" 38"), _T(" 39"), _T(" 49"), _T(" 4T"),
        _T(" 5T"), _T(" 5J"), _T(" 6J"), _T(" 6Q"),
        _T(" 7Q"), _T(" EV"), _T(" HD"), _T(" CD"),
        _T(" HC"), _T("M34"), _T("M46"), _T("M47"),
        _T("M58"), _T("M59"), _T("M6Q"), _T(" PR"),
        _T(" FI"), _T(" LU")
    };

    static const int sequence_bit_array[26] = \
    {
        SEQ_A6, SEQ_A7, SEQ_27, SEQ_28,
        SEQ_38, SEQ_39, SEQ_49, SEQ_4T,
        SEQ_5T, SEQ_5J, SEQ_6J, SEQ_6Q,
        SEQ_7Q, SEQ_EV, SEQ_HD, SEQ_CD,
        SEQ_HC, SEQ_M34, SEQ_M46, SEQ_M47,
        SEQ_M58, SEQ_M59, SEQ_M6Q, SEQ_PR,
        SEQ_FI, SEQ_LU
    };

    for (int i = 0; i < (int)(sizeof(sequence_bit_array)/sizeof(sequence_bit_array[0])); ++i)
    {
        if ((sequence_type_bits & sequence_bit_array[i]) != 0)
        {
            _tprintf(_T("%s sequence is supported.\n"), sequence_text_ptr_array[i]);
        }
    }

    _tprintf(_T("\n"));
}
