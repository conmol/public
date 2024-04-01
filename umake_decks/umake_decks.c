//======================================================================
// File: umake_decks.c
// Author: Bill Hallahan
// Date: Sep 8, 2018
//
// Abstract:
//
//   This file contains the implementation to create card stacks
//   using the HD, CD, HC, ODD, 7K, 8K, and 4T sequences
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __linux__
#include <unistd.h>
#endif

#include "../common/platform_os.h"
#include "../common/arg_get_arguments.h"
#include "../common/sequence_utilities.h"
#include "../common/deck_utilities.h"
#include "../common/bit_utilities.h"
#include "../common/platform_utilities.h"

#ifdef _WIN64
#pragma warning (disable:4996) // disable warning about string function safety.
#endif

#define REQUIRED_POSITIONAL_ARG_COUNT (1)
#define MAX_CHARACTER_READ_COUNT (256)

// Macro to check for bracelet sequence.
#define CHECK_FOR_BRACELET_SEQUENCE(ARRAY_NAME, SEQ_BIT) \
    sequence = deck_get_sequence_from_deck(&ARRAY_NAME[0], \
                                           sizeof(ARRAY_NAME) / sizeof(ARRAY_NAME[0]), \
                                           deck_info_ptr); \
    is_valid_flag = bit_has_unique_subsequences(sequence); \
    if (is_valid_flag) \
    { \
        sequence_type_bits |= SEQ_BIT; \
        ++sequence_count; \
    }

// These values must fit in 8 bits and define the ambiguous cards in the stack.
const int VALUE_ACE_OR_THREE = 103;
const int VALUE_FOUR_OR_SIX   = 46;
const int VALUE_EIGHT_OR_TEN  = 81;
const int VALUE_JACK_OR_KING = 113;

const int TINY_DECK_SCORE = 0;

/* Structure type used for program arguments. */
typedef struct
{
    TCHAR * input_file_name_ptr;
    unsigned int sequence_group_skip_count;
    BOOL find_all_sequences_flag;
    BOOL cut_nine_of_diamonds_to_bottom;
    BOOL display_8_sequences_min_flag;
    BOOL display_input_sequences_flag;
} ProgramArgument_T;

// Structure to store the 7 sequences and processing data. 
typedef struct seq_info_s
{
    uint64_t red_seq;
    uint64_t cd_seq;
    uint64_t hc_seq;
    uint64_t odd_seq;
    uint64_t c7k_seq;
    uint64_t c8k_seq;
    uint64_t c4t_seq;
    uint64_t subsequence_mask;
    int n_bits_max;
} seq_info_t;

/* Forward declarations. */
int umake_decks_main(ProgramArgument_T * program_args_ptr);

static void display_usage();

// Create the initial deck possibilities.
static void get_deck_possibilities(int * deck_array_ptr,
                                   seq_info_t * seq_info_ptr);

// Determine if any special sequences produce a valid Bracelet code.
static int test_deck_for_special_sequences(deck_info_t * deck_info_ptr);

// Get the deck score.
static int get_deck_score(deck_info_t * deck_info_ptr);

// Create decks from sequences.
static void find_best_deck_order(deck_info_t * deck_info_ptr,
                                 seq_info_t * seq_info_ptr);

