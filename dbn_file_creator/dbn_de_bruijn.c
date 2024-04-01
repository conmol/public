//======================================================================
// File: dbn_de_bruijn.c
// Author: Bill Hallahan
// Date: Aug 05, 2018
// 
// Abstract:
//
//   This file contains the implementation for functions to create
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

#include "../common/platform_os.h"
#include "../common/bit_utilities.h"
#include "dbn_de_bruijn.h"

// The size of the program stack.
#define BIT_OBJECT_STACK_SIZE (8192)

//======================================================================
// dbn_move_t structure definition.
//======================================================================

typedef struct seq_move_s
{
    uint64_t subseq_bit_store;
    uint64_t value;
    unsigned int length;
    unsigned int set_bit_count;
    unsigned int bit;
} dbn_move_t;

//======================================================================
// dbn_de_bruijn_s structure definition
//======================================================================

struct dbn_de_bruijn_s
{
    dbn_move_t * bit_obj_stack_ptr;
    uint64_t all_subseq_bits_are_zeros_bit;
    uint64_t all_subseq_bits_are_ones_bit;
    uint64_t subseq_mask;
    unsigned int n_sub_sequence_bit_count;
    unsigned int n_bits_max;
    unsigned int n_one_bits;
    int subseq_not_all_the_same_bits;
    int bit_obj_stack_index;
};

//======================================================================
// Forward declaration of static functions.
//======================================================================

static unsigned int dbn_get_next_power_of_two(unsigned int value);

static unsigned int dbn_get_log_base_2_rounded_up(unsigned int value);

#ifdef _DEBUG
static void display_sequence(TCHAR * text_ptr,
                             uint64_t sequence,
                             unsigned int length);
#endif // _DEBUG

//======================================================================
//  Function: dbn_make
//
//  Abstract:
//
//       Generate the next De Bruijn sequence of length 'n_bits_max' that
//       has exactly 'n_one_bits' binary digits that are the value 1.
//       If 'n_one_bits' is equal to zero, then allow any possible number
//       of binary digits to be the value 1.
//
//  Input:
//
//    dbn_ptr_ptr    A pointer to a pointer to an instance of type
//                   dbn_de_bruijn_s
//
//    n_bits_max     The sequence length.
//
//    n_one_bits     The number of one bits in the sequence. If this is too low
//                   or too high, no sequences will be found. If this is set to
//                   zero, then all possible De Bruijn sequences will be found,
//                   and the number of ones will vary in the sequences.
//
//    subseq_not_all_the_same_bits  If this is set to DBN_ALLOW_ALL_BITS_THE_SAME
//                                  then a subsequence may be all zeros or all ones.
//                                  If this is set to DBN_REQUIRE_BITS_NOT_ALL_THE_SAME,
//                                  then there must be at least one bit that differs
//                                  from others in a subsequence.
//
//======================================================================

int dbn_make(dbn_de_bruijn_handle_t * dbn_ptr_ptr,
             unsigned int n_bits_max,
             unsigned int n_one_bits,
             int subseq_not_all_the_same_bits)
{
    ASSERT(dbn_ptr_ptr != NULL);

    dbn_de_bruijn_handle_t dbn_ptr = NULL;
    int status = DBN_SUCCESS;

    // This do loop construct ends with "while (0)", so it is not a loop.
    // This allows using "break" to exit on errors and avoids deep-nesting
    // of code while still allowing a single "return" statement without
    // using a "goto" statement.
    do
    {
        // Check passed arguments.
        if ((dbn_ptr_ptr == NULL) || (n_bits_max > 64))
        {
            status = DBN_BAD_ARGUMENT;
            break;
        }

        // Pass back NULL on failure.
        *dbn_ptr_ptr = NULL;

        //  Initialize the structure instance.
        dbn_ptr = (dbn_de_bruijn_handle_t)malloc(sizeof(*dbn_ptr));

        if (dbn_ptr == NULL)
        {
            status = DBN_OUT_OF_MEMORY;
            break;
        }

        // Initialize the handle memory to zero.
        memset(dbn_ptr, 0, sizeof(*dbn_ptr));

        // Pass the handle back to the caller.
        *dbn_ptr_ptr = dbn_ptr;

        // Allocate stack memory.
        dbn_ptr->bit_obj_stack_ptr = (dbn_move_t *)malloc(BIT_OBJECT_STACK_SIZE * sizeof(*dbn_ptr->bit_obj_stack_ptr));
    
        if (dbn_ptr->bit_obj_stack_ptr == NULL)
        {
            // Failed to allocate the stack pointer.
            return DBN_OUT_OF_MEMORY;
        }

        dbn_ptr->n_bits_max = n_bits_max;
        dbn_ptr->n_one_bits = n_one_bits;
    
        // Calculate the number of bits in a sub-sequence code.
        dbn_ptr->n_sub_sequence_bit_count = dbn_get_log_base_2_rounded_up(dbn_ptr->n_bits_max);
        dbn_ptr->subseq_mask = (1LL << dbn_ptr->n_sub_sequence_bit_count) - 1LL;
    
        dbn_ptr->subseq_not_all_the_same_bits = subseq_not_all_the_same_bits;
        dbn_ptr->all_subseq_bits_are_zeros_bit = 1LL;
        dbn_ptr->all_subseq_bits_are_ones_bit = 1LL << dbn_ptr->subseq_mask;

        dbn_reset(dbn_ptr);
    }
    while (0);

    // If a failure occurred, then free all memory.
    if (status != DBN_SUCCESS)
    {
        dbn_delete(dbn_ptr_ptr);
    }

    return status;
}

