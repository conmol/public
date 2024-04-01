//======================================================================
// File: ultimate_search.c
// Author: Bill Hallahan
// Date: July 31, 2018
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
#include "../common/arg_get_arguments.h"
#include "../common/f64_seq.h"
#include "../common/bit_utilities.h"
#include "../common/platform_utilities.h"

const int N_BITS_MAX = 52;
const int NUMBER_OF_CARDS_IN_A_SUIT = 13;
const int NUMBER_OF_CARDS_IN_TWO_SUITS = 26;

/* Structure type used for program arguments. */
typedef struct
{
    int red_start_count;
    int cd_start_count;
    int odd_start_count;
    int c7k_start_count;
    BOOL value_sequence_modifier_flag;
    BOOL suit_sequence_modifier_flag;
    BOOL c8k_sequence_flag;
    BOOL sevens_apart_flag;
    BOOL c4t_sequence_flag;
} ProgramArgument_T;

typedef struct ultimate_struct_s
{
    uint64_t red_sequence;
    uint64_t cd_sequence;
    uint64_t hc_sequence;
    uint64_t spade_bits;
    uint64_t heart_bits;
    uint64_t club_bits;
    uint64_t diamond_bits;
    uint64_t odd_sequence;
    uint64_t c7k_sequence;
    uint64_t c8k_sequence;
    uint64_t c4t_sequence;
    uint64_t c7k_c8k_diff;
    uint64_t sequence_mask;
    uint64_t subsequence_mask;
    unsigned int subsequence_length;
    unsigned int n_bits_max;
    BOOL sevens_apart_flag;
} ultimate_struct_t;

/* Forward declarations. */
int ultimate_search_main(ProgramArgument_T * program_args_ptr);

static void display_sequence(TCHAR * text_ptr,
                             uint64_t sequence,
                             unsigned int length);

// Count the number of bits set in a 64-bit unsigned integer.
static unsigned int count_set_bits_64(uint64_t value,
                                      int max_bit_count);

// Validate that the sequences overlap properly for a potential suit sequence.
static int is_valid_suit_seq_overlap(uint64_t seq0,
                                     uint64_t seq1,
                                     uint64_t sequence_mask);

static int is_valid_odd_sequence(ultimate_struct_t * seq_set_ptr);

//static void get_overlap_counts(uint64_t seq0,
//                               uint64_t seq1,
//                               uint64_t sequence_mask,
//                               unsigned int * zero_zero_count_ptr,
//                               unsigned int * zero_one_count_ptr,
//                               unsigned int * one_zero_count_ptr,
//                               unsigned int * one_one_count_ptr);

static void get_overlap_counts(uint64_t seq0,
                               uint64_t seq1,
                               uint64_t sequence_mask,
                               unsigned int * zero_zero_count_ptr);

static int is_valid_c7k_sequence(ultimate_struct_t * seq_set_ptr);

static int is_valid_deck_sequence(ultimate_struct_t * seq_set_ptr);

// Check if the sequence can have four "odd" one bits of a sequence
// with a density of 28 changed to zeros and still be a valid sequence.
static int search_for_c8k_sequence(ultimate_struct_t * seq_set_ptr);

// Verify the four difference bits in the 7k-8k diff sequence are valid.
static int c7k_c8k_diff_valid(ultimate_struct_t * seq_set_ptr);

// Check if the sequence can support the 4T cards,
// which is 7 plus or minus 3.
static int search_for_c4t_sequence(ultimate_struct_t * seq_set_ptr);

// Check if the sequence has either six zeros or six ones.
static int has_long_uniform_subsequence(uint64_t seq);

// Display help text
void display_usage();

/* Start of main program. */
int _tmain(int argc, TCHAR * argv[])
{
    TCHAR * stop_ptr = NULL;
    int arg_index = 0;
    int option_int = 0;
    int status = 0;

    ProgramArgument_T program_args;

    static const arg_option_t argument_option_array[] =
    {
        { _T('r'), _T("red"), OPTION_REQUIRES_PARAMETER, },
        { _T('c'), _T("cd"), OPTION_REQUIRES_PARAMETER, },
        { _T('o'), _T("odd"), OPTION_REQUIRES_PARAMETER, },
        { _T('k'), _T("c7k"), OPTION_REQUIRES_PARAMETER, },
        { _T('n'), _T("valshort"), OPTION_NO_PARAMETER, },
        { _T('s'), _T("suitshort"), OPTION_NO_PARAMETER, },
        { _T('e'), _T("c8k"), OPTION_NO_PARAMETER, },
        { _T('p'), _T("sep"), OPTION_NO_PARAMETER, },
        { _T('t'), _T("c4t"), OPTION_NO_PARAMETER, },
        { _T('h'), _T("help"), OPTION_NO_PARAMETER },
        { 0, NULL, OPTION_NO_PARAMETER }
    };

    /* Initialize the program argument structure. */
    program_args.red_start_count = 0;
    program_args.cd_start_count = 0;
    program_args.odd_start_count = 0;
    program_args.c7k_start_count = 0;
    program_args.value_sequence_modifier_flag = FALSE;
    program_args.suit_sequence_modifier_flag = FALSE;
    program_args.c8k_sequence_flag = FALSE;
    program_args.sevens_apart_flag = FALSE;
    program_args.c4t_sequence_flag = FALSE;

    /* Call the get_arguments function to populate the argument variables. */
    while ((status = arg_get_arguments(argc,
                                       argv,
                                       &argument_option_array[0],
                                       &arg_index,
                                       &option_int)) > ARG_NO_MORE_ARGUMENTS)
    {
        if (status == ARG_OPTIONAL_ARGUMENT)
        {
            /* Process an optional argument. */
            switch (option_int)
            {
            case _T('r'):
            {
                program_args.red_start_count = _tcstol(argv[arg_index], &stop_ptr, 10);

                if (_tcslen(stop_ptr) > 0)
                {
                    _tprintf(_T("Error in argument %s.\n"), argv[arg_index]);
                    status = -1;
                }
            }
            break;

            case _T('c'):
            {
                program_args.cd_start_count = _tcstol(argv[arg_index], &stop_ptr, 10);
                if (_tcslen(stop_ptr) > 0)
                {
                    _tprintf(_T("Error in argument %s.\n"), argv[arg_index]);
                    status = -1;
                }
            }
            break;

            case _T('o'):
            {
                program_args.odd_start_count = _tcstol(argv[arg_index], &stop_ptr, 10);
                if (_tcslen(stop_ptr) > 0)
                {
                    _tprintf(_T("Error in argument %s.\n"), argv[arg_index]);
                    status = -1;
                }
            }
            break;

            case _T('k'):
            {
                program_args.c7k_start_count = _tcstol(argv[arg_index], &stop_ptr, 10);
                if (_tcslen(stop_ptr) > 0)
                {
                    _tprintf(_T("Error in argument %s.\n"), argv[arg_index]);
                    status = -1;
                }
            }
            break;

            case _T('n'):
            {
                program_args.value_sequence_modifier_flag = TRUE;
            }
            break;

            case _T('s'):
            {
                program_args.suit_sequence_modifier_flag = TRUE;
            }
            break;

            case _T('e'):
            {
                program_args.c8k_sequence_flag = TRUE;
            }
            break;

            case _T('p'):
            {
                program_args.sevens_apart_flag = TRUE;
            }
            break;

            case _T('t'):
            {
                program_args.c4t_sequence_flag = TRUE;
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
                /* It should be impossible to reach here. */
                _tprintf(_T("Program error.  Contact support.\n"));
                status = ARG_EXIT_PROGRAM;
            }
            break;
            }
        }
        else
        {
            _tprintf(_T("Illegal argument \"%s\".\n"), argv[arg_index]);
            status = ARG_EXIT_PROGRAM;
            break;
        }

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
        if (program_args.red_start_count < 0)
        {
            _tprintf(_T("The HD (RED) start count must be a positive number.\n"));
            status = -1;
        }
        else if (program_args.cd_start_count < 0)
        {
            _tprintf(_T("The CD start count must be a positive number.\n"));
            status = -1;
        }
        else if (program_args.odd_start_count < 0)
        {
            _tprintf(_T("The ODD start count must be a positive number.\n"));
            status = -1;
        }
        else if (program_args.c7k_start_count < 0)
        {
            _tprintf(_T("The 7K start count must be a positive number.\n"));
        }
        else
        {
            // Before calling the main program, print the program name
            // and the program arguments.
            for (int i = 0; i < argc; ++i)
            {
                _tprintf(_T("%s "), argv[i]);
            }

            _tprintf(_T("\n\n"));

            /* Process the input data. */
            status = ultimate_search_main(&program_args);
        }
    } // if (status == 0)

    return status;
}

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

    _tprintf(_T("%s sequence:  %s\n"), text_ptr, &text_array[0]);
}

