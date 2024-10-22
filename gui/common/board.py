from __future__ import annotations
import enum
import dataclasses
import copy
from typing import Iterator


class GameOver(enum.Enum):
    None_ = enum.auto()
    WinnerBlack = enum.auto()
    WinnerWhite = enum.auto()
    TieBetweenBothPlayers = enum.auto()


class Player(enum.Enum):
    Black = 0b0001
    White = 0b0010


class Square(enum.Enum):
    None_     = 0b0000  # I don't usually align like this :P
    Black     = 0b0001
    BlackKing = 0b0101
    White     = 0b0010
    WhiteKing = 0b0110


class Board:
    __slots__ = ("_board",)

    def __init__(self):
        self._board: list[Square] = [Square.None_ for _ in range(32)]

    def __getitem__(self, index: int) -> Square:
        return self._board[index]

    def __setitem__(self, index: int, value: Square):
        self._board[index] = value

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Board):
            return NotImplemented

        return all(map(lambda i: self[i] == other[i], range(32)))

    def __iter__(self) -> Iterator:
        return iter(self._board)

    def __copy__(self) -> Board:
        board = Board()
        board._board = copy.copy(self._board)
        return board

    def clear(self):
        for i in range(32):
            self._board[i] = Square.None_


@dataclasses.dataclass(frozen=True)
class Position:
    board: Board
    turn: Player