//======================================================================
//  Function: dbn_delete
//
//  Abstract:
//
//    This function deletes an instance.
//
//  Input:
//
//    dbn_ptr_ptr    A pointer to a pointer to an instance of
//                   type dbn_de_bruijn_s
//
//======================================================================

void dbn_delete(dbn_de_bruijn_handle_t * dbn_ptr_ptr)
{
    dbn_de_bruijn_handle_t dbn_ptr = NULL;

    if (dbn_ptr_ptr != NULL)
    {
        dbn_ptr = *dbn_ptr_ptr;

        if (dbn_ptr != NULL)
        {
            if (dbn_ptr->bit_obj_stack_ptr != NULL)
            {
                // Free stack memory.
                free(dbn_ptr->bit_obj_stack_ptr);
            }

            // Free this object.
            free(dbn_ptr);
            *dbn_ptr_ptr = NULL;
        }
    }
}

//======================================================================
//  Function: dbn_next
//
//  Abstract:
//
//    This function returns the next De Bruijn sequence. If there
//    are no more De Bruijn sequences then the value zero is returned.
//
//  Input:
//
//    dbn_ptr    A pointer to an instance of type dbn_de_bruijn_s
//
//  Return value:
//
//    This function returns a value of type uint64_t that is the
//    next De Bruijn sequence. If there are no more De Bruijn sequences
//    then the value zero is returned.
//
//======================================================================

