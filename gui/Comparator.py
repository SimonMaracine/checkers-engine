#! /usr/bin/env python3

import sys
import argparse

import comparator.comparator as comp


def main(args: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("match", help="The match file describing the positions and other options")
    arguments = parser.parse_args(args)

    try:
        match_file = comp.parse_match_file(arguments.match)
    except comp.MatchError as err:
        print(f"Could not parse match file: {err}", file=sys.stderr)
        return 1

    try:
        comp.run_match(match_file, arguments.black, arguments.white)
    except comp.MatchError as err:
        print(f"An error occurred running the match: {err}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
