#!/usr/bin/env python
#=======================================================================
# File: bracelet_test.py
# Author: Bill Hallahan
# Date: January 10, 2024
# 
# Abstract:
#
#   This program reads a deck from the system clipboard and
#   tests for various de Bruijn sequences.
#
# Copyright (c) 2024, William Hallahan.
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
#=======================================================================
"""
    python bracelet_test.py 

    python bracelet_test.py [-h | --help]

    Read a card deck from the system clipboard
    and test for binary bracelet codes and for
    the base 4 suit bracelet code based on suits,
    similar to the code used in the routine
    Suitability.
    
    Example text input:

    QH, 7C, 8S, 2D, 3C, AH, 5S, QD, QS, 2C, 6S, KS, 3S,
    KD, 7D, 9D, 5C, 2S, AC, 6C, 5H, 8C, 7H, 10H, 4D, 8D,
    KH, 6D, QC, 5D, 7S, AD, 2H, JC, KC, 10D, 4C, 10C, JS,
    4H, 10S, 4S, JH, 3D, AS, 9H, 3H, 6H, 9C, JD, 8H, 9S

"""
import sys
import pyperclip
from argparse import ArgumentParser

CARD_VALUE_LIST = \
    ['A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K']
CARD_SUIT_LIST = ['S', 'H', 'C', 'D']

def parse_stack_text(text):
    """ Parse the card text and create a stack list. """
    # Parse the input text to get the stack list.
    stack_list = []
    line_list = text.split('\n')
    for line in line_list:
        sline = line.strip()
        if sline:
            if ',' in sline:
                card_list = sline.split(',')
            else:
                card_list = sline.split()                
            if len(card_list) > 0:
                for card in card_list:
                    scard = card.strip().upper()
                    if scard:
                        # Change "10" to "T"
                        scard = scard.replace("10", "T")
                        # If the card has a valid form then
                        # add the card to the stack list.
                        if len(scard) == 2 \
                            and scard[0] in CARD_VALUE_LIST \
                            and scard[-1] in CARD_SUIT_LIST:
                            stack_list.append(scard)
    return stack_list

def power_of_two_exponent(x):
    """ Get the power of two that is greater than or equal
        to the passed value.
    """
    count = 0
    x -= 1
    while x > 0:
        count += 1
        x >>= 1
    return count

def get_code(stack_list, index, code_length, seq_fn):
    """ Extract 6 consecutive cards from the passed stack list starting at
        the specified index and, if necessary, wrapping to the beginning of
        the stack, and use the passed bit function to determine whether each
        card is represented by a 0 or a 1. Create a two-digit octal number
        from the sequence of 6 binary digits formed by the six cards.
    """
    suit_code_text = ''
    suit_code = 0
    card_count = len(stack_list)
    # Get the suit code.
    for i in range(index, index + code_length):
        card_text = stack_list[i % card_count]
        binary_digit = seq_fn(card_text)
        suit_code = suit_code << 1
        suit_code += binary_digit
    # Convert the 6 bit suit code to two octal digits.
    high_octal_digit = suit_code >> 3
    low_octal_digit = suit_code & 7
    suit_code_text = f"{high_octal_digit}{low_octal_digit}"
    return suit_code_text

def test_for_bracelet_code(stack_list,
                           code_length,
                           seq_fn,
                           seq_name,
                           show_failures_flag):
    """ Test for bracelet code. """
    has_bracelet_code = True
    seq_code_unique_dict = {}
    # Loop over all cards.
    for i in range(0, len(stack_list)):
        # Get the code for the card at index i.
        seq_code = get_code(stack_list, i, code_length, seq_fn)
        if seq_code in seq_code_unique_dict:
            has_bracelet_code = False
            if show_failures_flag:
                index = i + 1
                print(f"Sequence {seq_name} has duplicate code {seq_code} at index {index}")
            else:
                break
        seq_code_unique_dict[seq_code] = 1
    return has_bracelet_code