uint64_t dbn_next(dbn_de_bruijn_handle_t dbn_ptr)
{
    ASSERT(dbn_ptr != NULL);

    dbn_move_t * bit_obj_ptr = NULL;
    dbn_move_t * new_bit_obj_ptr = NULL;
    uint64_t subseq_bit = 0;
    uint64_t subseq_bit_store = 0;
    uint64_t value = 0;
    uint64_t temp = 0;
    uint64_t result_sequence = 0;
    unsigned int length = 0;
    unsigned int set_bit_count = 0;
    unsigned int subseq_index = 0;
    BOOL is_valid_sequence_flag = FALSE;
    int i = 0;

    // Start of reentrant depth-first search loop.
    while (dbn_ptr->bit_obj_stack_index > 0)
    {
        // Pop the first item off of the move stack.
        bit_obj_ptr = &dbn_ptr->bit_obj_stack_ptr[--dbn_ptr->bit_obj_stack_index];

        // Adjuset the count of the number of set bits for the new bit.
        set_bit_count = bit_obj_ptr->set_bit_count + bit_obj_ptr->bit;

        // If too many bits are set, then this sequence is not valid.
        // If m_n_one_bits is zero, then allow any number of set bits.
        // If the sequence is not valid, then read the next digit off the stack.
        if ((dbn_ptr->n_one_bits != 0) && (set_bit_count > dbn_ptr->n_one_bits))
        {
            continue;
        }

        // Update sequence value and sequence length.
        value = (bit_obj_ptr->value << 1) + bit_obj_ptr->bit;
        length = bit_obj_ptr->length + 1;

        // Get the bit-vector for the current sequences.
        subseq_bit_store = bit_obj_ptr->subseq_bit_store;

        // If there are enough bits to test a subsequence, then
        // make sure the subsequence is unique.
        is_valid_sequence_flag = TRUE;

        if (length >= dbn_ptr->n_sub_sequence_bit_count)
        {
            // Make sure that the new sub-sequence is unique. The
            // is_valid_sequence_flag flag will become TRUE is the new sub-sequence
            // is unqiue.
            subseq_index = (unsigned int)(value & dbn_ptr->subseq_mask);
            subseq_bit = 1LL << subseq_index;
            is_valid_sequence_flag = (subseq_bit_store & subseq_bit) == 0;

            // Do not allow all subseq bits to be the same.
            if (dbn_ptr->subseq_not_all_the_same_bits && is_valid_sequence_flag)
            {
                is_valid_sequence_flag = (subseq_bit != dbn_ptr->all_subseq_bits_are_zeros_bit)
                    && (subseq_bit != dbn_ptr->all_subseq_bits_are_ones_bit);
            }

            // Update the subsequence bit store for the next sequence value.
            subseq_bit_store |= subseq_bit;
        }

        // If the sequence is not valid, then read the next digit off the stack.
        if (!is_valid_sequence_flag)
        {
            continue;
        }

        // Is the sequence complete?
        if ((length == dbn_ptr->n_bits_max) && ((dbn_ptr->n_one_bits == 0) || (set_bit_count == dbn_ptr->n_one_bits)))
        {
            // The sequence is long enough. There are
            // 'm_n_sub_sequence_bit_count - 1' sub-sequences formed using
            // the most significant 'm_n_sub_sequence_bit_count - 1' bits
            // in 'value' wrapped with the least significant bit in
            // 'value'. These sub-sequences have not been verified to be
            // unique.  Test them now.
            temp = (value << (dbn_ptr->n_sub_sequence_bit_count - 1)) | (value >> (dbn_ptr->n_bits_max - (dbn_ptr->n_sub_sequence_bit_count - 1)));

            for (i = 0; is_valid_sequence_flag && (i < (int)(dbn_ptr->n_sub_sequence_bit_count - 1)); ++ i)
            {
                subseq_index = (unsigned int)(temp & dbn_ptr->subseq_mask);
                subseq_bit = 1LL << subseq_index;
                is_valid_sequence_flag = (subseq_bit_store & subseq_bit) == 0;

                // Do not allow all subseq bits to be the same.
                if (dbn_ptr->subseq_not_all_the_same_bits && is_valid_sequence_flag)
                {
                    is_valid_sequence_flag = (subseq_bit != dbn_ptr->all_subseq_bits_are_zeros_bit)
                        && (subseq_bit != dbn_ptr->all_subseq_bits_are_ones_bit);
                }

                // Update the subsequence bit store for the next sequence value.
                subseq_bit_store |= subseq_bit;
                temp = temp >> 1;
            }

            // Are all sub-sequences unique?
            if (is_valid_sequence_flag)
            {
                // All subsequences are unique. Return the valid sequence value.
                result_sequence = value;
                break;
            }
        }
        else // length < dbn_ptr->n_bits_max
        {
            // The sequence has not reached the required length.
            // Will the the stack size be exceeded?
            // This check is not necessary since the bit-length is restricted
            // to 64-bits.
            //if (dbn_ptr->bit_obj_stack_index >= BIT_OBJECT_STACK_SIZE - 3)
            //{
            //    // The stack size will be exceeded if the function continues.
            //    return DBN_PROGRAM_STACK_OVERFLOW;
            //}

            // Put a bit on the stack.
            int bit = 0;
            if (length < dbn_ptr->n_sub_sequence_bit_count)
            {
                bit = 1;
            }

            new_bit_obj_ptr = &dbn_ptr->bit_obj_stack_ptr[dbn_ptr->bit_obj_stack_index++];

            new_bit_obj_ptr->subseq_bit_store = subseq_bit_store;
            new_bit_obj_ptr->value = value;
            new_bit_obj_ptr->length = length;
            new_bit_obj_ptr->set_bit_count = set_bit_count;
            new_bit_obj_ptr->bit = bit;

            // Put the opposite bit on the stack.
            new_bit_obj_ptr = &dbn_ptr->bit_obj_stack_ptr[dbn_ptr->bit_obj_stack_index++];

            new_bit_obj_ptr->subseq_bit_store = subseq_bit_store;
            new_bit_obj_ptr->value = value;
            new_bit_obj_ptr->length = length;
            new_bit_obj_ptr->set_bit_count = set_bit_count;
            new_bit_obj_ptr->bit = 1 - bit;
        }   // if (seq_length < m_n_bits_max)
    }   // while (m_bit_obj_stack_index > 0)

#ifdef _DEBUG
    display_sequence(_T("Sequence = "), result_sequence, dbn_ptr->n_bits_max);
#endif


    return result_sequence;
}