#define DBN_MAX_FILE_NAME_LENGTH (1024)
#define DBN_SUIT_FILE_NAME _T("dbn_52_26")
#define DBN_VALUE_FILE_NAME _T("dbn_52_28")
#define DBN_SHORT_FILE_NAME_SUFFIX _T("_short")

/* Function: ultimate_search_main */
int ultimate_search_main(ProgramArgument_T * program_args_ptr)
{
    ASSERT(program_args_ptr != NULL);

    TCHAR dbn_suit_file_name_array[DBN_MAX_FILE_NAME_LENGTH];
    TCHAR dbn_value_file_name_array[DBN_MAX_FILE_NAME_LENGTH];
    f64_seq_handle_t dbn_red_handle;
    f64_seq_handle_t dbn_cd_handle;
    f64_seq_handle_t dbn_odd_handle;
    f64_seq_handle_t dbn_c7k_handle;
    ultimate_struct_t seq_set = { 0LL, 0LL, 0LL, 0LL, 0LL, 0LL, 0LL, 0LL, 0LL, \
                                  0LL, 0LL, 0LL, 0LL, 0LL, 0, 0, FALSE };
    uint64_t inv_red_sequence = 0LL;
    uint64_t inv_cd_sequence = 0LL;
    unsigned int diamond_bit_count = 0;
    unsigned int spade_bit_count = 0;
    unsigned int club_bit_count = 0;
    unsigned int heart_bit_count = 0;
    unsigned int hc_bit_count = 0;
    unsigned int red_start_count = 0;
    unsigned int cd_start_count = 0;
    unsigned int odd_start_count = 0;
    unsigned int c7k_start_count = 0;
    BOOL c8k_sequence_flag = FALSE;
    BOOL c4t_sequence_flag = FALSE;
    int not_done = TRUE;
    BOOL is_valid_flag = FALSE;
    int ired = 0;
    int icd = 0;
    int iodd = 0;
    int i7k = 0;
    int text_length = 0;
    int status = 0;

    // Read environment variable "DBNPATH" to get the path
    // to the 64-bit binary sequence files..
    static const TCHAR * f64_env_to_path_ptr = _T("DBNPATH");

    text_length = plt_read_env_var(f64_env_to_path_ptr,
                              &dbn_suit_file_name_array[0],
                              DBN_MAX_FILE_NAME_LENGTH);

    if (text_length == 0)
    {
        _tprintf(_T("Error: Unable to get path from environment variable \"DBNPATH\"\n"));
        fflush(NULL);
        _exit(-1);
    }
    else if (text_length > (DBN_MAX_FILE_NAME_LENGTH - 20))
    {
        // The longest file name is "dbn_52_26_short.bin".
        // That is 18 characters long. Add one to be able to
        // add a path separator character and one for the
        // terminating NULL character to get 20.
        _tprintf(_T("Error: path in environment variable \"DBNPATH\" must not be longer than %d\n"),
                 DBN_MAX_FILE_NAME_LENGTH - 20);
        fflush(NULL);
        _exit(-1);
    }

    // The path to the 64-bit binary sequence files is now in array
    // dbn_suit_file_name_array. Check for a terminating NULL character
    // and add one if needed.
    TCHAR last_char = dbn_suit_file_name_array[text_length - 1];

    // This should possibly just check for the path separator for either
    // Windows(TM) or for Linux, however, for now either are accepted.
    if ((last_char != _T('\\')) && (last_char != _T('/')))
    {
        // There is no ending path separator character. Add one.
        dbn_suit_file_name_array[text_length++] = PLT_PATH_SEP_CHAR;
        // Null-terminate the string.
        dbn_suit_file_name_array[text_length] = _T('\0');
    }

    // Copy the path in the suit file name array to the value file name array.
    _tcscpy_s(&dbn_value_file_name_array[0],
              DBN_MAX_FILE_NAME_LENGTH,
              &dbn_suit_file_name_array[0]);

    // Add the first part of the file name to the suit path.
    _tcscat_s(&dbn_suit_file_name_array[0],
              DBN_MAX_FILE_NAME_LENGTH,
              DBN_SUIT_FILE_NAME);

    // Add the first part of the file name to the value path.
    _tcscat_s(&dbn_value_file_name_array[0],
              DBN_MAX_FILE_NAME_LENGTH,
              DBN_VALUE_FILE_NAME);

    // Optionally append "_short" for the suit sequences
    // that do NOT contains codes that are all zeros or
    // all ones.
    if (program_args_ptr->suit_sequence_modifier_flag)
    {
        _tcscat_s(&dbn_suit_file_name_array[0],
                  DBN_MAX_FILE_NAME_LENGTH,
                  DBN_SHORT_FILE_NAME_SUFFIX);
    }

    // Optionally append "_short" for the value sequences
    // that do NOT contains codes that are all zeros or
    // all ones.
    if (program_args_ptr->value_sequence_modifier_flag)
    {
        _tcscat_s(&dbn_value_file_name_array[0],
                  DBN_MAX_FILE_NAME_LENGTH,
                  DBN_SHORT_FILE_NAME_SUFFIX);
    }

    // Add the file extension to the suit path and file name.
    _tcscat_s(dbn_suit_file_name_array,
              DBN_MAX_FILE_NAME_LENGTH,
              _T(".bin"));

    // Add the file extension to the value path and file name.
    _tcscat_s(dbn_value_file_name_array,
              DBN_MAX_FILE_NAME_LENGTH,
              _T(".bin"));

    // Copy some program arguments to local variables.
    red_start_count = program_args_ptr->red_start_count;
    cd_start_count = program_args_ptr->cd_start_count;
    odd_start_count = program_args_ptr->odd_start_count;
    c7k_start_count = program_args_ptr->c7k_start_count;

    // The 8k sequence is found using the special 'search_for_c8k_sequence' function.
    c8k_sequence_flag = program_args_ptr->c8k_sequence_flag;
    seq_set.sevens_apart_flag = program_args_ptr->sevens_apart_flag;
    c4t_sequence_flag = program_args_ptr->c4t_sequence_flag;

    // Create a generator for the red sequence.
    status = f64_make(&dbn_red_handle, &dbn_suit_file_name_array[0]);
    //status = f64_make(&dbn_red_handle, DBN_RED_FILE_NAME);
 
    if (status != F64_SUCCESS)
    {
        _tprintf(_T("f64_make HD failed\n"));
        fflush(NULL);
        _exit(-1);
    }

    // Create a generator for the cd sequence.
    status = f64_make(&dbn_cd_handle, &dbn_suit_file_name_array[0]);

    if (status != F64_SUCCESS)
    {
        _tprintf(_T("f64_make CD failed\n"));
        fflush(NULL);
        _exit(-1);
    }

    // Create a generator for the odd sequence.
    status = f64_make(&dbn_odd_handle, &dbn_value_file_name_array[0]);

    if (status != F64_SUCCESS)
    {
        _tprintf(_T("f64_make ODD failed\n"));
        fflush(NULL);
        _exit(-1);
    }

    // Create a generator for the hi sequence.
    status = f64_make(&dbn_c7k_handle, &dbn_value_file_name_array[0]);

    if (status != F64_SUCCESS)
    {
        _tprintf(_T("f64_make c7K failed\n"));
        fflush(NULL);
        _exit(-1);
    }

    // Make a bit mask for n_bits_max bits.
    seq_set.n_bits_max = N_BITS_MAX;
    seq_set.sequence_mask = ((1LL << seq_set.n_bits_max) - 1LL);
    seq_set.subsequence_mask = SUBSEQUENCE_MASK;
    seq_set.subsequence_length = 6;

    // Optionally skip some red sequences.
    if (red_start_count > 0)
    {
        for (ired = 0; ired < (int)red_start_count; ++ired)
        {
            seq_set.red_sequence = f64_next(dbn_red_handle);
        }

        // Only start deep for the first red sequence.
        red_start_count = 0;
    }

    // Start of main search loop.
    while (not_done)
    {
        seq_set.red_sequence = 0;

        do
        {
            seq_set.red_sequence = f64_next(dbn_red_handle);

        } while (has_long_uniform_subsequence(seq_set.red_sequence) || (seq_set.red_sequence == 0LL));

        if (seq_set.red_sequence == 0LL)
        {
            _tprintf(_T("Processed all HD sequences. Exiting"));
            break;
        }

        // Reset the cd sequence generator.
        f64_reset(dbn_cd_handle);

        // Optionally skip some cd sequences.
        if (cd_start_count > 0)
        {
            for (icd = 0; icd < (int)cd_start_count; ++icd)
            {
                seq_set.cd_sequence = f64_next(dbn_cd_handle);
            }

            // Only start deep for the first cd sequence.
            cd_start_count = 0;
        }

        while (TRUE)
        {
            // Get a cd sequence.
            seq_set.cd_sequence = f64_next(dbn_cd_handle);

            if (seq_set.cd_sequence == 0LL)
            {
                break;
            }

            // Check that the sequences overlap with the right bit counts.
            is_valid_flag = is_valid_suit_seq_overlap(seq_set.red_sequence,
                                                      seq_set.cd_sequence,
                                                      seq_set.sequence_mask);

            if (!is_valid_flag)
            {
                continue;
            }

            // Generate the three suit sequences.
            // The red sequence determines the hearts and diamonds.
            // the cd sequence determines which bits are hearts, and which are diamonds.
            // Find the ones in red_sequence that align with ones in cd_sequence.
            seq_set.diamond_bits = seq_set.red_sequence & seq_set.cd_sequence;
            diamond_bit_count = count_set_bits_64(seq_set.diamond_bits,
                                                  NUMBER_OF_CARDS_IN_A_SUIT);

            if (diamond_bit_count != 13)
            {
                continue;
            }

            // Find the zeros in red_sequence that align with zeros in cd_sequence.
            inv_red_sequence = ~seq_set.red_sequence & seq_set.sequence_mask;
            inv_cd_sequence = ~seq_set.cd_sequence & seq_set.sequence_mask;

            // Calculate the spade suit bits.
            seq_set.spade_bits = inv_red_sequence & inv_cd_sequence;
            spade_bit_count = count_set_bits_64(seq_set.spade_bits,
                                                NUMBER_OF_CARDS_IN_A_SUIT);

            if (spade_bit_count != NUMBER_OF_CARDS_IN_A_SUIT)
            {
                continue;
            }

            // Find the zeros in red_sequence that align with ones in cd_sequence.
            seq_set.club_bits = inv_red_sequence & seq_set.cd_sequence;
            club_bit_count = count_set_bits_64(seq_set.club_bits,
                                               NUMBER_OF_CARDS_IN_A_SUIT);

            if (club_bit_count != NUMBER_OF_CARDS_IN_A_SUIT)
            {
                continue;
            }

            // Find the ones in red_sequence that align with zeros in cd_sequence.
            seq_set.heart_bits = seq_set.red_sequence & inv_cd_sequence;
            heart_bit_count = count_set_bits_64(seq_set.heart_bits,
                                                NUMBER_OF_CARDS_IN_A_SUIT);

            if (heart_bit_count != NUMBER_OF_CARDS_IN_A_SUIT)
            {
                continue;
            }

            // Create the hc_sequence
            seq_set.hc_sequence = seq_set.club_bits | seq_set.heart_bits;
            hc_bit_count = count_set_bits_64(seq_set.hc_sequence,
                                             NUMBER_OF_CARDS_IN_TWO_SUITS);

            if (hc_bit_count != 26)
            {
                continue;
            }

            is_valid_flag = bit_has_unique_subsequences(seq_set.hc_sequence);

            if (!is_valid_flag)
            {
                continue;
            }

            seq_set.diamond_bits = seq_set.spade_bits | seq_set.heart_bits | seq_set.club_bits;
            seq_set.diamond_bits = ~seq_set.diamond_bits & seq_set.sequence_mask;

            // Create the red, cd, and hc suit sequences.
            seq_set.red_sequence = seq_set.heart_bits | seq_set.diamond_bits;
            seq_set.cd_sequence = seq_set.club_bits | seq_set.diamond_bits;
            seq_set.hc_sequence = seq_set.club_bits | seq_set.heart_bits;

            //display_sequence(_T(" HD"), seq_set.red_sequence, seq_set.n_bits_max);
            //display_sequence(_T(" CD"), seq_set.cd_sequence, seq_set.n_bits_max);
            //display_sequence(_T(" HC"), seq_set.hc_sequence, seq_set.n_bits_max);

            _tprintf(_T("Found suit sequences.\n"));

            // Reset the odd sequence generator.
            f64_reset(dbn_odd_handle);

            // Optionally skip some odd sequences.
            if (odd_start_count > 0)
            {
                for (iodd = 0; iodd < (int)odd_start_count; ++iodd)
                {
                    seq_set.odd_sequence = f64_next(dbn_odd_handle);
                }

                // Only start deep for the first odd sequence.
                odd_start_count = 0;
            }

            // Search for an odd sequence.
            while (TRUE)
            {
                seq_set.odd_sequence = f64_next(dbn_odd_handle);

                if (seq_set.odd_sequence == 0LL)
                {
                    break;
                }

                is_valid_flag = is_valid_odd_sequence(&seq_set);

                if (!is_valid_flag)
                {
                    continue;
                }

                _tprintf(_T("Found odd sequence candidate.\n"));

                // Reset the hi sequence generator.
                f64_reset(dbn_c7k_handle);

                // Optionally skip some 7k sequences.
                if (c7k_start_count > 0)
                {
                    for (i7k = 0; i7k < (int)c7k_start_count; ++i7k)
                    {
                        seq_set.c7k_sequence = f64_next(dbn_c7k_handle);
                    }

                    // Only start deep for the first 7k sequence.
                    c7k_start_count = 0;
                }

                // Create 7k sequences.
                while (TRUE)
                {
                    // Get a hi sequence.
                    seq_set.c7k_sequence = f64_next(dbn_c7k_handle);

                    if (seq_set.c7k_sequence == 0LL)
                    {
                        break;
                    }

                    is_valid_flag = is_valid_c7k_sequence(&seq_set);

                    if (!is_valid_flag)
                    {
                        continue;
                    }

                    is_valid_flag = is_valid_deck_sequence(&seq_set);

                    if (!is_valid_flag)
                    {
                        continue;
                    }

                    if (c8k_sequence_flag)
                    {
                        is_valid_flag = search_for_c8k_sequence(&seq_set);

                        if (!is_valid_flag)
                        {
                            continue;
                        }
                    }   // if (c8k_sequence_flag)

                    if (c4t_sequence_flag)
                    {
                        is_valid_flag = search_for_c4t_sequence(&seq_set);
                        if (!is_valid_flag)
                        {
                            continue;
                        }
                    } // if (c4t_sequence_flag)

                    display_sequence(_T(" 7K"),
                                     seq_set.c7k_sequence,
                                     seq_set.n_bits_max);

                    if (c8k_sequence_flag)
                    {
                        display_sequence(_T(" 8K"),
                                         seq_set.c8k_sequence,
                                         seq_set.n_bits_max);
                    } // if (c8k_sequence_flag)

                    if (c4t_sequence_flag)
                    {
                        display_sequence(_T(" 4T"),
                                         seq_set.c4t_sequence,
                                         seq_set.n_bits_max);
                    }

                    display_sequence(_T("ODD"),
                                     seq_set.odd_sequence,
                                     seq_set.n_bits_max);

                    display_sequence(_T("RED"),
                                     seq_set.red_sequence,
                                     seq_set.n_bits_max);

                    display_sequence(_T(" CD"),
                                     seq_set.cd_sequence,
                                     seq_set.n_bits_max);

                    display_sequence(_T(" HC"),
                                     seq_set.hc_sequence,
                                     seq_set.n_bits_max);

                    _tprintf(_T("\n"));
                }   // while (TRUE)
            }   // while (TRUE)
        }   // while (TRUE)
    }   // while (not_done)

    // Free the De Bruijn sequence generators.
    f64_delete(&dbn_c7k_handle);
    f64_delete(&dbn_odd_handle);
    f64_delete(&dbn_cd_handle);
    f64_delete(&dbn_red_handle);

    return 0;
}

