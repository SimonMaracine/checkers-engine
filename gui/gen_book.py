#! /usr/bin/env python3

import sys
from typing import TextIO

from common import board


def write_book(openings: list[str], file: TextIO):
    book: dict[str, set[str]] = {}

    for opening in openings:
        move1, move2, move3 = opening.split()
        local_board = board.CheckersBoard(None, None)

        try:
            book[local_board.get_position()].add(move1)
        except KeyError:
            book[local_board.get_position()] = set()
            book[local_board.get_position()].add(move1)

        local_board.play_move(move1)

        try:
            book[local_board.get_position()].add(move2)
        except KeyError:
            book[local_board.get_position()] = set()
            book[local_board.get_position()].add(move2)

        local_board.play_move(move2)

        try:
            book[local_board.get_position()].add(move3)
        except KeyError:
            book[local_board.get_position()] = set()
            book[local_board.get_position()].add(move3)

        local_board.play_move(move3)

    for position in book:
        file.write(position)

        for move in book[position]:
            file.write(" ")
            file.write(move)

        file.write("\n")


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
        with open("book", "w") as file:
            write_book(openings, file)
    except Exception as err:
        print(f"Could not write file: {err}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
