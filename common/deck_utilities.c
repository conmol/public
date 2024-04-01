//======================================================================
// File: deck_utilities.c
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
#include "deck_utilities.h"

// Get the index of the top card if the Nine of Diamonds
// is cut to the bottom of the deck.
int deck_get_top_card_index(deck_info_t * deck_info_ptr)
{
    ASSERT(deck_info_ptr != NULL);

    int card = 0;
    int i = 0;
    int top_card_index = 0;

    // Find the index of the top card.
    for (i = 0; i < deck_info_ptr->deck_length; ++i)
    {
        card = deck_info_ptr->deck_array[i];
        if (card == ((SUIT_DIAMOND << 8) + 9))
        {
            top_card_index = (i + 1) % deck_info_ptr->deck_length;
        }
    } // for (i = 0; i < deck_info_ptr->deck_length; ++i)

    return top_card_index;
}

// Display the full stack.
void deck_display_stack(deck_info_t * deck_info_ptr,
                        int top_card_index)
{
    ASSERT(deck_info_ptr != NULL);

    static const TCHAR * card_value_lookup_array[16] = \
    {
        _T("X"), _T("A"), _T("2"), _T("3"),
        _T("4"), _T("5"), _T("6"), _T("7"),
        _T("8"), _T("9"), _T("10"), _T("J"),
        _T("Q"), _T("K"), _T("X"), _T("X")
    };

    static const TCHAR * card_suit_lookup_array[4] = \
    {
        _T("S"), _T("C"), _T("H"), _T("D")
    };

    const TCHAR * suit_ptr = NULL;
    const TCHAR * value_ptr = NULL;
    int card = 0;
    int i = 0;

    // Write out the deck order.
    for (i = 0; i < deck_info_ptr->deck_length; ++i)
    {
        card = deck_info_ptr->deck_array[(i + top_card_index) % deck_info_ptr->deck_length];
        suit_ptr = card_suit_lookup_array[(card >> 8) & 3];
        value_ptr = card_value_lookup_array[card & 0xF];

        if (i != deck_info_ptr->deck_length - 1)
        {
            _tprintf(_T("%s%s, "), value_ptr, suit_ptr);
        }
        else
        {
            _tprintf(_T("%s%s"), value_ptr, suit_ptr);
        }

        if (((i + 1) & 7) == 0)
        {
            _tprintf(_T("\n"));
        }
    } // for (i = 0; i < deck_info_ptr->deck_length; ++i)

    _tprintf(_T("\n\n"));
}

// Get the deck duplication count score. Cards with the same
// value within 6 cards of each other get a penalty of 1,
// and a penalty of 10 if next to each other. This is to favor
// deck orders where the cards with the same value are spread
// out the most. The returned score is between 0 and 65535.
int deck_get_dup_count_score(deck_info_t * deck_info_ptr)
{
    ASSERT(deck_info_ptr != NULL);

    int last_value_position_array[NUMBER_OF_CARDS_IN_A_SUIT] = \
    {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };
    int card = 0;
    int card_value = 0;
    int last_value_position = 0;
    int value_position_diff = 0;
    int i = 0;
    int dup_count = 0;

    // Loop over all cards in the deck
    for (i = 0; i < deck_info_ptr->deck_length; ++i)
    {
        card = deck_info_ptr->temp_deck_array[i];
        card_value = card & 0xF;

        ASSERT(card_value > 0);
        ASSERT(card_value <= 13);

        last_value_position = last_value_position_array[card_value - 1];
        last_value_position_array[card_value - 1] = i;

        if (last_value_position >= 0)
        {
            value_position_diff = i - last_value_position;

            switch (value_position_diff)
            {
            case 1:
                dup_count += 32;
                break;

            case 2:
                dup_count += 16;
                break;

            case 3:
                dup_count += 8;
                break;

            case 4:
                dup_count += 4;
                break;

            case 5:
                dup_count += 2;
                break;

            case 6:
                dup_count += 1;
                break;

            default:
                break;
            } // switch (value_position_diff)
        } // if (last_value_position != 0)
    } // for (i = 0; i < deck_info_ptr->deck_length; ++i)

    // Loop over the first five cards.
    for (i = 0; i < 5; ++i)
    {
        card = deck_info_ptr->temp_deck_array[i];
        card_value = card & 0xF;

        ASSERT(card_value > 0);
        ASSERT(card_value <= 13);

        last_value_position = last_value_position_array[card_value - 1];
        last_value_position_array[card_value - 1] = i;

        if (last_value_position > 5)
        {
            if (last_value_position > 46)
            {
                value_position_diff = i - (last_value_position - 52);
            }
            else
            {
                value_position_diff = last_value_position - i;
            }
        }

        switch (value_position_diff)
        {
        case 1:
            dup_count += 32;
            break;

        case 2:
            dup_count += 16;
            break;

        case 3:
            dup_count += 8;
            break;

        case 4:
            dup_count += 4;
            break;

        case 5:
            dup_count += 2;
            break;

        case 6:
            dup_count += 1;
            break;

        default:
            break;
        } // switch (value_position_diff)
    } // for (i = 0; i < 5; ++i)

    return 65535 - dup_count;
}

// Generate a sequence with one-bits for cards with the
// specified values.
uint64_t deck_get_sequence_from_deck(int * value_ptr,
                                     int value_length,
                                     deck_info_t * deck_info_ptr)
{
    ASSERT(value_ptr != NULL);

    uint64_t sequence = 0LL;
    int i = 0;
    int card = 0;
    int card_value = 0;

    for (i = 0; i < deck_info_ptr->deck_length; ++i)
    {
        sequence = sequence << 1;
        card = deck_info_ptr->temp_deck_array[i];
        card_value = card & 0xFF;

        if (deck_is_value_in_array(card_value, value_ptr, value_length))
        {
            sequence |= 1;
        }
    }

    return sequence;
}

// Determine if the passed value is in the passed array.
int deck_is_value_in_array(int value, int * array_ptr, int array_length)
{
    ASSERT(array_ptr != NULL);

    int i = 0;
    int is_in_array = FALSE;

    for (i = 0; i < array_length; ++i)
    {
        if (value == array_ptr[i])
        {
            is_in_array = TRUE;
            break;
        }
    }

    return is_in_array;
}
