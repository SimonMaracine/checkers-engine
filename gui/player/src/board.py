import enum
import dataclasses

NULL_INDEX = -1


class MoveType(enum.Enum):
    Normal = enum.auto()
    Captre = enum.auto()


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

    move: _Normal | _Capture
    type: MoveType


class Player(enum.Enum):
    Black = 0b0001
    White = 0b0010


class GameOver(enum.Enum):
    None_ = enum.auto()
    WinnerBlack = enum.auto()
    WinnerWhite = enum.auto()
    TieBetweenBothPlayers = enum.auto()


class _Square(enum.Enum):
    None_     = 0b0000  # I don't usually align like this :P
    Black     = 0b0001
    BlackKing = 0b0101
    White     = 0b0010
    WhiteKing = 0b0110


type _Board = list[_Square]


@dataclasses.dataclass(frozen=True)
class _Position:
    board: _Board
    turn: Player


@dataclasses.dataclass
class _JumpCtx:
    board: _Board  # Use a copy of the board
    source_index: int
    destination_indices: list[int]


class _Direction(enum.Enum):
    NorthEast = enum.auto()
    NorthWest = enum.auto()
    SouthEast = enum.auto()
    SouthWest = enum.auto()


class _Diagonal(enum.Enum):
    Short = 1
    Long = 2


class CheckersBoard:
    def __init__(self):
        # Game data
        self._board: _Board = [_Square.None_] * 32
        self._turn = Player.Black
        self._plies_without_advancement = 0
        self._repetition_positions: list[_Position] = []

        # Management data
        self._selected_piece_index = NULL_INDEX
        self._legal_moves: list[Move] = []
        self._jump_square_indices: list[int] = []
        self._game_over = GameOver.None_

    def press_square(self, square: int):
        pass

    def release_square(self, square: int):
        pass

    def reset(self, position: str):
        pass

    def play_move(self, move: str):
        pass

    def get_game_over(self) -> GameOver:
        return self._game_over

    def get_turn(self) -> Player:
        return self._turn

    def get_plies_without_advancement(self) -> int:
        return self._plies_without_advancement

    @staticmethod
    def _generate_moves(board: _Board) -> list[Move]:
        pass

    @staticmethod
    def _generate_piece_capture_moves(board: _Board, moves: list[Move], square_index: int, player: Player, king: bool):
        pass

    @staticmethod
    def _generate_piece_capture_moves(board: _Board, moves: list[Move], square_index: int, player: Player, king: bool):
        pass

    @staticmethod
    def _check_piece_jumps(board: _Board, moves: list[Move], square_index: int, player: Player, king: bool, ctx: _JumpCtx) -> bool:
        pass

    @staticmethod
    def _offset_index(square_index: int, direction: _Direction, diagonal: _Diagonal) -> int:
        pass