//======================================================================
//  Function: dbn_reset
//
//  Abstract:
//
//    This function resets the De Bruijn generators.  This is called
//    inside of the dbn_make function. Usually this function will not
//    be needed by clients because dbn_next can be called repeatedly,
//    and when there are no more sequences, this function is
//    automatically called inside of the dbn_next function.
//
//  Input:
//
//    dbn_ptr    A pointer to an instance of type dbn_de_bruijn_s
//
//  Return value:
//
//    This function has no return value.
//
//======================================================================

void dbn_reset(dbn_de_bruijn_handle_t dbn_ptr)
{
    ASSERT(dbn_ptr != NULL);

    dbn_move_t * bit_obj_ptr = NULL;

    // Inititialize the stack index.
    dbn_ptr->bit_obj_stack_index = 0;

    // Put a one bit on the stack.
    bit_obj_ptr = &dbn_ptr->bit_obj_stack_ptr[dbn_ptr->bit_obj_stack_index++];

    bit_obj_ptr->subseq_bit_store = 0;
    bit_obj_ptr->value = 0;
    bit_obj_ptr->length = 0;
    bit_obj_ptr->set_bit_count = 0;
    bit_obj_ptr->bit = 1;

    // Put a zero bit on the stack.
    bit_obj_ptr = &dbn_ptr->bit_obj_stack_ptr[dbn_ptr->bit_obj_stack_index++];

    bit_obj_ptr->subseq_bit_store = 0;
    bit_obj_ptr->value = 0;
    bit_obj_ptr->length = 0;
    bit_obj_ptr->set_bit_count = 0;
    bit_obj_ptr->bit = 0;
}


//======================================================================
//  Static function: dbn_get_next_power_of_two
//
//  Abstract:
//
//       This function returns the first power of two that is greater
//       than or equal to the passed value. This fast algorithm only
//       works for values from 1 to 2^31 - 1.
//
//  Input:
//
//    value    A value of type unsigned int.
//
//  Return value:
//
//    This function returns a value of type unsigned int that
//    is the next power of two greater than or equal to the passed
//    value.
//
//======================================================================

unsigned int dbn_get_next_power_of_two(unsigned int value)
{
    // Get the first power of 2 greater than or equal to value.
    // This fast algorithm only works for values from 1 to 2^31 - 1.

    value -= 1;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value += 1;

    return value;
}

//======================================================================
//  Static function: dbn_get_log_base_2_rounded_up
//
//  Abstract:
//
//       This function returns the log base 2 of a power of two
//       that is greater than or equal to the passed value.
//
//  Input:
//
//    value    A value of type unsigned int
//
//  Return value:
//
//    This function returns a value of type unsigned int that xxxxxx
//
//======================================================================

unsigned int dbn_get_log_base_2_rounded_up(unsigned int value)
{
    unsigned int bit_count = 0;
    unsigned int power_of_two = 0;

    // Get the log_base 2 of value rounded up.
    power_of_two = dbn_get_next_power_of_two(value);
    bit_count = bit_count_set_bits(power_of_two - 1);

    return bit_count;
}

#ifdef _DEBUG
// Display a De Bruijn sequence.
void display_sequence(TCHAR * text_ptr,
                      uint64_t sequence,
                      unsigned int length)
{
    ASSERT(text_ptr != NULL);

    TCHAR text_array[256];
    uint64_t high_bit_mask = 0;
    unsigned int i = 0;
    TCHAR c = _T('\0');

    high_bit_mask = 1LL << (length - 1);

    for (i = 0; i < length; ++i)
    {
        if ((sequence & high_bit_mask) == 0)
        {
            c = _T('0');
        }
        else
        {
            c = _T('1');
        }

        text_array[i] = c;

        sequence = sequence << 1;
    }

    text_array[length] = _T('\0');

    _tprintf(_T("%s%s\n"), text_ptr, &text_array[0]);
}

#endif // _DEBUG