/* Start of main program. */
int _tmain(int argc, TCHAR* argv[])
{
    TCHAR * stop_ptr = NULL;
    int arg_index = 0;
    int positional_arg_index = 0;
    int option_int = 0;
    int status = 0;

    ProgramArgument_T program_args;

    static const arg_option_t argument_option_array[] =
    {
        { _T('s'), _T("skip"), OPTION_REQUIRES_PARAMETER, },
        { _T('a'), _T("all"), OPTION_NO_PARAMETER, },
        { _T('c'), _T("cut"), OPTION_NO_PARAMETER, },
        { _T('e'), _T("eight"), OPTION_NO_PARAMETER, },
        { _T('i'), _T("input"), OPTION_NO_PARAMETER, },
        { _T('h'), _T("help"), OPTION_NO_PARAMETER },
        { 0, NULL, OPTION_NO_PARAMETER }
    };

    /* If there are no arguments supplied then display the command to get help. */
    if (argc < REQUIRED_POSITIONAL_ARG_COUNT + 1)
    {
        _tprintf(_T("Program umake_decks [version 1.1]\n"));
        _tprintf(_T("Copyright (C) 2018-2024 William Hallahan\n\n"));
        _tprintf(_T("For help, enter:\n\n"));
        _tprintf(_T("    umake_decks -h\n\n"));
        _exit(1);
    }

    /* Initialize the program argument structure. */
    program_args.input_file_name_ptr = _T("");
    program_args.sequence_group_skip_count = 0;
    program_args.find_all_sequences_flag = FALSE;
    program_args.cut_nine_of_diamonds_to_bottom = FALSE;
    program_args.display_8_sequences_min_flag = FALSE;
    program_args.display_input_sequences_flag = FALSE;

    /* Call the get_arguments function to populate the argument variables. */
    while ((status = arg_get_arguments(argc,
                                       argv,
                                       &argument_option_array[0],
                                       &arg_index,
                                       &option_int)) > ARG_NO_MORE_ARGUMENTS) 
    {
        if (status != ARG_POSITIONAL_ARGUMENT)
        {
            /* Process an optional argument. */
            switch (option_int)
            {
            case _T('s'):
            {
                program_args.sequence_group_skip_count = _tcstol(argv[arg_index],
                                                                    &stop_ptr,
                                                                    10);
                if (_tcslen(stop_ptr) > 0)
                {
                    _tprintf(_T("Error in argument %s.\n"), argv[arg_index]);
                    status = -1;
                }
            }
            break;

            case _T('a'):
            {
                program_args.find_all_sequences_flag = TRUE;
            }
            break;

            case _T('c'):
            {
                program_args.cut_nine_of_diamonds_to_bottom = TRUE;
            }
            break;

            case _T('e'):
            {
                program_args.display_8_sequences_min_flag = TRUE;
            }
            break;

            case _T('i'):
            {
                program_args.display_input_sequences_flag = TRUE;
            }
            break;

            case _T('h'):
            {
                display_usage();
                status = ARG_EXIT_PROGRAM;
            }
            break;

            default:
            {
                _tprintf(_T("Error: Invalid program input: -%s\n"), *argv);
                status = ARG_EXIT_PROGRAM;
            }
            break;
            } // switch (option_int)
        }
        else
        {
            /* Process a positional argument. */
            switch (positional_arg_index)
            {
            case 0:
            {
                program_args.input_file_name_ptr = argv[arg_index];
            }
            break;

            default:
            {
                _tprintf(_T("Too many positional arguments starting at %s.\n"), argv[arg_index]);
                status = ARG_EXIT_PROGRAM;
                break;
            }
            } // switch (positional_arg_index)

            /* Increment the positional argument index. */
            ++positional_arg_index;
        } // if (status != ARG_POSITIONAL_ARGUMENT)

        // If the help was displayed or there was an error, then exit.
        if (status == ARG_EXIT_PROGRAM)
        {
            break;
        }
    } // while ((status = get_arguments(argc, ...

    // If no error occured, arg_report_status has no output.
    arg_report_status(status, argv[arg_index]);

    /* If no parser errors occured then call the umake_decks_main function. */
    if (status == ARG_NO_MORE_ARGUMENTS)
    {
        /* Were the required number of positional arguments supplied? */
        if (positional_arg_index == REQUIRED_POSITIONAL_ARG_COUNT)
        {
            /* Call the primary processing function */
            status = umake_decks_main(&program_args);
        }
        else
        {
            _tprintf(_T("Error: too few arguments. For help\n    %s -h\n"), argv[0]);
            status = -1;
        }
    } // if (status == ARG_NO_MORE_ARGUMENTS)

    return status;
}

