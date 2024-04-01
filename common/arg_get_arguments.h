/* ********************************************************************
 * Header File: arg_get_arguments.h
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

#ifndef ARG_GET_ARGUMENTS_H
#define ARG_GET_ARGUMENTS_H

#include "platform_os.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* The get_arguments function either returns an option
 * character or one of these values. */
typedef enum
{
    ARG_ERROR_DUPLICATE_SHORT_OPTIONS = -4,
    ARG_ERROR_PARAM_WITH_MULTIPLE_SHORT_OPTIONS = -3,
    ARG_ERROR_INVALID_SWITCH = -2,
    ARG_ERROR_MISSING_SWITCH_ARGUMENT = -1,
    ARG_NO_MORE_ARGUMENTS = 0,
    ARG_OPTIONAL_ARGUMENT = 1,
    ARG_POSITIONAL_ARGUMENT = 2,
    ARG_EXIT_PROGRAM = 3
} arg_option_tChar_t;

/* Used to set whether an option requires a parameter. */
typedef enum
{
    OPTION_NO_PARAMETER,
    OPTION_REQUIRES_PARAMETER
} arg_option_tParameter_t;

/* The option structure. 
 * Element c stores the short option character, i.e. "b" for "-b".
 *   name is the long option name string, "build" for "--build".
 * requires_params is set to one of the arg_option_parameter_t
 *   enum values to indicate whether the option requires a parameter. */
typedef struct
{
    int c;
    TCHAR * long_name;
    arg_option_tParameter_t requires_param;
} arg_option_t;

/* ====================================================================
 * Function: get_arguments
 *
 * Abstract:
 *
 *  The get_arg_info function was written to provide argument
 *  parsing code for multiple platforms.  Also, the code can
 *  be used under the terms of the MIT license.
 *
 *  The get_arg_info function allows both short and long
 *  optional switches, and will work with the ASCII, UTF-8, and
 *  Unicode character sets.
 *
 *  Both single character and long optional argument switch
 *  named are allowed.
 *
 *  Optional arguments and positional arguments can be interspersed.
 *
 *  The benefit of allowing interspersed optional and positional
 *  arguments comes at the cost of requiring that an optional argument
 *  either does, or does not, take an additional parameter, but not both.
 *
 *  Also, optional argument names cannot start with a digit
 *  character.  This simplified parsing as a dash followed by
 *  a digit character could also be the start of a numeric
 *  positional argument.
 *
 *  If there are multiple short options, i.e. "-abc," then the
 *  argument parser maintains state in the variable named
 *  next_short_option_ptr, which is initialized to NULL. To
 *  process more than a single command line, the get_arguments
 *  function can be called with the second argument, i.e. argv
 *  set to NULL to reset next_short_option_ptr to the value NULL.
 *  Because of this internal static variable, this function
 *  is not thread-safe.
 *
 *  Input:
 *
 *    The first two arguments are passed into the 'main' function of
 *    a C or C++ console program.
 *
 *    argc                      - The number of arguments.
 *
 *    argv                      - An array of pointers to argument strings.
 *
 *    argument_option_array_ptr - A pointer to an array of arg_option_t
 *                                structures that specifies that attributes
 *                                of each optional argument.
 *
 *                                An example declaration for the array might be:
 *
 *                                static const arg_option_t argument_option_array[] =
 *                                {
 *                                    { _T('x'), NULL, OPTION_NO_PARAMETER, },
 *                                    { _T('f'), NULL, OPTION_REQUIRES_PARAMETER, },
 *                                    { _T('b'), "build", OPTION_REQUIRES_PARAMETER, },
 *                                    { _T('h'), "help", OPTION_NO_PARAMETER, },
 *                                    { _T('\0'), NULL, OPTION_NO_PARAMETER }
 *                                 };
 *
 *                                This would allow optional arguments of the form:
 *
 *                                -x
 *                                -f <somestring>
 *                                -b <somestring>
 *                                --build <somestring>
 *                                -h
 *                                --help
 *
 *                                A string must be supplied after the -s parameter
 *                                because OPTION_REQUIRES_PARAMETER is used.
 *                                
 *                                The final line must always be supplied as written
 *                                to terminate the data.
 *
 *                                Either a short argument name, a long
 *                                argument name, or both must be specified.
 *                                If only a long name is specified, the first
 *                                field of the arg_option_t structure must be
 *                                set to an out-of-band integer value, which
 *                                for either ASCII or Unicode character sets
 *                                can be any value above 0x011FFFF.
 *                                 
 *
 *    arg_index_ptr               A pointer to an 'int' value that is used
 *                                to index into the argv[] array. The value
 *                                pointed to by arg_index_ptr specifies either
 *                                the index of a positional argument or the
 *                                index of the required parameter for an
 *                                optional argument that has the
 *                                OPTION_REQUIRES_PARAMETER attribute.
 *
 *    option_int_ptr              A pointer to value of type int. If the
 *                                returned status value is the value
 *                                ARG_OPTIONAL_ARGUMENT then the integer
 *                                is set to a character value, or to an an
 *                                integer value for a long option that has
 *                                no corresponding short option.
 *
 *  Returns:
 *
 *    An integer value that is either the option character for an
 *    optional argument, which is the first field in OptionsArg_t
 *    structure that matches the optional argument passed on the
 *    command line, or, one of the following values.
 *
 *    ARG_POSITIONAL_ARGUMENT - Specifies that the argument is a
 *                              positional argument.
 *
 *    ARG_OPTIONAL_ARGUMENT   - Specified that argument in the argv
 *                              array, indexed by the index value
 *                              pointed to by arg_index_ptr, is an
 *                              optional argument.
 *
 *    ARG_NO_MORE_ARGUMENTS   - Specifies that there are no more
 *                              arguments to parse.
 *
 *    ARG_ERROR_INVALID_SWITCH - Invalid switch specified on the command
 *                               line.
 *
 *    ARG_ERROR_MISSING_SWITCH_ARGUMENT - A dash character for an
 *                                        optional argument was found
 *                                        but is missing required
 *                                        characters following the dash.
 *
 *    ARG_ERROR_PARAM_WITH_MULTIPLE_SHORT_OPTIONS -
 *                                        A dash character followed by
 *                                        multiple short options has a short
 *                                        option that requires a parameter.
 *
 *    ARG_ERROR_DUPLICATE_SHORT_OPTIONS -
 *                                        A dash character followed by
 *                                        multiple short options has
 *                                        duplicate short option characters.
 *
 *  This function should called for each argument until the value
 *  ARG_NO_MORE_ARGUMENTS (0), is returned or a negative error value
 *  is returned.
 *
 =================================================================== */

int arg_get_arguments(int argc,
                     TCHAR *argv[],
                     const arg_option_t * argument_option_array_ptr,
                     int * arg_index_ptr,
                     int * option_int_ptr);

/* ====================================================================
 * Function: arg_report_status
 *
 * Abstract:
 *
 *    Report the status returned by the arg_get_arguments.
 * 
 * Inputs:
 *
 *     status            The status returned by the arg_get_arguments
 *                       function.
 * 
 *     argv_index_ptr    Pass argv[arg_index] where index is the variable
 *                       passed by address to the arg_get_arguments function.
 *
 *  This function has no return value.
 * 
 * ================================================================== */

void arg_report_status(int status, TCHAR * argv_index_ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ARG_GET_ARGUMENTS_H */