// Count the number of bits set in a 64-bit unsigned integer.
unsigned int count_set_bits_64(uint64_t value, int max_bit_count)
{
    int count = 0;

    while (value != 0LL)
    {
        value = value & (value - 1LL);
        count += 1;

        if (count > max_bit_count)
        {
            break;
        }
    }

    return count;
}

// Validate that the sequences overlap properly for a potential suit sequence.
int is_valid_suit_seq_overlap(uint64_t seq0,
                              uint64_t seq1,
                              uint64_t sequence_mask)
{
    uint64_t inv_seq0 = 0LL;
    uint64_t inv_seq1 = 0LL;
    uint64_t common_zero_bits = 0LL;
    uint64_t common_one_bits = 0LL;
    uint64_t common_zero_one_bits = 0LL;
    uint64_t common_one_zero_bits = 0LL;
    unsigned int common_zeros_count = 0;
    unsigned int common_ones_count = 0;
    unsigned int common_zero_one_count = 0;
    unsigned int common_one_zero_count = 0;
    BOOL is_valid_flag = FALSE;

    // Find the ones in seq0 that align with ones in seq1.
    common_one_bits = seq0 & seq1;
    common_ones_count = count_set_bits_64(common_one_bits,
                                          NUMBER_OF_CARDS_IN_A_SUIT);

    if ((common_ones_count == 13) && (seq0 != seq1))
    {
        // Find the zeros in seq0 that align with zeros in seq1.
        inv_seq0 = ~seq0 & sequence_mask;
        inv_seq1 = ~seq1 & sequence_mask;
        common_zero_bits = inv_seq0 & inv_seq1;
        common_zeros_count = count_set_bits_64(common_zero_bits,
                                               NUMBER_OF_CARDS_IN_A_SUIT);

        if (common_zeros_count == 13)
        {
            // Find the zeros in seq0 that align with ones in seq1.
            common_zero_one_bits = inv_seq0 & seq1;
            common_zero_one_count = count_set_bits_64(common_zero_one_bits,
                                                      NUMBER_OF_CARDS_IN_A_SUIT);

            if (common_zero_one_count == 13)
            {
                // Find the ones in seq0 that align with zeros in seq1.
                common_one_zero_bits = seq0 & inv_seq1;
                common_one_zero_count = count_set_bits_64(common_one_zero_bits,
                                                          NUMBER_OF_CARDS_IN_A_SUIT);

                if (common_one_zero_count == 13)
                {
                    is_valid_flag = TRUE;
                }
            }   // if (common_zero_one_count == 13)
        }   // if (common_zeros_count == 13)
    }   //  if ((common_ones_count == 13) && (seq0 != seq1))

    return is_valid_flag;
}

