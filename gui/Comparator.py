#! /usr/bin/env python3

import sys
import argparse

from comparator import comparator
from comparator import error


def main(args: list[str]) -> int:
    try:
        parser = argparse.ArgumentParser(description="Compare the strength of two checkers engines")
        parser.add_argument("match_file", help="Match file describing the positions and other options")
        parser.add_argument("path_black", help="File path to the engine playing as black")
        parser.add_argument("path_white", help="File path to the engine playing as white")
        arguments = parser.parse_args(args[1:])

        try:
            match_file = comparator.parse_match_file(arguments.match_file)
        except error.ComparatorError as err:
            print(f"Could not parse match file: {err}", file=sys.stderr)
            return 1

        try:
            comparator.run(match_file, arguments.path_black, arguments.path_white)
        except error.ComparatorError as err:
            print(f"An error occurred during the process: {err}", file=sys.stderr)
            return 1
    except KeyboardInterrupt:
        print(file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
