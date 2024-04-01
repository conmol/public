#!/usr/bin/env python
#=======================================================================
# File: make_eval_table.py
# Author: Bill Hallahan
# Date: February 18, 2024
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
    python make_eval_table.py <sequence_codes>

    python make_eval_table.py [-h | --help]
    
    sequence_codes is a comma-delimited list of four sequence codes
    with no spaces in the list.
    
    python make_eval_table.py ODD,7K,8K,4T
"""
import sys
#import os
import collections
from argparse import ArgumentParser

card_value_list = ['A', '2', '3', '4', '5', '6', '7', \
                   '8', '9', 'T', 'J', 'Q', 'K']

card_value_name_list = ['ACE', 'TWO', 'THREE', 'FOUR', 'FIVE', 'SIX', 'SEVEN', \
                        'EIGHT', 'NINE', 'TEN', 'JACK', 'QUEEN', 'KING']

def get_sequence_fn_dictionary():
    """ Create a sequence file from the card stack. """
    # Create the sequence functions.
    # Major sequences
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
    # Minor sequences
    seq_7k_fn = lambda x: 1 if x[0] in ['7', '8', '9', 'T', 'J', 'Q', 'K'] else 0
    seq_8k_fn = lambda x: 1 if x[0] in ['8', '9', 'T', 'J', 'Q', 'K'] else 0
    seq_8a_fn = lambda x: 1 if x[0] in ['8', '9', 'T', 'J', 'Q', 'K', 'A'] else 0
    seq_9a_fn = lambda x: 1 if x[0] in ['9', 'T', 'J', 'Q', 'K', 'A'] else 0
    seq_92_fn = lambda x: 1 if x[0] in ['9', 'T', 'J', 'Q', 'K', 'A','2'] else 0
    seq_t2_fn = lambda x: 1 if x[0] in ['T', 'J', 'Q', 'K', 'A','2'] else 0
    seq_t3_fn = lambda x: 1 if x[0] in ['T', 'J', 'Q', 'K', 'A','2', '3'] else 0
    seq_j3_fn = lambda x: 1 if x[0] in ['J', 'Q', 'K', 'A','2', '3'] else 0
    seq_j4_fn = lambda x: 1 if x[0] in ['J', 'Q', 'K', 'A','2', '3', '4'] else 0
    seq_q4_fn = lambda x: 1 if x[0] in ['Q', 'K', 'A','2', '3', '4'] else 0
    seq_q5_fn = lambda x: 1 if x[0] in ['Q', 'K', 'A','2', '3', '4', '5'] else 0
    seq_k5_fn = lambda x: 1 if x[0] in ['K', 'A','2', '3', '4', '5'] else 0
    seq_k6_fn = lambda x: 1 if x[0] in ['K', 'A','2', '5', '7', '9', 'J', 'K'] else 0
    seq_odd_fn = lambda x: 1 if x[0] in ['A', '3', '5', '7', '9', 'J', 'K'] else 0
    seq_sc_fn = lambda x: 1 if x[-1] in ['S', 'C'] else 0
    seq_sh_fn = lambda x: 1 if x[-1] in ['S', 'H'] else 0
    seq_sd_fn = lambda x: 1 if x[-1] in ['S', 'D'] else 0
    # Create a dictionary that allows looking up the sequence code.
    seq_code_fn_dict = {"A6" : seq_a6_fn,
                        "A7" : seq_a7_fn,
                        "27" : seq_27_fn,
                        "28" : seq_28_fn,
                        "38" : seq_38_fn,
                        "39" : seq_39_fn,
                        "49" : seq_49_fn,
                        "4T" : seq_4t_fn,
                        "5T" : seq_5t_fn,
                        "5J" : seq_5j_fn,
                        "6J" : seq_6j_fn,
                        "6Q" : seq_6q_fn,
                        "7Q" : seq_7q_fn,
                        "EV" : seq_ev_fn,
                        "RED" : seq_red_fn,
                        "CD" : seq_cd_fn,
                        "HC" : seq_hc_fn,
                        "7K" : seq_7k_fn,
                        "8K" : seq_8k_fn,
                        "8A" : seq_8a_fn,
                        "9A" : seq_9a_fn,
                        "92" : seq_92_fn,
                        "T2" : seq_t2_fn,
                        "T3" : seq_t3_fn,
                        "J3" : seq_j3_fn,
                        "J4" : seq_j4_fn,
                        "Q4" : seq_q4_fn,
                        "Q5" : seq_q5_fn,
                        "K5" : seq_k5_fn,
                        "K6" : seq_k6_fn,
                        "ODD" : seq_odd_fn,
                        "SC" : seq_sc_fn,
                        "SH" : seq_sh_fn,
                        "SD" : seq_sd_fn}
    return seq_code_fn_dict

def parse_comma_delimited_list(sequence_codes):
    """ Parse a comma-delimited list of four sequence codes. """
    usequence_codes = sequence_codes.strip().upper()
    seq_code_list = usequence_codes.split(',')
    # Validate that there are exactly four sequence codes.
    if len(seq_code_list) != 4:
        raise ValueError("Error: There must be exactly four different sequence codes")
    # Make sure that all sequence codes in the list are unique.
    if not all(count == 1 for count in collections.Counter(seq_code_list).values()):
        raise ValueError("Error: All sequence codes must be unique")         
    return seq_code_list

def get_index_value_name_list(seq_code_list, seq_code_fn_dict):
    """ Display a table for the four sequence codes. """
    index_card_value_name_list = []
    # Loop over the values from 0 to 15.
    for index in range(0, 16):
        index_card_name_list = []
        # Loop over all card value indices from 0 to 12
        for card_index in range(0, 13):
            # Get the card value from 'A' to 'K'.
            card_value = card_value_list[card_index]
            # Loop over each sequence code in the sequence code
            # list. For a match, if the current bit in the index
            # is a zero then the card value must NOT be in the
            # specified sequence group. If the current bit in the
            # index is a one, then the card value MUST be in the
            # specified sequence group.
            is_match_flag = True
            bit = 8
            for seq_code in seq_code_list:
                seq_fn = seq_code_fn_dict[seq_code]
                seq_group_flag = seq_fn(card_value)
                # If the current index bit is zero, then invert
                # the in_seq_group_flag
                if (bit & index) == 0:
                    seq_group_flag = not seq_group_flag
                # If the card_value did not pass, skip this card value.
                if not seq_group_flag:
                    is_match_flag = False
                    break;
                # Shift bit down to correspond to the next sequence code.
                bit >>= 1
            # Is the card value a match for all index bits for all sequences?
            if is_match_flag:
                # Instead of appending the card value, append the card value name.
                card_value_name = card_value = card_value_name_list[card_index]
                index_card_name_list.append(card_value_name)
        index_card_value_name_list.append(index_card_name_list)
    return index_card_value_name_list

def display_sequence_table(seq_code_list, seq_code_fn_dict):
    """ Display a table for the four sequence codes. """
    ivn_list = get_index_value_name_list(seq_code_list, seq_code_fn_dict)
    # Create the table.
    print("----------------------------")
    print(f"{seq_code_list[0]:>3}  {seq_code_list[1]:>3}  {seq_code_list[2]:>3}  {seq_code_list[3]:>3}   Card Values")
    print("----------------------------")
    # Loop over all binary values.
    for index in range(0, 16):
        bit8 = '0' if (8 & index) == 0 else 1
        bit4 = '0' if (4 & index) == 0 else 1
        bit2 = '0' if (2 & index) == 0 else 1
        bit1 = '0' if (1 & index) == 0 else 1
        # Get the list of card values.
        card_value_name_list = ivn_list[index]
        # Get a comma-delimited list of value names.
        if len(card_value_name_list) > 0:
            card_value_name_text = ", ".join(card_value_name_list)
        else:
            card_value_name_text = "NONE"        
        # Create the card value text.
        print(f"{bit8:>3}  {bit4:>3}  {bit2:>3}  {bit1:>3}   {card_value_name_text}") 
    
def make_eval_table_main(sequence_codes):
    """ Create a sequence file from a deck of cards. """
    seq_code_list = parse_comma_delimited_list(sequence_codes)
    seq_code_fn_dict = get_sequence_fn_dictionary()
    display_sequence_table(seq_code_list, seq_code_fn_dict)
    return 0

# Start of main program.
def main(argv=None):
    # Initialize the command line parser.
    parser = ArgumentParser(description='Create a card value evaluation table for four sequences.',
                            epilog=f'Copyright (c) 2024 TODO: your-name-here.',
                            add_help=True,
                            argument_default=None, # Global argument default
                            usage=__doc__)
    parser.add_argument(action='store',
                        dest='sequence_codes',
                        help='A comma-delimited list of four sequence codes with no spaces.')
    # Parse the command line.
    arguments = parser.parse_args(args=argv)
    sequence_codes = arguments.sequence_codes
    status = 0
    try:
        make_eval_table_main(sequence_codes)
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