// Determine if the odd sequence is valid.
int is_valid_odd_sequence(ultimate_struct_t * seq_set_ptr)
{
    ASSERT(seq_set_ptr != NULL);

    uint64_t inv_odd_sequence = 0LL;
    uint64_t spade_even_bits = 0LL;
    uint64_t spade_odd_bits = 0LL;
    uint64_t heart_even_bits = 0LL;
    uint64_t heart_odd_bits = 0LL;
    uint64_t club_even_bits = 0LL;
    uint64_t club_odd_bits = 0LL;
    uint64_t diamond_even_bits = 0LL;
    uint64_t diamond_odd_bits = 0LL;
    unsigned int spade_even_count = 0;
    unsigned int spade_odd_count = 0;
    unsigned int heart_even_count = 0;
    unsigned int heart_odd_count = 0;
    unsigned int club_even_count = 0;
    unsigned int club_odd_count = 0;
    unsigned int diamond_even_count = 0;
    unsigned int diamond_odd_count = 0;
    BOOL is_valid_flag = FALSE;

    // This do loop construct ends with "while (0)", so it is not a loop.
    // This allows using "break" to exit on errors and avoids deep-nesting
    // of code while still allowing a single "return" statement without
    // using a "goto" statement.
    do
    {
        // Find the ones in seq0 that align with ones in odd_sequence.
        inv_odd_sequence = ~seq_set_ptr->odd_sequence & seq_set_ptr->sequence_mask;

        spade_even_bits = seq_set_ptr->spade_bits & inv_odd_sequence;
        spade_odd_bits = seq_set_ptr->spade_bits & seq_set_ptr->odd_sequence;
        spade_even_count = count_set_bits_64(spade_even_bits, 6);
        spade_odd_count = count_set_bits_64(spade_odd_bits, 7);

        if ((spade_even_count != 6) || (spade_odd_count != 7))
        {
            break;
        }

        // Get the odd and even counts for each suit.
        heart_even_bits = seq_set_ptr->heart_bits & inv_odd_sequence;
        heart_odd_bits = seq_set_ptr->heart_bits & seq_set_ptr->odd_sequence;
        heart_even_count = count_set_bits_64(heart_even_bits, 6);
        heart_odd_count = count_set_bits_64(heart_odd_bits, 7);

        if ((heart_even_count != 6) || (heart_odd_count != 7))
        {
            break;
        }

        club_even_bits = seq_set_ptr->club_bits & inv_odd_sequence;
        club_odd_bits = seq_set_ptr->club_bits & seq_set_ptr->odd_sequence;
        club_even_count = count_set_bits_64(club_even_bits, 6);
        club_odd_count = count_set_bits_64(club_odd_bits, 7);

        if ((club_even_count != 6) || (club_odd_count != 7))
        {
            break;
        }

        diamond_even_bits = seq_set_ptr->diamond_bits & inv_odd_sequence;
        diamond_odd_bits = seq_set_ptr->diamond_bits & seq_set_ptr->odd_sequence;
        diamond_even_count = count_set_bits_64(diamond_even_bits, 6);
        diamond_odd_count = count_set_bits_64(diamond_odd_bits, 7);

        if ((diamond_even_count != 6) || (diamond_odd_count != 7))
        {
            break;
        }

        is_valid_flag = TRUE;
    }
    while (0);

    return is_valid_flag;
}

