#! /usr/bin/env python3

import sys
import argparse

import comparator.comparator as comp


def main(args: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Compare the strength of two checkers engines")
    parser.add_argument("match_file", help="Match file describing the positions and other options")
    parser.add_argument("path_black", help="File path to the engine playing as black")
    parser.add_argument("path_white", help="File path to the engine playing as white")
    arguments = parser.parse_args(args[1:])

    try:
        match_file = comp.parse_match_file(arguments.match_file)
    except comp.MatchError as err:
        print(f"Could not parse match file: {err}", file=sys.stderr)
        return 1

    try:
        comp.run(match_file, arguments.path_black, arguments.path_white)
    except comp.MatchError as err:
        print(f"An error occurred running the match: {err}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
