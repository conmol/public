#!/usr/bin/env python
#=======================================================================
# File: red_stack_trainer.py
# Author: Bill Hallahan
# Date: February 12, 2024
# 
# Abstract:
#
#   This program is for training to learn the Red stack.
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
    python red_stack_trainer.py
"""
import sys
from random import randrange
from argparse import ArgumentParser

red_stack = ['AD', 'JC', '10D', '4D', '5H', '8S', '6C', '2D',
             'KC', 'KD', 'QD', '8D', 'QH', '8C', '5D', '7H',
             'JS', 'QC', 'JD', '7D', '10H', '3C', '9H', '2C',
             '6H', '10S', '9C', '6D', '8H', 'QS', 'AH', 'AS',
             '2S', '6S', 'AC', '4H', '5S', 'KS', '3H', '4S',
             '9S', '7C', '3D', '2H', '3S', '7S', '4C', 'JH',
             '5C', 'KH', '10C', '9D']

def get_six_cards(index):
    """ Validate the cherry stack. """
    len_red_stack = len(red_stack)
    card0 = red_stack[index]
    card1 = red_stack[(index + 1) % len_red_stack]
    card2 = red_stack[(index + 2) % len_red_stack]
    card3 = red_stack[(index + 3) % len_red_stack]
    card4 = red_stack[(index + 4) % len_red_stack]
    card5 = red_stack[(index + 5) % len_red_stack]
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

def red_stack_trainer_main():
    """ Display suit color and allow user to determine the first card. """
    print("Enter a two-character value and suit, e.g. AH. If the correct")
    print("answer is entered then another problem is presented.")
    # Loop and give the user a pattern to identify.
    while True:
        # Get a random number from 0 to 51.
        index = randrange(len(red_stack))
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
        red_stack_trainer_main()
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