// Function to get overlap counts for the hi-sequence.
//void get_overlap_counts(uint64_t seq0,
//                        uint64_t seq1,
//                        uint64_t sequence_mask,
//                        unsigned int * zero_zero_count_ptr,
//                        unsigned int * zero_one_count_ptr,
//                        unsigned int * one_zero_count_ptr,
//                        unsigned int * one_one_count_ptr)
void get_overlap_counts(uint64_t seq0,
                        uint64_t seq1,
                        uint64_t sequence_mask,
                        unsigned int * zero_zero_count_ptr)
{
    ASSERT(zero_zero_count_ptr != NULL);

//    uint64_t one_one_bits = 0LL;
    uint64_t zero_zero_bits = 0LL;
//    uint64_t zero_one_bits = 0LL;
//    uint64_t one_zero_bits = 0LL;
    uint64_t inv_seq0 = 0LL;
    uint64_t inv_seq1 = 0LL;
//    unsigned int one_one_count = 0;
    unsigned int zero_zero_count = 0;
//    unsigned int zero_one_count = 0;
//    unsigned int one_zero_count = 0;

    // Becaue of how this function is currently used, if the overlap counts
    // go above 12, the sequence is rejected. So, currently the bit-counting
    // is aborted if the count reached 13.  This might have to be changed if
    // this function is used differently in the future.

    //// Get the one - one count.
    //one_one_bits = seq0 & seq1;
    //one_one_count = count_set_bits_64(one_one_bits,
    //                                  NUMBER_OF_CARDS_IN_A_SUIT);

    //// invert the two sequences.
    inv_seq0 = ~seq0 & sequence_mask;
    inv_seq1 = ~seq1 & sequence_mask;

    // Get the zero - zero count.
    zero_zero_bits = inv_seq0 & inv_seq1;
    zero_zero_count = count_set_bits_64(zero_zero_bits,
                                        NUMBER_OF_CARDS_IN_A_SUIT);

    //// Get the zero - one count.
    //zero_one_bits = inv_seq0 & seq1;
    //zero_one_count = count_set_bits_64(zero_one_bits,
    //                                   NUMBER_OF_CARDS_IN_A_SUIT);

    //// Get the one - zero count.
    //one_zero_bits = seq0 & inv_seq1;
    //one_zero_count = count_set_bits_64(one_zero_bits,
    //                                   NUMBER_OF_CARDS_IN_A_SUIT);

    // Pass the calculated counts back to the caller.
    *zero_zero_count_ptr = zero_zero_count;
    //*zero_one_count_ptr = zero_one_count;
    //*one_zero_count_ptr = one_zero_count;
    //*one_one_count_ptr = one_one_count;
}

// Determine if the hi sequence is valid.
int is_valid_c7k_sequence(ultimate_struct_t * seq_set_ptr)
{
    ASSERT(seq_set_ptr != NULL);

    unsigned int xzz = 0;
    //unsigned int xzo = 0;
    //unsigned int xoz = 0;
    //unsigned int xoo = 0;
    unsigned int red_zz = 0;
    //unsigned int red_zo = 0;
    //unsigned int red_oz = 0;
    //unsigned int red_oo = 0;
    unsigned int cd_zz = 0;
    //unsigned int cd_zo = 0;
    //unsigned int cd_oz = 0;
    //unsigned int cd_oo = 0;
    unsigned int hc_zz = 0;
    //unsigned int hc_zo = 0;
    //unsigned int hc_oz = 0;
    //unsigned int hc_oo = 0;
    BOOL is_valid_flag = FALSE;

    // This do loop construct ends with "while (0)", so it is not a loop.
    // This allows using "break" to exit on errors and avoids deep-nesting
    // of code while still allowing a single "return" statement without
    // using a "goto" statement.
    do
    {
        // Get the overlap counts for the odd sequence and the c7k sequence.
        //get_overlap_counts(seq_set_ptr->odd_sequence,
        //                   seq_set_ptr->c7k_sequence,
        //                   seq_set_ptr->sequence_mask,
        //                   &xzz,
        //                   &xzo,
        //                   &xoz,
        //                   &xoo);
        get_overlap_counts(seq_set_ptr->odd_sequence,
                           seq_set_ptr->c7k_sequence,
                           seq_set_ptr->sequence_mask,
                           &xzz);

        if (xzz != 12)
        {
            break;
        }

        // Get the red sequence overlap with the c7k sequence.
        //get_overlap_counts(red_sequence,
        //                   c7k_sequence,
        //                   sequence_mask,
        //                   &red_zz,
        //                   &red_zo,
        //                   &red_oz,
        //                   &red_oo);
        get_overlap_counts(seq_set_ptr->red_sequence,
                           seq_set_ptr->c7k_sequence,
                           seq_set_ptr->sequence_mask,
                           &red_zz);

        if (red_zz != 12)
        {
            break;
        }

        // Get the cd sequence overlap with the c7k sequence.
        //get_overlap_counts(seq_set_ptr->cd_sequence,
        //                   seq_set_ptr->c7k_sequence,
        //                   seq_set_ptr->sequence_mask,
        //                   &cd_zz,
        //                   &cd_zo,
        //                   &cd_oz,
        //                   &cd_oo);
        get_overlap_counts(seq_set_ptr->cd_sequence,
                           seq_set_ptr->c7k_sequence,
                           seq_set_ptr->sequence_mask,
                           &cd_zz);

        if (cd_zz != 12)
        {
            break;
        }

        // Get the hc sequence overlap with the c7k sequence.
        //get_overlap_counts(seq_set_ptr->hc_sequence,
        //                   seq_set_ptr->c7k_sequence,
        //                   seq_set_ptr->sequence_mask,
        //                   &hc_zz,
        //                   &hc_zo,
        //                   &hc_oz,
        //                   &hc_oo);
        get_overlap_counts(seq_set_ptr->hc_sequence,
                           seq_set_ptr->c7k_sequence,
                           seq_set_ptr->sequence_mask,
                           &hc_zz);

        if (hc_zz != 12)
        {
            break;
        }

        is_valid_flag = TRUE;
    } while (0);

    return is_valid_flag;
}

// Validate that this sequence can form a valid deck arrangment.
int is_valid_deck_sequence(ultimate_struct_t * seq_set_ptr)
{
    ASSERT(seq_set_ptr != NULL);

    int hit_count_array[16];
    uint64_t red_sequence = seq_set_ptr->red_sequence;
    uint64_t cd_sequence = seq_set_ptr->cd_sequence;
    uint64_t odd_sequence = seq_set_ptr->odd_sequence;
    uint64_t c7k_sequence = seq_set_ptr->c7k_sequence;
    int i = 0;
    int index = 0;
    BOOL is_valid_flag = FALSE;

    memset(hit_count_array, 0, sizeof(hit_count_array));

    for (i = 0; i < (int)seq_set_ptr->n_bits_max; ++i)
    {
        index += red_sequence & 1;
        index = index << 1;
        index += cd_sequence & 1;
        index = index << 1;
        index += odd_sequence & 1;
        index = index << 1;
        index += c7k_sequence & 1;
        hit_count_array[index] += 1;
        index = 0;
        red_sequence = red_sequence >> 1;
        cd_sequence = cd_sequence >> 1;
        odd_sequence = odd_sequence >> 1;
        c7k_sequence = c7k_sequence >> 1;
    }

    if ((hit_count_array[0] == 3)
        && (hit_count_array[1] == 3)
        && (hit_count_array[2] == 3)
        && (hit_count_array[3] == 4)
        && (hit_count_array[4] == 3)
        && (hit_count_array[5] == 3)
        && (hit_count_array[6] == 3)
        && (hit_count_array[7] == 4)
        && (hit_count_array[8] == 3)
        && (hit_count_array[9] == 3)
        && (hit_count_array[10] == 3)
        && (hit_count_array[11] == 4)
        && (hit_count_array[12] == 3)
        && (hit_count_array[13] == 3)
        && (hit_count_array[14] == 3)
        && (hit_count_array[15] == 4))
    {
        is_valid_flag = TRUE;
    }

    return is_valid_flag;
}