/* Function: umake_decks_main */
int umake_decks_main(ProgramArgument_T * program_args_ptr)
{
    ASSERT(program_args_ptr != NULL);

    /* Display the input parameters. */
    //_tprintf(_T("TCHAR * input_file_name_ptr = %s\n"), program_args_ptr->input_file_name_ptr);
    //_tprintf(_T("BOOL find_sequence = %d\n"), program_args_ptr->find_sequence);

    // Main program function
    seq_info_t seq_info;
    deck_info_t deck_info;
    CHAR_TYPE seq_array[MAX_CHARACTER_READ_COUNT];
    TCHAR red_sequence_array[64];
    TCHAR cd_sequence_array[64];
    TCHAR hc_sequence_array[64];
    TCHAR odd_sequence_array[64];
    TCHAR c7k_sequence_array[64];
    TCHAR c8k_sequence_array[64];
    TCHAR c4t_sequence_array[64];
    TCHAR * input_file_name_ptr = NULL;
    TCHAR * seq_ptr = NULL;
    TCHAR * name_ptr = NULL;
    TCHAR * temp_ptr = NULL;
    FILE * input_file_ptr = NULL;
    int display_8_sequences_min_flag = FALSE;
    int display_input_sequences_flag = FALSE;
    int cut_nine_of_diamonds_to_bottom = FALSE;
    int top_card_index = 0;
    int major_sequence_count = 0;
    unsigned int sequence_group_count = 0;
    unsigned int sequence_group_skip_count = 0;
    int status = 0;

    memset(&seq_info, 0, sizeof(seq_info));

    seq_info.subsequence_mask = SUBSEQUENCE_MASK;
    seq_info.n_bits_max = N_BITS_MAX;

    memset(&deck_info, 0, sizeof(deck_info));

    deck_info.deck_length = seq_info.n_bits_max;
    deck_info.sequence_type_bits = 0;

    input_file_name_ptr = program_args_ptr->input_file_name_ptr;
    sequence_group_skip_count = program_args_ptr->sequence_group_skip_count;
    deck_info.find_all_sequences_flag = program_args_ptr->find_all_sequences_flag;
    display_8_sequences_min_flag = program_args_ptr->display_8_sequences_min_flag;
    display_input_sequences_flag = program_args_ptr->display_input_sequences_flag;
    cut_nine_of_diamonds_to_bottom = program_args_ptr->cut_nine_of_diamonds_to_bottom;

    // This do loop construct ends with "while (0)", so it is not a loop.
    // This allows using "break" to exit on errors and avoids deep-nesting
    // of code while still allowing a single "return" statement without
    // using a "goto" statement.
    do
    {
        status = _tfopen_s(&input_file_ptr,
                           input_file_name_ptr,
                           (CHAR_TYPE*)_T("r"));

        if (status != 0)
        {
            _tprintf(_T("Unable to open file %s\n"), input_file_name_ptr);
            break;
        }

        // Read a line of text from the file.
        while (_fgetts(&seq_array[0],
                       sizeof(seq_array) / sizeof(seq_array[0]),
                       input_file_ptr) != NULL)
        {
            // Remove the newline character in the input buffer.
            temp_ptr = _tcschr(&seq_array[0], _T('\n'));

            if (temp_ptr != NULL)
            {
                *temp_ptr = _T('\0');
            }

            // If the line contains the word "Found", then discard the line.
            seq_ptr = _tcsstr(&seq_array[0], _T("Found"));
            if (seq_ptr != NULL)
            {
                continue;
            }

            seq_ptr = _tcsstr(&seq_array[0], _T(":"));
                
            // If a colon character is not found, then this is assumed
            // to be a blank line.
            if (seq_ptr != NULL)
            {
                // Extract the name.
                name_ptr = &seq_array[0];
                while (*name_ptr == _T(' '))
                {
                    ++name_ptr;
                }

                // Now look for a space or colon character
                // and terminate the string at that character.
                temp_ptr = name_ptr;
                while (ISALPHA(*temp_ptr) || ISDIGIT(*temp_ptr))
                {
                    ++temp_ptr;
                }

                // Terminate the name.
                *temp_ptr = _T('\0');

                // Move past the colon character to point to the sequence.
                ++seq_ptr;

                // Advance the seq_ptr, until a non-space count is encountered.
                while (*seq_ptr == _T(' '))
                {
                    ++seq_ptr;
                }

                if (_tcscmp(name_ptr, _T("RED")) == 0)
                {
                    _tcscpy(red_sequence_array, seq_ptr);
                }
                else if (_tcscmp(name_ptr, _T("CD")) == 0)
                {
                    _tcscpy(cd_sequence_array, seq_ptr);
                }
                else if (_tcscmp(name_ptr, _T("HC")) == 0)
                {
                    _tcscpy(hc_sequence_array, seq_ptr);
                }
                else if (_tcscmp(name_ptr, _T("ODD")) == 0)
                {
                    _tcscpy(odd_sequence_array, seq_ptr);
                }
                else if (_tcscmp(name_ptr, _T("7K")) == 0)
                {
                    _tcscpy(c7k_sequence_array, seq_ptr);
                }
                else if (_tcscmp(name_ptr, _T("8K")) == 0)
                {
                    _tcscpy(c8k_sequence_array, seq_ptr);
                }
                else if (_tcscmp(name_ptr, _T("4T")) == 0)
                {
                    _tcscpy(c4t_sequence_array, seq_ptr);
                }
                else
                {
                    _tprintf(_T("malformed file. Unknown sequence type\n"));
                }
            }
            else
            {
                // Every 256 sequences, print the number of sequences processed.
                ++sequence_group_count;
                if ((sequence_group_count & 0xFF) == 0)
                {
                    _ftprintf(stderr, _T("\r%d"), sequence_group_count);
                }

                // Do not start processing sequences until the sequence
                // group skip count is exceeded.
                if (sequence_group_count > sequence_group_skip_count)
                {
                    // Convert each sequence to an integer representation.
                    seq_info.red_seq = bit_get_integer_seq(&red_sequence_array[0]);
                    seq_info.cd_seq = bit_get_integer_seq(&cd_sequence_array[0]);
                    seq_info.hc_seq = bit_get_integer_seq(&hc_sequence_array[0]);
                    seq_info.odd_seq = bit_get_integer_seq(&odd_sequence_array[0]);
                    seq_info.c7k_seq = bit_get_integer_seq(&c7k_sequence_array[0]);
                    seq_info.c8k_seq = bit_get_integer_seq(&c8k_sequence_array[0]);
                    seq_info.c4t_seq = bit_get_integer_seq(&c4t_sequence_array[0]);

                    // Verify that all the sequences are valid.
                    if (!bit_has_unique_subsequences(seq_info.red_seq)
                        || !bit_has_unique_subsequences(seq_info.cd_seq)
                        || !bit_has_unique_subsequences(seq_info.odd_seq)
                        || !bit_has_unique_subsequences(seq_info.c7k_seq)
                        || !bit_has_unique_subsequences(seq_info.c8k_seq)
                        || !bit_has_unique_subsequences(seq_info.c4t_seq))
                    {
                        _tprintf(_T("invalid sequence encountered\n"));
                        exit(-1);
                    }

                    // A blank line means a sequence is complete.
                    find_best_deck_order(&deck_info, &seq_info);

                    major_sequence_count = deck_info.deck_score >> 16;

                    // The ODD, RED, CD, and HC sequences are input sequences and
                    // are always supported. So, only 4 other value sequences
                    // are required to have at least 8 major sequences.
                    if (!display_8_sequences_min_flag || (major_sequence_count >= 8))
                    {
                        _tprintf(_T("Major sequence count = %d\n"), major_sequence_count);

                        // The deck duplcation count is in the low 16 bits of the deck score,
                        // and was stored as 65535 - dup_score to make the high score better
                        // to match the sequence count in the upper 16-bits, where a higher
                        // sequence count is better.
                        _tprintf(_T("Deck spread score = %d\n\n"), 65535 - (deck_info.deck_score & 65535));

                        // If cutting the deck, get the top card index that results
                        // in the Nine of Diamonds being on the bottom of the deck;
                        // otherwise, the top card index is zero. This is needed
                        // to rotate the input sequences so that they match the
                        // displayed deck.
                        if (cut_nine_of_diamonds_to_bottom)
                        {
                            top_card_index = deck_get_top_card_index(&deck_info);
                        }
                        else
                        {
                            top_card_index = 0;
                        }

                        // Conditionally display the input sequences.
                        if (display_input_sequences_flag)
                        {
                            bit_display_integer_sequence(_T(" 4T"),
                                                         seq_info.c4t_seq,
                                                         top_card_index,
                                                         deck_info.deck_length);

                            bit_display_integer_sequence(_T(" 7K"),
                                                         seq_info.c7k_seq,
                                                         top_card_index,
                                                         deck_info.deck_length);

                            bit_display_integer_sequence(_T(" 8K"),
                                                         seq_info.c8k_seq,
                                                         top_card_index,
                                                         deck_info.deck_length);

                            bit_display_integer_sequence(_T("ODD"),
                                                         seq_info.odd_seq,
                                                         top_card_index,
                                                         deck_info.deck_length);

                            bit_display_integer_sequence(_T("RED"),
                                                         seq_info.red_seq,
                                                         top_card_index,
                                                         deck_info.deck_length);

                            bit_display_integer_sequence(_T(" CD"),
                                                         seq_info.cd_seq,
                                                         top_card_index,
                                                         deck_info.deck_length);

                            bit_display_integer_sequence(_T(" HC"),
                                                         seq_info.hc_seq,
                                                         top_card_index,
                                                         deck_info.deck_length);

                            _tprintf(_T("\n"));
                        } // if (display_input_sequences_flag)

                        seq_display_sequence_matches(deck_info.sequence_type_bits);

                        deck_display_stack(&deck_info, top_card_index);
                    }   // if ((deck_info.sequence_type_bits != 0LL) ...
                }   //if (sequence_group_count > sequence_group_skip_count)

                red_sequence_array[0] = _T('\0');
                cd_sequence_array[0] = _T('\0');
                hc_sequence_array[0] = _T('\0');
                odd_sequence_array[0] = _T('\0');
                c7k_sequence_array[0] = _T('\0');
                c8k_sequence_array[0] = _T('\0');
                c4t_sequence_array[0] = _T('\0');
            }   // if (seq_ptr != NULL)
        }   // while (fgets(&seq_array[0], sizeof(seq_array), input_file_ptr))
    }   // do
    while (0);

    if (input_file_ptr != NULL)
    {
        fclose(input_file_ptr);
    }

    return 0;
}

