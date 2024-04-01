#!/usr/bin/env python
#=======================================================================
# File: cherry_stack_trainer.py
# Author: Bill Hallahan
# Date: February 12, 2024
# 
# Abstract:
#
#   This program is for training to learn the Cherry stack.
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
    python cherry_stack_trainer.py
"""
import sys
from random import randrange
from argparse import ArgumentParser

cherry_stack = ['9H', '5S', '10H', '8C', '2D', '4C', '7C', 'KC',
                'KD', 'AH', '2H', '4H', '7H', 'AD', 'QS', '3C',
                '9S', '9C', '5D', '10C', '7D', 'AS', 'QD', '3H',
                '5H', '4D', '6S', 'QH', '10S', '7S', 'KH', 'KS',
                'AC', '2C', 'JS', '6C', 'QC', '10D', '8H', '2S',
                'JD', '8D', 'JH', '8S', 'JC', '6H', '3D', '5C',
                '4S', '6D', '3S', '9D']

def get_six_cards(index):
    """ Validate the cherry stack. """
    len_cherry_stack = len(cherry_stack)
    card0 = cherry_stack[index]
    card1 = cherry_stack[(index + 1) % len_cherry_stack]
    card2 = cherry_stack[(index + 2) % len_cherry_stack]
    card3 = cherry_stack[(index + 3) % len_cherry_stack]
    card4 = cherry_stack[(index + 4) % len_cherry_stack]
    card5 = cherry_stack[(index + 5) % len_cherry_stack]
    return (card0, card1, card2, card3, card4, card5)

def get_colored_text(text, is_red_flag):
    """ Return a red version of 'text'."""
    if is_red_flag:
        attr = []
        attr.append('31')
        # Make bold text.
        #attr.append('1')
        text = '\x1b[%sm%s\x1b[0m' % (';'.join(attr), text)
    return text

def display_suit_colors(card_tuple):
    """ Display a colored symbol for each card. """
    card_shape = u"\u2588"
    print("    ", end="")
    for card in card_tuple:
        suit = card[-1]
        is_red_flag = suit == 'H' or suit == 'D'
        symbol = get_colored_text(card_shape, is_red_flag)
        print(f"{symbol} ", end="")
    print()

def cherry_stack_trainer_main():
    """ Display suit color and allow user to determine the first card. """
    print("Enter a two-character value and suit, e.g. AH. If the correct")
    print("answer is entered then another problem is presented.")
    # Loop and give the user a pattern to identify.
    while True:
        # Get a random number from 0 to 51.
        index = randrange(len(cherry_stack))
        # Get the six cards starting at the random index.
        card_tuple = get_six_cards(index)
        # Display the card suit colors.
        display_suit_colors(card_tuple)
        # Allow the user to enter a a card.
        while True:
            card = input("Enter a card > ")
            ucard = card.strip().upper()
            if ucard == card_tuple[0]:
                break
            else:
                print("Wrong. Try Again")
    return 0

# Start of main program.
def main(argv=None):
   # Initialize the command line parser.
    parser = ArgumentParser(description='This program generates a "Code to Card stack."',
                            epilog=f'Copyright (c) 2023 Bill Hallahan',
                            add_help=True,
                            argument_default=None, # Global argument default
                            usage=__doc__)
    # Parse the command line.
    arguments = parser.parse_args(args=argv)
    status = 0
    try:
        cherry_stack_trainer_main()
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