// Check if the sequence can have four "odd" one bits of a sequence
// with a density of 28 changed to zeros and still be a valid sequence.
int search_for_c8k_sequence(ultimate_struct_t * seq_set_ptr)
{
    ASSERT(seq_set_ptr != NULL);

    uint64_t spade_bit_array[13];
    uint64_t heart_bit_array[13];
    uint64_t club_bit_array[13];
    uint64_t diamond_bit_array[13];
    uint64_t spade_bit = 0LL;
    uint64_t heart_bit = 0LL;
    uint64_t club_bit = 0LL;
    uint64_t diamond_bit = 0LL;
    uint64_t bit = 1LL;
    uint64_t red_bit = 1LL;
    uint64_t cd_bit = 1LL;
    uint64_t temp = 0LL;
    unsigned int spade_count = 0;
    unsigned int heart_count = 0;
    unsigned int club_count = 0;
    unsigned int diamond_count = 0;
    int s = 0;
    int c = 0;
    int h = 0;
    int d = 0;
    int i = 0;
    BOOL is_valid_flag = FALSE;

    // Separate all sequence bits by suit value.
    for (i = 0; i < (int)seq_set_ptr->n_bits_max; ++i)
    {
        // Is the c7k_sequence bit set and is this an odd card value?
        if (((seq_set_ptr->c7k_sequence & bit) != 0) && ((seq_set_ptr->odd_sequence & bit) != 0))
        {
            red_bit = seq_set_ptr->red_sequence & bit;
            cd_bit = seq_set_ptr->cd_sequence & bit;

            if (red_bit == 0)
            {
                if (cd_bit == 0)
                {
                    spade_bit_array[spade_count] = bit;
                    ++spade_count;
                }
                else
                {
                    club_bit_array[club_count] = bit;
                    ++club_count;
                }
            }
            else
            {
                if (cd_bit == 0)
                {
                    heart_bit_array[heart_count] = bit;
                    ++heart_count;
                }
                else
                {
                    diamond_bit_array[diamond_count] = bit;
                    ++diamond_count;
                }
            }
        }

        bit = bit << 1;
    } // for (i = 0; i < (int)seq_set_ptr->n_bits_max; ++i)

    // Change four bits from one to zero, one for each suit and
    // test whether the resulting sequence is valid.
    for (s = 0; s < (int)(spade_count); ++s)
    {
        spade_bit = spade_bit_array[s];

        for (h = 0; h < (int)(heart_count); ++h)
        {
            heart_bit = heart_bit_array[h];

            for (c = 0; c < (int)(club_count); ++c)
            {
                club_bit = club_bit_array[c];

                for (d = 0; d < (int)(diamond_count); ++d)
                {
                    diamond_bit = diamond_bit_array[d];
                    temp = seq_set_ptr->c7k_sequence;
                    temp -= spade_bit;
                    temp -= heart_bit;
                    temp -= club_bit;
                    temp -= diamond_bit;

                    is_valid_flag = bit_has_unique_subsequences(temp);

                    if (is_valid_flag)
                    {
                        seq_set_ptr->c8k_sequence = temp;

                        // Is it required that the bits corresponding to the
                        // card value seven are at least 5 bits apart?
                        if (seq_set_ptr->sevens_apart_flag)
                        {
                            is_valid_flag = c7k_c8k_diff_valid(seq_set_ptr);

                            if (!is_valid_flag)
                            {
                                continue;
                            }
                        }   // if (sevens_apart_flag)

                        seq_set_ptr->c7k_c8k_diff = \
                            seq_set_ptr->c7k_sequence ^ seq_set_ptr->c8k_sequence;;
                        return TRUE;
                    }
                }
            }
        }
    }   // for (s = 0; s < (int)(spade_count); ++s)

    //_tprintf(_T("No 8k sequence found.\n"));

    return FALSE;
}

// Verify the four difference bits in the 7k-8k diff sequence are valid.
int c7k_c8k_diff_valid(ultimate_struct_t * seq_set_ptr)
{
    ASSERT(seq_set_ptr != NULL);

    uint64_t c7k_c8k_diff = 0LL;
    int i = 0;
    int found_bit_count = 0;
    int first_set_bit_position = -1;
    int set_bit_position = 0;
    int last_set_bit_position = -1000;
    int diff_position = 0;

    c7k_c8k_diff = seq_set_ptr->c7k_sequence ^ seq_set_ptr->c8k_sequence;
    //display_sequence(_T("dif"), c7k_c8k_diff, n_bits_max);

    while (found_bit_count < 4)
    {
        // Find the next bit.
        while (i < (int)seq_set_ptr->n_bits_max)
        {
            // Is there a bit set in the next group?
            if ((c7k_c8k_diff & seq_set_ptr->subsequence_mask) != 0)
            {
                // Now check 1 bit at a time.
                while (i < (int)seq_set_ptr->n_bits_max)
                {
                    if ((c7k_c8k_diff & 1LL) == 1LL)
                    {
                        ++found_bit_count;

                        // Now clear this bit, and if there are any other
                        // bits set, then fail.
                        c7k_c8k_diff -= 1LL;

                        if ((c7k_c8k_diff & seq_set_ptr->subsequence_mask) != 0)
                        {
                            return FALSE;
                        }

                        c7k_c8k_diff = c7k_c8k_diff >> 1;
                        ++i;
                        break;
                    }

                    c7k_c8k_diff = c7k_c8k_diff >> 1;
                    ++i;
                }

                break;
            }   // if ((value & subsequence_mask) != 0)

            c7k_c8k_diff = c7k_c8k_diff >> seq_set_ptr->subsequence_length;
            i += seq_set_ptr->subsequence_length;
        }   // while (i < (int)n_bits_max)

        if (i >= (int)seq_set_ptr->n_bits_max)
        {
            break;
        }

        // The current set bit position is i - 1
        set_bit_position = i - 1;

        // Save the very first set bit position.
        if (first_set_bit_position == -1)
        {
            first_set_bit_position = set_bit_position;
        }

        // Is this far enough from the last bit position?
        diff_position = set_bit_position - last_set_bit_position;

        // Save the set bit position so it can be compared to the
        // next set bit position.
        last_set_bit_position = set_bit_position;

        if (diff_position < (int)seq_set_ptr->subsequence_length)
        {
            return FALSE;
        }
    }   // while (found_bit_count < 4)

    // Is the first set bit position far enough from the last set bit position?
    diff_position = (seq_set_ptr->n_bits_max + first_set_bit_position) - last_set_bit_position;

    if (diff_position < (int)seq_set_ptr->subsequence_length)
    {
        return FALSE;
    }

    return TRUE;
}

