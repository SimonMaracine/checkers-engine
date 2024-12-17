#! /usr/bin/env python3

import sys
from typing import TextIO

from common import board


def write_position(opening: str, file: TextIO):
    move1, move2, move3 = opening.split()

    local_board = board.CheckersBoard(None, None)
    local_board.play_move(move1)
    local_board.play_move(move2)
    local_board.play_move(move3)

    file.write(local_board.get_position() + "\n")


def main(args: list[str]) -> int:
    if len(args) != 2:
        print("No input", file=sys.stderr)
        return 1

    try:
        with open(args[1], "r") as file:
            openings = list(filter(lambda line: line.strip(), file.readlines()))
    except Exception as err:
        print(f"Could not read file: {err}", file=sys.stderr)
        return 1

    try:
        with open("positions", "w") as file:
            for opening in openings:
                write_position(opening, file)
    except Exception as err:
        print(f"Could not write file: {err}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
