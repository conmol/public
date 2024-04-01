/* ********************************************************************
 * File: arg_get_arguments.c
 * Author: Bill Hallahan
 * Date: September 26, 2013
 *
 * Copyright (C) 2013-2024 William Hallahan
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************* */

#include "platform_os.h"
#include "arg_get_arguments.h"

/* Function to ensure all characters in a string are unique. */
static BOOL has_unique_characters(TCHAR* text_ptr);

/* Find if short option character that matches passed character. */
static int search_for_short_option_char(const arg_option_t * arg_option_ptr,
                                        TCHAR option_char,
                                        int * found_option_int_ptr,
                                        int * option_param_type_ptr);

/* Find the short option character for the specified long option string. */
static int search_for_long_option_string(const arg_option_t * arg_option_ptr,
                                         TCHAR * argument_ptr,
                                         int * found_option_int_ptr,
                                         int * option_param_type_ptr);

/* Function to parse a single argument on the command line. */
int arg_get_arguments(int argc,
                      TCHAR * argv[],
                      const arg_option_t * argument_option_array_ptr,
                      int * arg_index_ptr,
                      int * option_int_ptr)
{
    static TCHAR * next_short_option_ptr = NULL;
    TCHAR * argument_ptr = NULL;
    const arg_option_t * arg_option_ptr = argument_option_array_ptr;
    int arg_length = 0;
    int option_param_type = OPTION_NO_PARAMETER;
    int status = ARG_NO_MORE_ARGUMENTS;

    ASSERT(argv != NULL);
    ASSERT(argument_option_array_ptr != NULL);
    ASSERT(arg_index_ptr != NULL);
    ASSERT(option_int_ptr != NULL);

    /* This do statement ends with "while (FALSE)", so it is not a loop.
     * This allows using "break" to exit on errors and avoids deep-nesting
     * of code while still allowing a single "return" statement without
     * using a "goto" statement. */
    do
    {
        // To reset next_short_option_ptr. This is needed if more than one
        // command line will be processed.
        if (argv == NULL)
        {
            next_short_option_ptr = NULL;
            break;
        }

        /* Is there an unprocessed short option that is part of
         * multiple short options? */
        if ((next_short_option_ptr != NULL) && (*next_short_option_ptr != _T('\0')))
        {
            /* This is a continued short option string. Process the next
             * short option character. */
             argument_ptr = next_short_option_ptr;
             status = search_for_short_option_char(arg_option_ptr,
                                                   *argument_ptr,
                                                   option_int_ptr,
                                                   &option_param_type);

             /* A continued short option cannot require a parameter. */
             if (option_param_type == OPTION_REQUIRES_PARAMETER)
             {
                 status = ARG_ERROR_PARAM_WITH_MULTIPLE_SHORT_OPTIONS;
             }

             /* Point to either the next short option character or
              * the null terminator. */
             next_short_option_ptr = &argument_ptr[1];
             break;
        }

        /* This is not a continued multiple short option character.
         * Increment the index to point to the next argument.
         * On the first call to this function, this increment
         * skips the program name. */
        ++(*arg_index_ptr);

        /* Are there more arguments? */
        if (*arg_index_ptr >= argc)
        {
            /* There are no more arguments. Variable status already
             * equals ARG_NO_MORE_ARGUMENTS */
            break;
        }

        /* Get the argument at the current index and the
         * argument length. */
        argument_ptr = argv[*arg_index_ptr];
        arg_length = (int)_tcslen(argument_ptr);

        /* A dash character starts either an optional argument
         * or a number. */
        if (argument_ptr[0] != _T('-'))
        {
            /* No starting dash character. */
            status = ARG_POSITIONAL_ARGUMENT;
            break;
        }

        /* The argument starts with a dash character.
         * A single dash character is not a valid argument. Optional
         * arguments always have at least one non-digit character after
         * the dash character. */
        if (arg_length == 1)
        {
            status = ARG_ERROR_MISSING_SWITCH_ARGUMENT;
            break;
        }

        /* A dash followed by a digit is a negative number, for
         * example, "-2". */
        if (_istdigit(argument_ptr[1]))
        {
            /* Code to parse numbers could be added here.
             * Just consider this a positional argument and
             * let the caller validate this argument. */
            status = ARG_POSITIONAL_ARGUMENT;
            break;
        }

        /* A dash followed by a period can also be a number. */
        if (argument_ptr[1] == _T('.'))
        {
            /* There must be at least one other character to
             * form a number. Let the caller validate this
             * argument. */
            if (arg_length == 2)
            {
                status = ARG_ERROR_MISSING_SWITCH_ARGUMENT;
                break;
            }

            /* Code to parse numbers could be added here.
             * Just consider this a positional argument and
             * let the caller validate this argument. */
            status = ARG_POSITIONAL_ARGUMENT;
            break;
        }

        /* This is an optional argument.  Long optional arguments
         * start with two dash characters followed by at least one
         * character.  Check for a second dash character. */
        if ((arg_length > 1) && (argument_ptr[1] == _T('-')))
        {
            /* Only two dash characters is not valid. */
            if (arg_length == 2)
            {
                status = ARG_ERROR_MISSING_SWITCH_ARGUMENT;
                break;
            }

            /* Skip the first and second dash characters. */
            argument_ptr += 2 * sizeof(*argument_ptr);

            status = search_for_long_option_string(arg_option_ptr,
                                                   argument_ptr,
                                                   option_int_ptr,
                                                   &option_param_type);
        }
        else
        {
            /* This is a short optional argument of the form "-n".
             * Skip the dash character. */
            ++argument_ptr;

            status = search_for_short_option_char(arg_option_ptr,
                                                  argument_ptr[0],
                                                  option_int_ptr,
                                                  &option_param_type);

            if (status < ARG_NO_MORE_ARGUMENTS)
            {
                break;
            }

            /* Point to either the next short option character or
             * the null terminator. */
            next_short_option_ptr = &argument_ptr[1];

            /* If there are more short options, make sure there are no
             * duplicate short option characters. */
            if ((*argument_ptr != _T('\0')) && !has_unique_characters(argument_ptr))
            {
                status = ARG_ERROR_DUPLICATE_SHORT_OPTIONS;
            }
        }

        /* Was an option switch found? */
        if (status > ARG_NO_MORE_ARGUMENTS)
            /* An option switch was found.  Does the option require an argument? */
        {
            if (option_param_type == OPTION_REQUIRES_PARAMETER)
            {
                /* Increment the index to point to the switch argument. */
                ++(*arg_index_ptr);

                if (*arg_index_ptr >= argc)
                {
                    /* The option parameter is missing.  Return an error. */
                    --(*arg_index_ptr);
                    status = ARG_ERROR_MISSING_SWITCH_ARGUMENT;
                }
            }
        }
    } while (FALSE);

    return status;
}