// Create the initial deck possibilities.
void get_deck_possibilities(int * deck_array_ptr,
                            seq_info_t * seq_info_ptr)
{
    ASSERT(deck_array_ptr != NULL);

    // The value lookup array below is based on the following table.
    //
    // ----------------------------
    // ODD   7K   8K   4T   Card Values
    // ----------------------------
    //   0    0    0    0   TWO
    //   0    0    0    1   FOUR, SIX
    //   0    0    1    0   NONE
    //   0    0    1    1   NONE
    //   0    1    0    0   NONE
    //   0    1    0    1   NONE
    //   0    1    1    0   QUEEN
    //   0    1    1    1   EIGHT, TEN
    //   1    0    0    0   ACE, THREE
    //   1    0    0    1   FIVE
    //   1    0    1    0   NONE
    //   1    0    1    1   NONE
    //   1    1    0    0   NONE
    //   1    1    0    1   SEVEN
    //   1    1    1    0   JACK, KING

    int value_lookup_array[16] =
    {
        2, VALUE_FOUR_OR_SIX, 0, 0, 0, 0, 12, VALUE_EIGHT_OR_TEN,
        VALUE_ACE_OR_THREE, 5, 0, 0, 0, 7, VALUE_JACK_OR_KING, 9
    };

    uint64_t red_seq = seq_info_ptr->red_seq;
    uint64_t cd_seq = seq_info_ptr->cd_seq;
    uint64_t odd_seq = seq_info_ptr->odd_seq;
    uint64_t c7k_seq = seq_info_ptr->c7k_seq;
    uint64_t c8k_seq = seq_info_ptr->c8k_seq;
    uint64_t c4t_seq = seq_info_ptr->c4t_seq;

    int suit = 0;
    int value_code = 0;
    int value = 0;
    int i = 0;

    for (i = seq_info_ptr->n_bits_max - 1; i >= 0; --i)
    {
        suit = (int)(((red_seq & 1) << 1) | (cd_seq & 1));
        value_code = (int)(((odd_seq & 1) << 3) | ((c7k_seq & 1) << 2) \
            | ((c8k_seq & 1) << 1) | (c4t_seq & 1));
        value = value_lookup_array[value_code];
        deck_array_ptr[i] = (suit << 8) | value;
        red_seq = red_seq >> 1;
        cd_seq = cd_seq >> 1;
        odd_seq = odd_seq >> 1;
        c7k_seq = c7k_seq >> 1;
        c8k_seq = c8k_seq >> 1;
        c4t_seq =  c4t_seq >> 1;
    }
}

