//======================================================================
// File: dbn_file_creator.c
// Author: Bill Hallahan
// Date: Aug 05, 2018
// 
// Abstract:
//
//   This creates files that contains de Bruijn sequences.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef __linux__
#include <unistd.h>
#endif

#include "../common/platform_os.h"
#include "../common/arg_get_arguments.h"
#include "../common/platform_utilities.h"
#include "dbn_de_bruijn.h"

#define REQUIRED_POSITIONAL_ARG_COUNT (2)
#define DBN_MAX_FILE_NAME_LENGTH (256)

/* Structure type used for program arguments. */
typedef struct
{
    int bit_count;
    int one_count;
    BOOL not_all_bits_the_same;
} program_arguments_t;

/* Forward declarations. */
static int dbn_file_creator_main(program_arguments_t * program_args_ptr);

static void display_usage();

/* Start of main program. */
int _tmain(int argc, TCHAR* argv[])
{
    program_arguments_t program_args;
    TCHAR * stop_ptr = NULL;
    int arg_index = 0;
    int positional_arg_index = 0;
    int option_int = 0;
    int status = 0;

    static const arg_option_t argument_option_array[] =
    {
        { _T('s'), _T("short"), OPTION_NO_PARAMETER, },
        { _T('h'), _T("help"), OPTION_NO_PARAMETER },
        { 0, NULL, OPTION_NO_PARAMETER }
    };

    /* Initialize the program argument structure. */
    program_args.bit_count = 0;
    program_args.one_count = 0;
    program_args.not_all_bits_the_same = FALSE;

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
                program_args.not_all_bits_the_same = TRUE;
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
                _tprintf(_T("Error: internal parser error. Contact support.\n"));
                status = ARG_EXIT_PROGRAM;
            }
            break;
            }
        }
        else
        {
            /* Process a positional argument. */
            switch (positional_arg_index)
            {
            case 0:
            {
                program_args.bit_count = _tcstol(argv[arg_index], &stop_ptr, 10);
                if (_tcslen(stop_ptr) > 0)
                {
                    _tprintf(_T("Error: integer argument %s is invalid.\n"),
                             argv[arg_index]);
                    status = ARG_EXIT_PROGRAM;
                }
            }
            break;

            case 1:
            {
                program_args.one_count = _tcstol(argv[arg_index], &stop_ptr, 10);
                if (_tcslen(stop_ptr) > 0)
                {
                    _tprintf(_T("Error: integer argument %s is invalid.\n"),
                             argv[arg_index]);
                    status = ARG_EXIT_PROGRAM;
                }
            }
            break;

            default:
            {
                _tprintf(_T("Error: too many positional arguments starting at %s.\n"),
                         argv[arg_index]);
                status = ARG_EXIT_PROGRAM;
            }
            break;
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
            status = dbn_file_creator_main(&program_args);
        }
        else
        {
            _tprintf(_T("Error: too few arguments. For help\n    %s -h\n"), argv[0]);
            status = -1;
        }
    }

    return status;
}

/* Function: dbn_file_creator_main */
int dbn_file_creator_main(program_arguments_t * program_args_ptr)
{
    TCHAR dbn_file_name_array[DBN_MAX_FILE_NAME_LENGTH];
    TCHAR * temp_ptr = _T("");
    FILE * file_ptr = NULL;
    dbn_de_bruijn_handle_t dbn_handle;
    uint64_t seq_value = 0L;
    int sequence_modifier = DBN_ALLOW_ALL_BITS_THE_SAME;
    int status = 0;

    // This do loop construct ends with "while (0)", so it is not a loop.
    // This allows using "break" to exit on errors and avoids deep-nesting
    // of code while still allowing a single "return" statement without
    // using a "goto" statement.
    do
    {
        /* Display the input parameters. */
        /*
        _tprintf(_T("int bit_count = %d\n"), program_args_ptr->bit_count);
        _tprintf(_T("int one_count = %d\n"), program_args_ptr->one_count);
        _tprintf(_T("BOOL not_all_bits_the_same = %d\n"), program_args_ptr->not_all_bits_the_same);
        */

        temp_ptr = _T(".bin");

        if (program_args_ptr->not_all_bits_the_same)
        {
            temp_ptr = _T("_short.bin");
            sequence_modifier = DBN_REQUIRE_BITS_NOT_ALL_THE_SAME;
        }

        /* Create the file name based on the input parameters.
           Since the _stprintf_s and sprintf functions both use
           varadic arguments, it's simpler to make an exception
           and not put this in the ../common/platform_utilities file.
        */
#ifdef __linux__
        sprintf(&dbn_file_name_array[0],
                "dbn_%u_%u%s",
                program_args_ptr->bit_count,
                program_args_ptr->one_count,
                temp_ptr);
#else
        _stprintf_s(&dbn_file_name_array[0],
                    DBN_MAX_FILE_NAME_LENGTH,
                    _T("dbn_%u_%u%s"),
                    program_args_ptr->bit_count,
                    program_args_ptr->one_count,
                    temp_ptr);
#endif

        // Create a generator for the De Bruijn sequence.
        status = dbn_make(&dbn_handle,
                          program_args_ptr->bit_count,
                          program_args_ptr->one_count,
                          sequence_modifier);

        /* Create the file */
        if (_tfopen_s(&file_ptr, &dbn_file_name_array[0], _T("wb")) != 0)
        {
            status = -1;
            break;
        }

        seq_value = -1L;

        while (seq_value != 0L)
        {
            seq_value = dbn_next(dbn_handle);
            fwrite(&seq_value, sizeof(seq_value), 1, file_ptr);
        }
    } while (0);

    /* Free resources. */
    if (file_ptr != NULL)
    {
        fclose(file_ptr);
    }

    // Free the De Bruijn sequence generator.
    dbn_delete(&dbn_handle);

    return status;
}

/* Function: display_usage */
void display_usage()
{
    _tprintf(_T("\n"));
    _tprintf(_T("Program dbn_file_creator\n"));
    _tprintf(_T("Version: 1.1\n"));
    _tprintf(_T("Copyright (c) 2019-2024, William Hallahan.\n\n"));
    _tprintf(_T("This program generates a binary file that stores de Bruijn sequences.\n\n"));
    _tprintf(_T("Usage:\n\n"));
    _tprintf(_T("    dbn_file_creator [-s | --short] <bit_count> <one_count>\n"));
    _tprintf(_T("\nOptional arguments:\n"));
    _tprintf(_T("-s | --short  If specified, then no subsequences will be all zeros or all ones:\n"));
    _tprintf(_T("-h | --help   Display program help.\n\n"));
    _tprintf(_T("Positional arguments:\n"));
    _tprintf(_T("<bit-count>  The number of bits in the sequence.\n"));
    _tprintf(_T("             This in the range of 1 to 64.\n"));
    _tprintf(_T("<one-count>  The number of one bits set.\n"));
    _tprintf(_T("             If this is too far from half the bit-count\n"));
    _tprintf(_T("             then no sequences will be generated.\n"));
}
