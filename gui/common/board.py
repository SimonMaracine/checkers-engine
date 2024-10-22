from __future__ import annotations
import enum
import dataclasses
import copy
from typing import Iterator

from . import common


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


class MoveType(enum.Enum):
    Normal = enum.auto()
    Capture = enum.auto()


@dataclasses.dataclass
class Move:
    # Indices are always in the range [0, 31]

    @dataclasses.dataclass
    class _Normal:
        source_index: int
        destination_index: int

    @dataclasses.dataclass
    class _Capture:
        source_index: int
        destination_indices: list[int]

    def type(self) -> MoveType:
        match self.data:
            case self._Normal():
                return MoveType.Normal
            case self._Capture():
                return MoveType.Capture

    def __str__(self) -> str:
        match self.data:
            case self._Normal():
                result = f"{common._0_31_to_1_32(self.data.source_index)}x{common._0_31_to_1_32(self.data.destination_index)}"
            case self._Capture():
                result = str(common._0_31_to_1_32(self.data.source_index))
                for index in self.data.destination_indices:
                    result += f"x{common._0_31_to_1_32(index)}"

        return result

    data: _Normal | _Capture