// Determine if any special sequences produce a valid Bracelet code.
int test_deck_for_special_sequences(deck_info_t * deck_info_ptr)
{
    ASSERT(deck_info_ptr != NULL);

    //static int cA6_value_array[6] = { 1, 2, 3, 4, 5, 6, };
    //static int cA7_value_array[7] = { 1, 2, 3, 4, 5, 6, 7 };
    static int c27_value_array[6] = { 2, 3, 4, 5, 6, 7 };
    static int c28_value_array[7] = { 2, 3, 4, 5, 6, 7, 8 };
    static int c38_value_array[6] = { 3, 4, 5, 6, 7, 8 };
    static int c39_value_array[7] = { 3, 4, 5, 6, 7, 8, 9 };
    static int c49_value_array[6] = { 4, 5, 6, 7, 8, 9 };
    //static int c4T_value_array[7] = { 4, 5, 6, 7, 8, 9, 10 };
    static int c5T_value_array[6] = { 5, 6, 7, 8, 9, 10 };
    static int c5J_value_array[7] = { 5, 6, 7, 8, 9, 10, 11 };
    static int c6J_value_array[6] = { 6, 7, 8, 9, 10, 11 };
    static int c6Q_value_array[7] = { 6, 7, 8, 9, 10, 11, 12 };
    static int c7Q_value_array[6] = { 7, 8, 9, 10, 11, 12 };
    //static int cEV_value_array[6] = { 2, 4, 6, 8, 10, 12 };
    // Some non-consecutive value sequences
    static int cM34_value_array[6] = { 3, 4, 6, 8, 9, 12 };
    static int cM46_value_array[6] = { 4, 5, 6, 8, 10, 12 };
    static int cM47_value_array[7] = { 4, 5, 6, 7, 8, 10, 12 };
    static int cM58_value_array[6] = { 5, 6, 7, 8, 10, 12 };
    static int cM59_value_array[7] = { 5, 6, 7, 8, 9, 10, 12 };
    static int cM6Q_value_array[6] = { 6, 7, 8, 9, 10, 12 };
    static int cPR_value_array[6] = { 2, 3, 5, 7, 11, 13 };
    static int cFI_value_array[6] = { 1, 2, 3, 5, 8, 13 };
    static int cLU_value_array[6] = { 1, 2, 3, 4, 7, 11 };
    uint64_t sequence = 0LL;
    int sequence_count = 0;
    int sequence_type_bits = 0;
    BOOL is_valid_flag = FALSE;

    deck_info_ptr->sequence_count = 0LL;

    // The following "do" statement does not loop. The statement is
    // used to allow using "break" to avoid the use "goto" statements.
    do
    {
        // Add all the input sequences. The code for the consecutive value
        // input sequences is commented out below.
        // The A6 and A7 sequences are always supported because they are
        // the inverse of the corresponding 8K and 7K input sequences.
        // The EV sequence is the inverse of the ODD sequence.
        sequence_type_bits |= (SEQ_A6 | SEQ_A7 | SEQ_4T | SEQ_EV | SEQ_HD | SEQ_CD | SEQ_HC);

        //CHECK_FOR_BRACELET_SEQUENCE(cA6_value_array, SEQ_A6)
        //CHECK_FOR_BRACELET_SEQUENCE(cA7_value_array, SEQ_A7)
        CHECK_FOR_BRACELET_SEQUENCE(c27_value_array, SEQ_27)
        CHECK_FOR_BRACELET_SEQUENCE(c28_value_array, SEQ_28)
        CHECK_FOR_BRACELET_SEQUENCE(c38_value_array, SEQ_38)
        CHECK_FOR_BRACELET_SEQUENCE(c39_value_array, SEQ_39)
        CHECK_FOR_BRACELET_SEQUENCE(c49_value_array, SEQ_49)
        //CHECK_FOR_BRACELET_SEQUENCE(c4T_value_array, SEQ_4T)
        CHECK_FOR_BRACELET_SEQUENCE(c5T_value_array, SEQ_5T)
        CHECK_FOR_BRACELET_SEQUENCE(c5J_value_array, SEQ_5J)
        CHECK_FOR_BRACELET_SEQUENCE(c6J_value_array, SEQ_6J)
        CHECK_FOR_BRACELET_SEQUENCE(c6Q_value_array, SEQ_6Q)
        CHECK_FOR_BRACELET_SEQUENCE(c7Q_value_array, SEQ_7Q)
        //CHECK_FOR_BRACELET_SEQUENCE(cEV_value_array, SEQ_EV)

        // Should special sequences be checked?
        if (deck_info_ptr->find_all_sequences_flag)
        {
            CHECK_FOR_BRACELET_SEQUENCE(cM34_value_array, SEQ_M34)
            CHECK_FOR_BRACELET_SEQUENCE(cM46_value_array, SEQ_M46)
            CHECK_FOR_BRACELET_SEQUENCE(cM47_value_array, SEQ_M47)
            CHECK_FOR_BRACELET_SEQUENCE(cM58_value_array, SEQ_M58)
            CHECK_FOR_BRACELET_SEQUENCE(cM59_value_array, SEQ_M59)
            CHECK_FOR_BRACELET_SEQUENCE(cM6Q_value_array, SEQ_M6Q)
            CHECK_FOR_BRACELET_SEQUENCE(cPR_value_array, SEQ_PR)
            CHECK_FOR_BRACELET_SEQUENCE(cFI_value_array, SEQ_FI)
            CHECK_FOR_BRACELET_SEQUENCE(cLU_value_array, SEQ_LU)
        } // // Should special sequences be checked?
    } while (0);

    deck_info_ptr->sequence_count = sequence_count;

    return sequence_type_bits;
}