// Check if the sequence can support the 4T cards,
// which is 7 plus or minus 3.
int search_for_c4t_sequence(ultimate_struct_t * seq_set_ptr)
{
    ASSERT(seq_set_ptr != NULL);

    uint64_t odd_low_spade_bit_array[13];
    uint64_t even_low_spade_bit_array[13];
    uint64_t odd_high_spade_bit_array[13];
    uint64_t even_high_spade_bit_array[13];
    uint64_t odd_low_heart_bit_array[13];
    uint64_t even_low_heart_bit_array[13];
    uint64_t odd_high_heart_bit_array[13];
    uint64_t even_high_heart_bit_array[13];
    uint64_t odd_low_club_bit_array[13];
    uint64_t even_low_club_bit_array[13];
    uint64_t odd_high_club_bit_array[13];
    uint64_t even_high_club_bit_array[13];
    uint64_t odd_low_diamond_bit_array[13];
    uint64_t even_low_diamond_bit_array[13];
    uint64_t odd_high_diamond_bit_array[13];
    uint64_t even_high_diamond_bit_array[13];

    uint64_t els_bits0 = 0LL;
    uint64_t els_bits1 = 0LL;
    uint64_t ehs_bits = 0LL;
    uint64_t ols_bits = 0LL;
    uint64_t ohs_bits0 = 0LL;
    uint64_t ohs_bits1 = 0LL;

    uint64_t elh_bits0 = 0LL;
    uint64_t elh_bits1 = 0LL;
    uint64_t ehh_bits = 0LL;
    uint64_t olh_bits = 0LL;
    uint64_t ohh_bits0 = 0LL;
    uint64_t ohh_bits1 = 0LL;

    uint64_t elc_bits0 = 0LL;
    uint64_t elc_bits1 = 0LL;
    uint64_t ehc_bits = 0LL;
    uint64_t olc_bits = 0LL;
    uint64_t ohc_bits0 = 0LL;
    uint64_t ohc_bits1 = 0LL;

    uint64_t eld_bits0 = 0LL;
    uint64_t eld_bits1 = 0LL;
    uint64_t ehd_bits = 0LL;
    uint64_t old_bits = 0LL;
    uint64_t ohd_bits0 = 0LL;
    uint64_t ohd_bits1 = 0LL;

    uint64_t bit = 1LL;
    uint64_t temp = 0LL;
    unsigned int odd_low_spade_count = 0;
    unsigned int even_low_spade_count = 0;
    unsigned int odd_high_spade_count = 0;
    unsigned int even_high_spade_count = 0;
    unsigned int odd_low_heart_count = 0;
    unsigned int even_low_heart_count = 0;
    unsigned int odd_high_heart_count = 0;
    unsigned int even_high_heart_count = 0;
    unsigned int odd_low_club_count = 0;
    unsigned int even_low_club_count = 0;
    unsigned int odd_high_club_count = 0;
    unsigned int even_high_club_count = 0;
    unsigned int odd_low_diamond_count = 0;
    unsigned int even_low_diamond_count = 0;
    unsigned int odd_high_diamond_count = 0;
    unsigned int even_high_diamond_count = 0;
    unsigned int red = 0;
    unsigned int cd = 0;
    unsigned int suit = 0;

    int ols = 0;
    int els = 0;
    int ohs = 0;
    int ehs = 0;
    int olh = 0;
    int elh = 0;
    int ohh = 0;
    int ehh = 0;
    int olc = 0;
    int elc = 0;
    int ohc = 0;
    int ehc = 0;
    int old = 0;
    int eld = 0;
    int ohd = 0;
    int ehd = 0;

    int i = 0;
    BOOL is_valid_flag = FALSE;

    // Sort all sequence bits by suit value.
    for (i = 0; i < (int)seq_set_ptr->n_bits_max; ++i)
    {
        // Do not change the bits that make the 7k and 8k sequence work.
        // These four bits are for the four sevens.
        if ((seq_set_ptr->c7k_c8k_diff & bit) == 0)
        {
            red = ((seq_set_ptr->red_sequence & bit) == 0) ? 0 : 1;
            cd = ((seq_set_ptr->cd_sequence & bit) == 0) ? 0 : 1;
            suit = (red << 1) | cd;

            // Is this a 7k (high) card?
            if ((seq_set_ptr->c7k_sequence & bit) != 0)
            {
                // This is a high card.
                // Is this an odd card?
                if ((seq_set_ptr->odd_sequence & bit) != 0)
                {
                    // This is an odd high card.
                    switch (suit)
                    {
                    case 0:
                        odd_high_spade_bit_array[odd_high_spade_count] = bit;
                        ++odd_high_spade_count;
                        break;
                    case 1:
                        odd_high_club_bit_array[odd_high_club_count] = bit;
                        ++odd_high_club_count;
                        break;
                    case 2:
                        odd_high_heart_bit_array[odd_high_heart_count] = bit;
                        ++odd_high_heart_count;
                        break;
                    case 3:
                        odd_high_diamond_bit_array[odd_high_diamond_count] = bit;
                        ++odd_high_diamond_count;
                        break;
                    default:
                        _tprintf(_T("Error - invalid suit.\n"));
                        break;
                    }   // switch (suit)
                }
                else
                {
                    // This is an even high card.
                    switch (suit)
                    {
                    case 0:
                        even_high_spade_bit_array[even_high_spade_count] = bit;
                        ++even_high_spade_count;
                        break;
                    case 1:
                        even_high_club_bit_array[even_high_club_count] = bit;
                        ++even_high_club_count;
                        break;
                    case 2:
                        even_high_heart_bit_array[even_high_heart_count] = bit;
                        ++even_high_heart_count;
                        break;
                    case 3:
                        even_high_diamond_bit_array[even_high_diamond_count] = bit;
                        ++even_high_diamond_count;
                        break;
                    default:
                        _tprintf(_T("Error - invalid suit.\n"));
                        break;
                    }   // switch (suit)
                }   // if ((odd_sequence & bit) != 0)
            }
            else
            {
                // This is a low card.
                // Is this an odd card?
                if ((seq_set_ptr->odd_sequence & bit) != 0)
                {
                    // This is an odd low card.
                    switch (suit)
                    {
                    case 0:
                        odd_low_spade_bit_array[odd_low_spade_count] = bit;
                        ++odd_low_spade_count;
                        break;
                    case 1:
                        odd_low_club_bit_array[odd_low_club_count] = bit;
                        ++odd_low_club_count;
                        break;
                    case 2:
                        odd_low_heart_bit_array[odd_low_heart_count] = bit;
                        ++odd_low_heart_count;
                        break;
                    case 3:
                        odd_low_diamond_bit_array[odd_low_diamond_count] = bit;
                        ++odd_low_diamond_count;
                        break;
                    default:
                        _tprintf(_T("Error - invalid suit.\n"));
                        break;
                    }   // switch (suit)
                }
                else
                {
                    // This is an even low card.
                    switch (suit)
                    {
                    case 0:
                        even_low_spade_bit_array[even_low_spade_count] = bit;
                        ++even_low_spade_count;
                        break;
                    case 1:
                        even_low_club_bit_array[even_low_club_count] = bit;
                        ++even_low_club_count;
                        break;
                    case 2:
                        even_low_heart_bit_array[even_low_heart_count] = bit;
                        ++even_low_heart_count;
                        break;
                    case 3:
                        even_low_diamond_bit_array[even_low_diamond_count] = bit;
                        ++even_low_diamond_count;
                        break;
                    default:
                        _tprintf(_T("Error - invalid suit.\n"));
                        break;
                    }   // switch (suit)
                }   // if ((seq_set_ptr->odd_sequence & bit) != 0)
            }   // if ((seq_set_ptr->c7k_sequence & bit) != 0)
        }   // if ((bit && seq_set_ptr->c7k_c8k_diff) == 0)

        bit = bit << 1;
    }   // for (i = 0; i < (int)n_bits_max; ++i)

    // Generate the sequence.
    for (els = 0; els < (int)even_low_spade_count; ++els)
    {
        for (ehs = 0; ehs < (int)even_high_spade_count; ++ehs)
        {
            for (ols = 0; ols < (int)odd_low_spade_count; ++ols)
            {
                for (ohs = 0; ohs < (int)odd_high_spade_count; ++ohs)
                {
                    for (elh = 0; elh < (int)even_low_heart_count; ++elh)
                    {
                        for (ehh = 0; ehh < (int)even_high_heart_count; ++ehh)
                        {
                            for (olh = 0; olh < (int)odd_low_heart_count; ++olh)
                            {
                                for (ohh = 0; ohh < (int)odd_high_heart_count; ++ohh)
                                {
                                    for (elc = 0; elc < (int)even_low_club_count; ++elc)
                                    {
                                        for (ehc = 0; ehc < (int)even_high_club_count; ++ehc)
                                        {
                                            for (olc = 0; olc < (int)odd_low_club_count; ++olc)
                                            {
                                                for (ohc = 0; ohc < (int)odd_high_club_count; ++ohc)
                                                {
                                                    for (eld = 0; eld < (int)even_low_diamond_count; ++eld)
                                                    {
                                                        for (ehd = 0; ehd < (int)even_high_diamond_count; ++ehd)
                                                        {
                                                            for (old = 0; old < (int)odd_low_diamond_count; ++old)
                                                            {
                                                                for (ohd = 0; ohd < (int)odd_high_diamond_count; ++ohd)
                                                                {
                                                                    // A 2 3 4 5 6 7 8 9 T J Q K
                                                                    //             7 8 9 T J Q K 
                                                                    //       4 5 6 7 8 9 T
                                                                    //
                                                                    // For each suit :
                                                                    //
                                                                    //     2 odd high cards must become 2 even low cards
                                                                    //
                                                                    //     1 even high card must become 1 odd low card
                                                                    //

                                                                    // Spades
                                                                    els_bits0 = even_low_spade_bit_array[els];
                                                                    els_bits1 = even_low_spade_bit_array[(els + 1) % even_low_spade_count];
                                                                    ehs_bits = even_high_spade_bit_array[ehs];
                                                                    ols_bits = odd_low_spade_bit_array[ols];
                                                                    ohs_bits0 = odd_high_spade_bit_array[ohs];
                                                                    ohs_bits1 = odd_high_spade_bit_array[(ohs + 1) % odd_high_spade_count];

                                                                    // Hearts
                                                                    elh_bits0 = even_low_heart_bit_array[elh];
                                                                    elh_bits1 = even_low_heart_bit_array[(elh + 1) % even_low_heart_count];
                                                                    ehh_bits = even_high_heart_bit_array[ehh];
                                                                    olh_bits = odd_low_heart_bit_array[olh];
                                                                    ohh_bits0 = odd_high_heart_bit_array[ohh];
                                                                    ohh_bits1 = odd_high_heart_bit_array[(ohh + 1) % odd_high_heart_count];

                                                                    // Clubs
                                                                    elc_bits0 = even_low_club_bit_array[elc];
                                                                    elc_bits1 = even_low_club_bit_array[(elc + 1) % even_low_club_count];
                                                                    ehc_bits = even_high_club_bit_array[ehc];
                                                                    olc_bits = odd_low_club_bit_array[olc];
                                                                    ohc_bits0 = odd_high_club_bit_array[ohc];
                                                                    ohc_bits1 = odd_high_club_bit_array[(ohc + 1) % odd_high_club_count];

                                                                    // Diamonds
                                                                    eld_bits0 = even_low_diamond_bit_array[eld];
                                                                    eld_bits1 = even_low_diamond_bit_array[(eld + 1) % even_low_diamond_count];
                                                                    ehd_bits = even_high_diamond_bit_array[ehd];
                                                                    old_bits = odd_low_diamond_bit_array[old];
                                                                    ohd_bits0 = odd_high_diamond_bit_array[ohd];
                                                                    ohd_bits1 = odd_high_diamond_bit_array[(ohd + 1) % odd_high_diamond_count];

                                                                    temp = seq_set_ptr->c7k_sequence;

                                                                    // Spades
                                                                    // Clear 2 odd high bits.
                                                                    temp -= ohs_bits0;
                                                                    temp -= ohs_bits1;
                                                                    // Set 2 even low bits.
                                                                    temp |= els_bits0;
                                                                    temp |= els_bits1;
                                                                    // Clear 1 even high bit.
                                                                    temp -= ehs_bits;
                                                                    // Set one odd low bit.
                                                                    temp |= ols_bits;

                                                                    // Hearts
                                                                    // Clear 2 odd high bits.
                                                                    temp -= ohh_bits0;
                                                                    temp -= ohh_bits1;
                                                                    // Set 2 even low bits.
                                                                    temp |= elh_bits0;
                                                                    temp |= elh_bits1;
                                                                    // Clear 1 even high bit.
                                                                    temp -= ehh_bits;
                                                                    // Set one odd low bit.
                                                                    temp |= olh_bits;

                                                                    // Clubs
                                                                    // Clear 2 odd high bits.
                                                                    temp -= ohc_bits0;
                                                                    temp -= ohc_bits1;
                                                                    // Set 2 even low bits.
                                                                    temp |= elc_bits0;
                                                                    temp |= elc_bits1;
                                                                    // Clear 1 even high bit.
                                                                    temp -= ehc_bits;
                                                                    // Set one odd low bit.
                                                                    temp |= olc_bits;

                                                                    // Diamonds
                                                                    // Clear 2 odd high bits.
                                                                    temp -= ohd_bits0;
                                                                    temp -= ohd_bits1;
                                                                    // Set 2 even low bits.
                                                                    temp |= eld_bits0;
                                                                    temp |= eld_bits1;
                                                                    // Clear 1 even high bit.
                                                                    temp -= ehd_bits;
                                                                    // Set one odd low bit.
                                                                    temp |= old_bits;

                                                                    is_valid_flag = bit_has_unique_subsequences(temp);

                                                                    if (is_valid_flag)
                                                                    {
                                                                        seq_set_ptr->c4t_sequence = temp;
                                                                        return TRUE;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //_tprintf(_T("No 4t sequence found.\n"));

    return FALSE;
}

// Check if the sequence has either six zeros or six ones.
int has_long_uniform_subsequence(uint64_t seq)
{
    int has_luss = FALSE;

    uint64_t wseq = seq | (seq << 52);

    for (int i = 0; i < 52; ++i)
    {
        if (((wseq & 0x3FL) == 0x3FL)
          || (((~wseq) & 0x3FL) == 0x3FL))
        {
            has_luss = TRUE;
            break;
        }

        wseq >>= 1;
    }

    return has_luss;
}

/* Function: display_usage */
void display_usage()
{
    _tprintf(_T("\n"));
    _tprintf(_T("Program ultimate_search - version 1.1 - This program finds card sequences\n"));
    _tprintf(_T("Copyright (c) 2018 - William Hallahan - All Rights Reserved.\n\n"));
    _tprintf(_T("This program calculates De Bruijn sequences.\n\n"));
    _tprintf(_T("Usage:\n\n"));
    _tprintf(_T("    ultimate_search [options]\n\n"));
    _tprintf(_T("\nOptional arguments:\n\n"));
    _tprintf(_T("-r <hd-start-count | --red <hd-start-count>\n"));
    _tprintf(_T("                  Skip the specified number of red sequences.\n"));
    _tprintf(_T("-c <cd-start-count | --cd <cd-start-count>\n"));
    _tprintf(_T("                  Skip the specified number of cd sequences.\n"));
    _tprintf(_T("-o <odd-start-count | --odd <odd-start-count>\n"));
    _tprintf(_T("                  Skip the specified number of odd sequences.\n"));
    _tprintf(_T("-k <c7k-start-count | --c7k <c7k-start-count>\n"));
    _tprintf(_T("                  Skip the specified number of 7k sequences.\n"));
    _tprintf(_T("-n | --valshort   If specified, then do not allow value subsequences\n"));
    _tprintf(_T("                  that are all zeros or are all ones.\n"));
    _tprintf(_T("-s | --suitshort  If specified, then do not allow suit subsequences\n"));
    _tprintf(_T("                  that are all zeros or are all ones.\n"));
    _tprintf(_T("-e | --c8k        Search for a sequence for cards with values that range\n"));
    _tprintf(_T("                  from 8 to K.\n"));
    _tprintf(_T("-p | --sep        Ensure the 7 card values are more than 5 cards apart.\n"));
    _tprintf(_T("                  This flag only works when the --c8k option is specified\n"));
    _tprintf(_T("-t | --c4t        Search for a sequence for cards with values that range\n"));
    _tprintf(_T("                  from 4 to 10.\n"));
    _tprintf(_T("-h | --help       Display program help and exit.\n\n"));
}