/* Function to ensure all characters in a string are unique. */
static BOOL has_unique_characters(TCHAR * text_ptr) 
{
    ASSERT(text_ptr != NULL);

    int length = 0;

    length = (int)_tcslen(text_ptr);

    /* Check for duplicate characters in the string. */
    for (int i = 0; i < length; i++)
    { 
        for (int j = i + 1; j < length; j++)
        { 
            if (text_ptr[i] == text_ptr[j])
            {
                /* A duplicate character was found. */
                return FALSE; 
            } 
        } 
    } 
  
    /* No duplicate characters were found.. */
    return TRUE; 
}

/* Find if short option character that matches passed character. */
int search_for_short_option_char(const arg_option_t * arg_option_ptr,
                                 TCHAR option_char,
                                 int * found_option_int_ptr,
                                 int * option_param_type_ptr)
{
    ASSERT(arg_option_ptr != NULL);
    ASSERT(found_option_int_ptr != NULL);
    ASSERT(option_param_type_ptr != NULL);

    int status = ARG_ERROR_INVALID_SWITCH;

    /* This is a short optional argument of the form "-n".
     * Search for the argument character in the argument options array */
    while (arg_option_ptr->c != _T('\0'))
    {
        if (option_char == arg_option_ptr->c)
        {
            status = ARG_OPTIONAL_ARGUMENT;
            *found_option_int_ptr = arg_option_ptr->c;
            *option_param_type_ptr = arg_option_ptr->requires_param;
            break;
        }

        /* Point to the next arg_option_t instance in the array. */
        ++arg_option_ptr;
    }

    return status;
}

/* Find the short option character for the specified long option string. */
int search_for_long_option_string(const arg_option_t * arg_option_ptr,
                                  TCHAR * argument_ptr,
                                  int * found_option_int_ptr,
                                  int * option_param_type_ptr)
{
    ASSERT(arg_option_ptr != NULL);
    ASSERT(argument_ptr != NULL);
    ASSERT(found_option_int_ptr != NULL);
    ASSERT(option_param_type_ptr != NULL);

    int status = ARG_ERROR_INVALID_SWITCH;

    /* Search for the argument long name in the argument options array */
    while (arg_option_ptr->c != _T('\0'))
    {
        if ((arg_option_ptr->long_name != NULL)
            && (_tcscmp(arg_option_ptr->long_name, argument_ptr) == 0))
        {
            status = ARG_OPTIONAL_ARGUMENT;
            *found_option_int_ptr = arg_option_ptr->c;
            *option_param_type_ptr = arg_option_ptr->requires_param;
            break;
        }
        /* Point to the next arg_option_t instance in the array. */
        ++arg_option_ptr;
    }

    return status;
}

/* Report the status returned by the get_argument_function. */
void arg_report_status(int status, TCHAR * argv_index_ptr)
{
    ASSERT(argv_index_ptr != NULL);

    /* Report any command line argument errors. */
    switch (status)
    {
    case ARG_NO_MORE_ARGUMENTS:
    case ARG_EXIT_PROGRAM:
        // No error occured.
        break;

    case ARG_ERROR_DUPLICATE_SHORT_OPTIONS:
        _tprintf(_T("Error: duplicate characters in short option string, %s.\n"), argv_index_ptr);
        break;

    case ARG_ERROR_PARAM_WITH_MULTIPLE_SHORT_OPTIONS:
        _tprintf(_T("Error: only letters for boolean short options may be mixed, %s.\n"), argv_index_ptr);
        break;

    case ARG_ERROR_MISSING_SWITCH_ARGUMENT:
        _tprintf(_T("Error: missing switch argument for switch %s.\n"), argv_index_ptr);
        break;

    case ARG_ERROR_INVALID_SWITCH:
        _tprintf(_T("Error: invalid switch argument %s.\n"), argv_index_ptr);
        break;

    default:
    {
        /* It should be impossible to reach here. */
        _tprintf(_T("Error: invalid internal status.  Contact support.\n"));
        status = -1;
    }
    break;
    } // switch (status)
}