// Get the deck score.
int get_deck_score(deck_info_t * deck_info_ptr)
{
    ASSERT(deck_info_ptr != NULL);

    int sequence_type_bits = test_deck_for_special_sequences(deck_info_ptr);

    // Get the number os sequences.
    int deck_score = bit_count_set_bits(sequence_type_bits);

    // Weight the number of sequences so highly that it is
    // always more important that the deck duplication count.
    deck_score <<= 16;

    // Decrease the score by the deck duplication count.
    deck_score |= deck_get_dup_count_score(deck_info_ptr);

    return deck_score;
}

// Create decks from sequences.
void find_best_deck_order(deck_info_t * deck_info_ptr,
                          seq_info_t * seq_info_ptr)
{
    ASSERT(deck_info_ptr != NULL);
    ASSERT(seq_info_ptr != NULL);

    int xa3_spade_array[2] = {0, 0};
    int x46_spade_array[2] = {0, 0};
    int x8t_spade_array[2] = {0, 0};
    int xjk_spade_array[2] = {0, 0};
    int xa3_heart_array[2] = {0, 0};
    int x46_heart_array[2] = {0, 0};
    int x8t_heart_array[2] = {0, 0};
    int xjk_heart_array[2] = {0, 0};
    int xa3_club_array[2] = {0, 0};
    int x46_club_array[2] = {0, 0};
    int x8t_club_array[2] = {0, 0};
    int xjk_club_array[2] = {0, 0};
    int xa3_diamond_array[2] = {0, 0};
    int x46_diamond_array[2] = {0, 0};
    int x8t_diamond_array[2] = {0, 0};
    int xjk_diamond_array[2] = {0, 0};
    int suit = 0;
    int value = 0;
    int i = 0;
    int j = 0;
    int x = 0;
    int xa3s = 0;
    int x46s = 0;
    int x8ts = 0;
    int xjks = 0;
    int xa3h = 0;
    int x46h = 0;
    int x8th = 0;
    int xjkh = 0;
    int xa3c = 0;
    int x46c = 0;
    int x8tc = 0;
    int xjkc = 0;
    int xa3d = 0;
    int x46d = 0;
    int x8td = 0;
    int xjkd = 0;
    int index0 = 0;
    int index1 = 0;
    int deck_score = 0;
    int best_score = TINY_DECK_SCORE;
    int sequence_type_bits = 0;
    int best_sequence_type_bits = 0;
    int maximum_sequence_count = 0;
    int first_sequence_found = TRUE;

    memset(&deck_info_ptr->deck_array[0], 0, seq_info_ptr->n_bits_max);
    memset(&deck_info_ptr->temp_deck_array[0], 0, seq_info_ptr->n_bits_max);

    // The following "do" statement does not loop. The statement is
    // used to allow using "break" to avoid the use "goto" statements.
    do
    {
        // Create the initial deck possibilities.
        get_deck_possibilities(&deck_info_ptr->temp_deck_array[0],
                               seq_info_ptr);

        // Were there any illegal codes?
        if (deck_is_value_in_array(value,
                                   &deck_info_ptr->temp_deck_array[0],
                                   seq_info_ptr->n_bits_max))
        {
            _tprintf(_T("illegal code encountered\n"));
            break;
        }

        // Create the decks.
        // Get the indices of the cards where there are two choices.
        for (i = 0; i < seq_info_ptr->n_bits_max; ++i)
        {
            // Add the cards.
            value = deck_info_ptr->temp_deck_array[i] & 0xFF;
            suit = deck_info_ptr->temp_deck_array[i] >> 8;

            // Get the indices for the cards that can be swapped.
            if (value == VALUE_ACE_OR_THREE)
            {
                if (suit == SUIT_SPADE)
                {
                    xa3_spade_array[xa3s] = i;
                    ++xa3s;
                }
                else if (suit == SUIT_HEART)
                {
                    xa3_heart_array[xa3h] = i;
                    ++xa3h;
                }
                else if (suit == SUIT_CLUB)
                {
                    xa3_club_array[xa3c] = i;
                    ++xa3c;
                }
                else
                {
                    xa3_diamond_array[xa3d] = i;
                    ++xa3d;
                }
            }
            else if (value == VALUE_FOUR_OR_SIX)
            {
                if (suit == SUIT_SPADE)
                {
                    x46_spade_array[x46s] = i;
                    ++x46s;
                }
                else if (suit == SUIT_HEART)
                {
                    x46_heart_array[x46h] = i;
                    ++x46h;
                }
                else if (suit == SUIT_CLUB)
                {
                    x46_club_array[x46c] = i;
                    ++x46c;
                }
                else
                {
                    x46_diamond_array[x46d] = i;
                    ++x46d;
                }
            }
            else if (value == VALUE_EIGHT_OR_TEN)
            {
                if (suit == SUIT_SPADE)
                {
                    x8t_spade_array[x8ts] = i;
                    ++x8ts;
                }
                else if (suit == SUIT_HEART)
                {
                    x8t_heart_array[x8th] = i;
                    ++x8th;
                }
                else if (suit == SUIT_CLUB)
                {
                    x8t_club_array[x8tc] = i;
                    ++x8tc;
                }
                else
                {
                    x8t_diamond_array[x8td] = i;
                    ++x8td;
                }
            }
            else if (value == VALUE_JACK_OR_KING)
            {
                if (suit == SUIT_SPADE)
                {
                    xjk_spade_array[xjks] = i;
                    ++xjks;
                }
                else if (suit == SUIT_HEART)
                {
                    xjk_heart_array[xjkh] = i;
                    ++xjkh;
                }
                else if (suit == SUIT_CLUB)
                {
                    xjk_club_array[xjkc] = i;
                    ++xjkc;
                }
                else
                {
                    xjk_diamond_array[xjkd] = i;
                    ++xjkd;
                }
            }
        }   // for (i = 0; i < seq_info_ptr->n_bits_max; ++i)

        // Is the sequence valid?
        if ((xa3s != 2) || (x46s != 2) || (x8ts != 2) || (xjks != 2)
            || (xa3h != 2) || (x46h != 2) || (x8th != 2) || (xjkh != 2)
            || (xa3c != 2) || (x46c != 2) || (x8tc != 2) || (xjkc != 2)
            || (xa3d != 2) || (x46d != 2) || (x8td != 2) || (xjkd != 2))
        {
            break;
        }

        // Now find and test all 2^16 possible deck orders for the current sequences.
        for (i = 0; i < 65536; ++i)
        {
            x = i;
            xa3s = x & 1;
            x = x >> 1;
            x46s = x & 1;
            x = x >> 1;
            x8ts = x & 1;
            x = x >> 1;
            xjks = x & 1;
            x = x >> 1;
            xa3h = x & 1;
            x = x >> 1;
            x46h = x & 1;
            x = x >> 1;
            x8th = x & 1;
            x = x >> 1;
            xjkh = x & 1;
            x = x >> 1;
            xa3c = x & 1;
            x = x >> 1;
            x46c = x & 1;
            x = x >> 1;
            x8tc = x & 1;
            x = x >> 1;
            xjkc = x & 1;
            x = x >> 1;
            xa3d = x & 1;
            x = x >> 1;
            x46d = x & 1;
            x = x >> 1;
            x8td = x & 1;
            x = x >> 1;
            xjkd = x & 1;

            // Set the A3 cards for each suit.
            // A3 Spades
            index0 = xa3_spade_array[xa3s];
            index1 = xa3_spade_array[1 - xa3s];
            deck_info_ptr->temp_deck_array[index0] = 1;  // AS (SUIT_SPADE is zero)
            deck_info_ptr->temp_deck_array[index1] = 3;  // 3S

            // A3 Hearts
            index0 = xa3_heart_array[xa3h];
            index1 = xa3_heart_array[1 - xa3h];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_HEART << 8) | 1; // AH
            deck_info_ptr->temp_deck_array[index1] = (SUIT_HEART << 8) | 3; // 3H

            // A3 Clubs
            index0 = xa3_club_array[xa3c];
            index1 = xa3_club_array[1 - xa3c];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_CLUB << 8) | 1; // AC
            deck_info_ptr->temp_deck_array[index1] = (SUIT_CLUB << 8) | 3; // 3C

            // A3 Diamonds
            index0 = xa3_diamond_array[xa3d];
            index1 = xa3_diamond_array[1 - xa3d];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_DIAMOND << 8) | 1; // AD
            deck_info_ptr->temp_deck_array[index1] = (SUIT_DIAMOND << 8) | 3; // 3D

            // Set the 46 cards for each suit.
            // 46 Spades
            index0 = x46_spade_array[x46s];
            index1 = x46_spade_array[1 - x46s];
            deck_info_ptr->temp_deck_array[index0] = 4; // 4S
            deck_info_ptr->temp_deck_array[index1] = 6; // 6S

            // 46 Hearts
            index0 = x46_heart_array[x46h];
            index1 = x46_heart_array[1 - x46h];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_HEART << 8) | 4; // 4H
            deck_info_ptr->temp_deck_array[index1] = (SUIT_HEART << 8) | 6; // 6H

            // 46 Clubs
            index0 = x46_club_array[x46c];
            index1 = x46_club_array[1 - x46c];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_CLUB << 8) | 4; // 4C
            deck_info_ptr->temp_deck_array[index1] = (SUIT_CLUB << 8) | 6; // 6C

            // 46 Diamonds
            index0 = x46_diamond_array[x46d];
            index1 = x46_diamond_array[1 - x46d];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_DIAMOND << 8) | 4; // 4D
            deck_info_ptr->temp_deck_array[index1] = (SUIT_DIAMOND << 8) | 6; // 6D

            // Set the 81 cards for each suit.
            // 81 Spades
            index0 = x8t_spade_array[x8ts];
            index1 = x8t_spade_array[1 - x8ts];
            deck_info_ptr->temp_deck_array[index0] = 8; // 8S
            deck_info_ptr->temp_deck_array[index1] = 10; // 10S

            // 81 Hearts
            index0 = x8t_heart_array[x8th];
            index1 = x8t_heart_array[1 - x8th];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_HEART << 8) | 8; // 8H
            deck_info_ptr->temp_deck_array[index1] = (SUIT_HEART << 8) | 10; // 10H

            // 81 Clubs
            index0 = x8t_club_array[x8tc];
            index1 = x8t_club_array[1 - x8tc];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_CLUB << 8) | 8; // 8C
            deck_info_ptr->temp_deck_array[index1] = (SUIT_CLUB << 8) | 10; // 10C

            // 81 Diamonds
            index0 = x8t_diamond_array[x8td];
            index1 = x8t_diamond_array[1 - x8td];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_DIAMOND << 8) | 8; // 8D
            deck_info_ptr->temp_deck_array[index1] = (SUIT_DIAMOND << 8) | 10; // 10D

            // Set the JK cards for each suit.
            // JK Spades
            index0 = xjk_spade_array[xjks];
            index1 = xjk_spade_array[1 - xjks];
            deck_info_ptr->temp_deck_array[index0] = 11; // JS
            deck_info_ptr->temp_deck_array[index1] = 13; // KS

            // JK Hearts
            index0 = xjk_heart_array[xjkh];
            index1 = xjk_heart_array[1 - xjkh];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_HEART << 8) | 11; // JH
            deck_info_ptr->temp_deck_array[index1] = (SUIT_HEART << 8) | 13; // KH

            // JK Clubs
            index0 = xjk_club_array[xjkc];
            index1 = xjk_club_array[1 - xjkc];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_CLUB << 8) | 11; // JC
            deck_info_ptr->temp_deck_array[index1] = (SUIT_CLUB << 8) | 13; // KC

            // JK Diamonds
            index0 = xjk_diamond_array[xjkd];
            index1 = xjk_diamond_array[1 - xjkd];
            deck_info_ptr->temp_deck_array[index0] = (SUIT_DIAMOND << 8) | 11; // JD
            deck_info_ptr->temp_deck_array[index1] = (SUIT_DIAMOND << 8) | 13; // KD

            sequence_type_bits = test_deck_for_special_sequences(deck_info_ptr);

            if (sequence_type_bits != 0)
            {
                if (deck_info_ptr->sequence_count >= maximum_sequence_count)
                {
                    maximum_sequence_count = deck_info_ptr->sequence_count;

                    // If the first sequence is found, reset the best deck score
                    // to a high value so that this sequence is not ignored.
                    if (first_sequence_found)
                    {
                        first_sequence_found = FALSE;
                        best_score = TINY_DECK_SCORE;
                    }

                    // Find the deck score.
                    deck_score = get_deck_score(deck_info_ptr);

                    // If the new deck score is higher, then save the new score and
                    // the new deck order.
                    if (best_score < deck_score)
                    {
                        best_score = deck_score;
                        best_sequence_type_bits = sequence_type_bits;

                        // Copy the deck into best-deck array.
                        for (j = 0; j < deck_info_ptr->deck_length; ++j)
                        {
                            deck_info_ptr->deck_array[j] = deck_info_ptr->temp_deck_array[j];
                        }
                    }
                }
            }
            else if (deck_info_ptr->deck_array[0] == 0)
            {
                // Get the deck score for the current deck order.
                best_score = get_deck_score(deck_info_ptr);

                // Copy the deck into the passed deck_info_t structure.
                for (j = 0; j < seq_info_ptr->n_bits_max; ++j)
                {
                    deck_info_ptr->deck_array[j] = deck_info_ptr->temp_deck_array[j];
                }
            }
        }   // for (i = 0; i < 65536; ++i)
    }   // do
    while (0);

    // Pass back the results to the caller.
    deck_info_ptr->deck_score = best_score;
    deck_info_ptr->sequence_type_bits = best_sequence_type_bits;
}

