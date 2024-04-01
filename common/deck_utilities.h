//======================================================================
// Header File: deck_utilities.h
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

#ifndef DECK_UTILITIES_H
#define DECK_UTILITIES_H

#include "platform_os.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// Structure to store deck information.
typedef struct deck_info_s
{
    int deck_array[64];
    int temp_deck_array[64];
    int deck_length;
    int deck_score;
    int sequence_count;
    int sequence_type_bits;
    BOOL find_all_sequences_flag;
} deck_info_t;

// The suits are defined by the = red-bit << 1; + cd-bit.
static const int SUIT_SPADE   = 0;
static const int SUIT_CLUB    = 1;
static const int SUIT_HEART   = 2;
static const int SUIT_DIAMOND = 3;

static const int N_BITS_MAX = 52;

#define NUMBER_OF_CARDS_IN_A_SUIT (13)

// Get the index of the top card if the Nine of Diamonds
// is cut to the bottom of the deck.
int deck_get_top_card_index(deck_info_t * deck_info_ptr);

// Display the full stack.
void deck_display_stack(deck_info_t * deck_info_ptr,
                        int top_card_index);

// Get the deck duplication count score. Cards with the same
// value within 6 cards of each other get a penalty of 1,
// and a penalty of 10 if next to each other. This is to favor
// deck orders where the cards with the same value are spread
// out the most. The returned score is between 0 and 65535.
int deck_get_dup_count_score(deck_info_t * deck_info_ptr);

// Generate a sequence with one-bits for cards with the
// specified values.
uint64_t deck_get_sequence_from_deck(int * value_ptr,
                                     int value_length,
                                     deck_info_t * deck_info_ptr);

// Determine if the passed value is in the passed array.
int deck_is_value_in_array(int value, int * array_ptr, int array_len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DECK_UTILITIES_H