def do_bracelet_tests(stack_list, show_failures_flag):
    """  Run Test for binary bracelet codes.
    """
    # Create the sequence functions.
    seq_a6_fn = lambda x: 1 if x[0] in ['A', '2', '3', '4', '5', '6'] else 0
    seq_a7_fn = lambda x: 1 if x[0] in ['A', '2', '3', '4', '5', '6', '7'] else 0
    seq_27_fn = lambda x: 1 if x[0] in ['2', '3', '4', '5', '6', '7'] else 0
    seq_28_fn = lambda x: 1 if x[0] in ['2', '3', '4', '5', '6', '7', '8'] else 0
    seq_38_fn = lambda x: 1 if x[0] in ['3', '4', '5', '6', '7', '8'] else 0
    seq_39_fn = lambda x: 1 if x[0] in ['3', '4', '5', '6', '7', '8', '9'] else 0
    seq_49_fn = lambda x: 1 if x[0] in ['4', '5', '6', '7', '8', '9'] else 0
    seq_4t_fn = lambda x: 1 if x[0] in ['4', '5', '6', '7', '8', '9', 'T'] else 0
    seq_5t_fn = lambda x: 1 if x[0] in ['5', '6', '7', '8', '9', 'T'] else 0
    seq_5j_fn = lambda x: 1 if x[0] in ['5', '6', '7', '8', '9', 'T', 'J'] else 0
    seq_6j_fn = lambda x: 1 if x[0] in ['6', '7', '8', '9', 'T', 'J'] else 0
    seq_6q_fn = lambda x: 1 if x[0] in ['6', '7', '8', '9', 'T', 'J', 'Q'] else 0
    seq_7q_fn = lambda x: 1 if x[0] in ['7', '8', '9', 'T', 'J', 'Q'] else 0
    seq_ev_fn = lambda x: 1 if x[0] in ['2', '4', '6', '8', 'T', 'Q'] else 0
    seq_red_fn = lambda x: 1 if x[-1] in ['H', 'D'] else 0
    seq_cd_fn = lambda x: 1 if x[-1] in ['C', 'D'] else 0
    seq_hc_fn = lambda x: 1 if x[-1] in ['H', 'C'] else 0
    # Special sequences.
    seq_m34_fn = lambda x: 1 if x[0] in ['3', '4', '6', '8', '9', 'Q'] else 0
    seq_m46_fn = lambda x: 1 if x[0] in ['4', '5', '6', '8', 'T', 'Q'] else 0
    seq_m47_fn = lambda x: 1 if x[0] in ['4', '5', '6', '7', '8', 'T', 'Q'] else 0
    seq_m58_fn = lambda x: 1 if x[0] in ['5', '6', '7', '8', 'T', 'Q'] else 0
    seq_m59_fn = lambda x: 1 if x[0] in ['5', '6', '7', '8', '9', 'T', 'Q'] else 0
    seq_m6q_fn = lambda x: 1 if x[0] in ['6', '7', '8', '9', 'T', 'Q'] else 0
    seq_mpr_fn = lambda x: 1 if x[0] in ['2', '3', '5', '7', 'J', 'K'] else 0
    seq_mfi_fn = lambda x: 1 if x[0] in ['A', '2', '3', '5', '8', 'K'] else 0
    seq_mlu_fn = lambda x: 1 if x[0] in ['A', '2', '3', '4', '7', 'J'] else 0
    # Create a list with the sequences and the corresponding sequence code text.
    seq_fn_code_list = [(seq_a6_fn, "A6"),
                        (seq_a7_fn, "A7"),
                        (seq_27_fn, "27"),
                        (seq_28_fn, "28"),
                        (seq_38_fn, "38"),
                        (seq_39_fn, "39"),
                        (seq_49_fn, "49"),
                        (seq_4t_fn, "4T"),
                        (seq_5t_fn, "5T"),
                        (seq_5j_fn, "5J"),
                        (seq_6j_fn, "6J"),
                        (seq_6q_fn, "6Q"),
                        (seq_7q_fn, "7Q"),
                        (seq_ev_fn, "EV"),
                        (seq_red_fn, "RED"),
                        (seq_cd_fn, "CD"),
                        (seq_hc_fn, "HC"),
                        (seq_m34_fn, "M34"),
                        (seq_m46_fn, "M46"),
                        (seq_m47_fn, "M47"),
                        (seq_m58_fn, "M58"),
                        (seq_m59_fn, "M59"),
                        (seq_m6q_fn, "M6Q"),
                        (seq_mpr_fn, "PR"),
                        (seq_mfi_fn, "FI"),
                        (seq_mlu_fn, "LU")]
    # Calculate the length of the binary code.
    card_count = len(stack_list)
    code_length = power_of_two_exponent(card_count)
    # Loop over all sequence function and code tuples and test
    # if a valid bracelet code exists.
    for seq_fn, seq_name in seq_fn_code_list:
        if test_for_bracelet_code(stack_list,
                                  code_length,
                                  seq_fn,
                                  seq_name, # For reporting
                                  show_failures_flag):
            print(f"{seq_name} sequence found")

def do_suitability_test(stack_list, show_failures_flag):
    """ Test if the pattern of 3 consecutive card suits is unique. """
    """ Check if the passed stack is suitable. """
    is_suitable = True
    unique_dict = {}
    card_count = len(stack_list)
    # Calculate the length of the base 4 code.
    # First calculate the binary code length.
    code_length = power_of_two_exponent(card_count)
    # Because this is a base four, the power of two must
    # be divided by two. Make the power of two an even number.
    if (code_length % 2) != 0:
        code_length += 1
    # Divide the code length by two.
    code_length >>= 1
    for i in range(0, card_count):
        seq_code = \
        "".join(stack_list[(i+j) % card_count][-1] for j in range(0, code_length))
        if seq_code in unique_dict:
            is_suitable = False
            if show_failures_flag:
                index = i + 1
                print(f"Duplicate code {seq_code} at index {index}")
            else:
                break
        else:
            unique_dict[seq_code] = 1
    if is_suitable:
        print("Suitability supported")

def bracelet_test_main(show_failures_flag):
    """ Check the deck on the clipboard for bracelet codes. """
    # Read the text from the clipboard
    text = pyperclip.paste()
    stack_list = parse_stack_text(text)
    card_count = len(stack_list)
    print(f"The stack contains {card_count} cards.")
    if card_count > 1:
        do_bracelet_tests(stack_list, show_failures_flag)
        do_suitability_test(stack_list, show_failures_flag)
    return 0

def main(argv=None):
    # Initialize the command line parser.
    parser = ArgumentParser(description='Create a Latex table for a deck of cards.',
                            epilog=f'Copyright (c) 2023 William Hallahan.',
                            add_help=True,
                            argument_default=None, # Global argument default
                            usage=__doc__)
    parser.add_argument('-s',
                        '--show',
                        action='store_true',
                        dest='show_failures_flag',
                        default=False,
                        help='Show the code collisions that invalidates a bracelet code')
    arguments = parser.parse_args(args=argv)
    show_failures_flag = arguments.show_failures_flag
    status = 0
    try:
        bracelet_test_main(show_failures_flag)
    except ValueError as value_error:
        print(value_error)
        status = -1
    except OSError as os_error:
        print(os_error)
        status = -1
    except MemoryError as mem_error:
        print(mem_error)
        status = -1
    return status

if __name__ == "__main__":
    sys.exit(main())