/* Function: display_usage */
void display_usage()
{
    _tprintf(_T("\n"));
    _tprintf(_T("Program umake_decks\n"));
    _tprintf(_T("Copyright (c) 2018-2024, William Hallahan - All Rights Reserved.\n\n"));
    _tprintf(_T("This program generates decks orders using input created by\n"));
    _tprintf(_T("program ultimate_search.\n\n"));
    _tprintf(_T("Usage:\n\n"));
    _tprintf(_T("    umake_decks [-f | --find] [-e | --eight] [-c | --cut]\n"));
    _tprintf(_T("                [-i | --input] <sequence-file-name>\n\n"));
    _tprintf(_T("\nOptional arguments:\n\n"));
    _tprintf(_T("-s <seq-group-skip-count> | --skip <seq-group-skip-count>\n"));
    _tprintf(_T("               Skip the specified number of sequence groups.\n"));
    _tprintf(_T("-a, --all      If specified, also find all special sequences. Special\n"));
    _tprintf(_T("               sequences are NOT suit sequence, odd or even sequences\n"));
    _tprintf(_T("               or consecutive card sequences, and includes strange,\n"));
    _tprintf(_T("               choices, such as card values that are prime numbers.\n"));
    _tprintf(_T("-c, --cut      If specified, cut the Nine of Diamonds to the bottom.\n"));
    _tprintf(_T("-e, --eight    If specified, only display results if at least 8 sequences.\n"));
    _tprintf(_T("-i, --input    If specified, display the binary input sequences read\n"));
    _tprintf(_T("               from the sequence input file. Before being displayed, the\n"));
    _tprintf(_T("               sequences are adjusted to correct for if the deck is cut.\n"));
    _tprintf(_T("-h, --help     Display program help.\n\n"));
}
