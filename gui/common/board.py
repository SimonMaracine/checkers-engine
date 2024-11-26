"""
    branched capture B:W1,3,8,9,10,16,17:B12,20,21,23,26,27,29,31
    longest capture W:WK4:B6,7,8,14,15,16,22,23,24

    winner white W:WK7,K8:B16
    winner black W:W8:BK12,K15
    winner white B:W1,2,K7:B5,6,15,9
    winner black W:W25,6:BK14,29,30

    tie W:WK2:BK30
"""

from __future__ import annotations
import sys
import enum
import dataclasses
import re
import copy
import tkinter as tk
from typing import Callable, Iterator

from . import common

NULL_INDEX = -1

def opponent(player: Player) -> Player:
    return CheckersBoard._opponent(player)


class GameOver(enum.Enum):
    None_ = enum.auto()
    WinnerBlack = enum.auto()
    WinnerWhite = enum.auto()
    TieBetweenBothPlayers = enum.auto()


class Player(enum.Enum):
    Black = 0b0001
    White = 0b0010


class _Square(enum.Enum):
    None_     = 0b0000  # I don't usually align like this :P
    Black     = 0b0001
    BlackKing = 0b0101
    White     = 0b0010
    WhiteKing = 0b0110


class _Board:
    __slots__ = ("_board",)

    def __init__(self):
        self._board: list[_Square] = [_Square.None_ for _ in range(32)]

    def __getitem__(self, index: int) -> _Square:
        return self._board[index]

    def __setitem__(self, index: int, value: _Square):
        self._board[index] = value

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, _Board):
            return NotImplemented

        return all(map(lambda i: self[i] == other[i], range(32)))

    def __iter__(self) -> Iterator[_Square]:
        return iter(self._board)

    def __copy__(self) -> _Board:
        board = _Board()
        board._board = copy.copy(self._board)
        return board

    def clear(self):
        for i in range(32):
            self._board[i] = _Square.None_


@dataclasses.dataclass(slots=True, frozen=True)
class _Position:
    board: _Board
    turn: Player


class MoveType(enum.Enum):
    Normal = enum.auto()
    Capture = enum.auto()


@dataclasses.dataclass(slots=True, frozen=True)
class Move:
    # Indices are always in the range [0, 31]

    @dataclasses.dataclass(slots=True)
    class _Normal:
        source_index: int
        destination_index: int

    @dataclasses.dataclass(slots=True)
    class _Capture:
        source_index: int
        destination_indices: list[int]

    def type(self) -> MoveType:
        match self._move:
            case self._Normal():
                return MoveType.Normal
            case self._Capture():
                return MoveType.Capture

    def normal(self) -> _Normal:
        assert isinstance(self._move, self._Normal)

        return self._move

    def capture(self) -> _Capture:
        assert isinstance(self._move, self._Capture)

        return self._move

    def __str__(self) -> str:
        match self._move:
            case self._Normal():
                result = f"{common._0_31_to_1_32(self._move.source_index)}x{common._0_31_to_1_32(self._move.destination_index)}"
            case self._Capture():
                result = str(common._0_31_to_1_32(self._move.source_index))
                for index in self._move.destination_indices:
                    result += f"x{common._0_31_to_1_32(index)}"

        return result

    _move: _Normal | _Capture


@dataclasses.dataclass(slots=True)
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
    PIECE_WHITE = "#8c0a0a"
    PIECE_BLACK = "#101019"
    GOLD = "#F3A916"
    PINK = "#F69A8E"
    INDIGO = "#958ECD"
    DARKER_INDIGO = "#6D63BB"

    def __init__(self, on_piece_move: Callable[[Move], None] | None, canvas: tk.Canvas | None, redraw: bool = True):
        # Game data
        self._board = _Board()
        self._turn = Player.Black
        self._plies_without_advancement = 0
        self._repetition_positions: list[_Position] = []

        # Management data
        self._legal_moves: list[Move] = []
        self._selected_piece_square = NULL_INDEX
        self._jump_squares: list[int] = []
        self._game_over = GameOver.None_
        self._user_input = False
        self._on_piece_move = on_piece_move
        self._canvas = canvas
        self._redraw = redraw

        self._setup()

    def set_user_input(self, user_input: bool):
        self._user_input = user_input

    def redraw(self):
        assert self._canvas is not None

        self._draw_pieces(self._canvas)

    def press_square_left_button(self, square: int):
        if not self._user_input:
            return

        if not CheckersBoard._is_black_square(square):
            return

        square = common._1_32_to_0_31(common._0_64_to_1_32(square))

        if self._select_piece(square):
            return

        if self._selected_piece_square == NULL_INDEX:
            return

        normal = all(map(lambda move: move.type() == MoveType.Normal, self._legal_moves))
        capture = all(map(lambda move: move.type() == MoveType.Capture, self._legal_moves))

        if normal:
            for move in self._legal_moves:
                if CheckersBoard._playable_normal_move(move, self._selected_piece_square, square):
                    self._play_normal_move(move)

                    self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)
                    self._selected_piece_square = NULL_INDEX

                    if self._redraw and self._canvas:
                        self._canvas.delete("selection")
                        self._canvas.delete("tiles")

                    break
        elif capture:
            self._select_jump_square(square)

            for move in self._legal_moves:
                if CheckersBoard._playable_capture_move(move, self._selected_piece_square, self._jump_squares):
                    self._play_capture_move(move)

                    self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)
                    self._selected_piece_square = NULL_INDEX
                    self._jump_squares.clear()

                    if self._redraw and self._canvas:
                        self._canvas.delete("selection")
                        self._canvas.delete("tiles")

                    break
        else:
            assert False

    def press_square_right_button(self):
        if not self._user_input:
            return

        if self._selected_piece_square == NULL_INDEX:
            return

        self._selected_piece_square = NULL_INDEX
        self._jump_squares.clear()

        if self._redraw and self._canvas:
            self._canvas.delete("selection")
            self._canvas.delete("tiles")

    def reset(self, position_string: str | None = None):
        self._clear()
        self._setup(position_string)

    def play_move(self, move_string: str):
        # Validate only the format
        if not CheckersBoard._valid_move_string(move_string):
            raise RuntimeError(f"Invalid move string: {move_string}")

        move = CheckersBoard._parse_move_string(move_string)

        if not move in self._legal_moves:
            raise RuntimeError(f"Invalid move {move}")

        # Play the move
        match move.type():
            case MoveType.Normal:
                self._play_normal_move(move)
            case MoveType.Capture:
                self._play_capture_move(move)

        # This needs to be done even for the computer
        if self._redraw and self._canvas:
            self._canvas.delete("selection")
            self._canvas.delete("tiles")

    def get_game_over(self) -> GameOver:
        return self._game_over

    def get_turn(self) -> Player:
        return self._turn

    def get_plies_without_advancement(self) -> int:
        return self._plies_without_advancement

    def get_position(self) -> str:
        black: list[str] = []
        white: list[str] = []

        for i, square in enumerate(self._board):
            match square:
                case _Square.None_:
                    pass
                case _Square.Black:
                    black.append(str(i))
                    pass
                case _Square.BlackKing:
                    black.append(f"K{i}")
                    pass
                case _Square.White:
                    white.append(str(i))
                    pass
                case _Square.WhiteKing:
                    white.append(f"K{i}")
                    pass

        turn = "B" if self._turn == Player.Black else "W"

        return f"{turn}:W{",".join(white)}:B{",".join(black)}"

    def _setup(self, position_string: str | None = None):
        START_POSITION = "B:W1,2,3,4,5,6,7,8,9,10,11,12:B21,22,23,24,25,26,27,28,29,30,31,32"

        string = position_string or START_POSITION

        # Validate only the format
        if not CheckersBoard._valid_position_string(string):
            print(f"Invalid position string: {string}", file=sys.stderr)
            string = START_POSITION

        position = CheckersBoard._parse_position_string(string)

        self._board = position.board
        self._turn = position.turn

        self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)

        if self._redraw and self._canvas:
            self._draw_pieces(self._canvas)

    def _select_piece(self, square: int) -> bool:
        if self._board[square] != _Square.None_ and self._selected_piece_square != square:
            self._selected_piece_square = square
            self._jump_squares.clear()

            if self._redraw and self._canvas:
                CheckersBoard._create_selection(self._canvas, square, self._square_size(self._canvas))
                self._draw_tiles(self._canvas)

            return True

        return False

    def _play_normal_move(self, move: Move):
        assert move.type() == MoveType.Normal
        assert self._board[move.normal().destination_index] == _Square.None_

        CheckersBoard._swap(self._board, move.normal().source_index, move.normal().destination_index)

        advancement = not self._board[move.normal().destination_index].value & (1 << 2)

        self._check_piece_crowning(move.normal().destination_index)
        self._check_forty_move_rule(advancement)
        self._check_repetition(advancement)
        self._change_turn()
        self._check_legal_moves()  # This sets game over and has the highest priority

        if self._on_piece_move is not None:
            self._on_piece_move(move)

        if self._redraw and self._canvas:
            self._draw_pieces(self._canvas)

        self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)

    def _play_capture_move(self, move: Move):
        assert move.type() == MoveType.Capture
        assert self._board[move.capture().destination_indices[-1]] == _Square.None_ or move.capture().source_index == move.capture().destination_indices[-1]

        CheckersBoard._remove_piece(self._board, move.capture().source_index, move.capture().source_index, move.capture().destination_indices[0])

        for i in range(len(move.capture().destination_indices) - 1):
            CheckersBoard._remove_piece(self._board, move.capture().source_index, move.capture().destination_indices[i], move.capture().destination_indices[i + 1])

        CheckersBoard._swap(self._board, move.capture().source_index, move.capture().destination_indices[-1])

        self._check_piece_crowning(move.capture().destination_indices[-1])
        self._check_forty_move_rule(True)
        self._check_repetition(True)
        self._change_turn()
        self._check_legal_moves()  # This sets game over and has the highest priority

        if self._on_piece_move is not None:
            self._on_piece_move(move)

        if self._redraw and self._canvas:
            self._draw_pieces(self._canvas)

        self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)

    def _select_jump_square(self, square: int):
        # A piece may jump on a square twice at most

        count = self._jump_squares.count(square)

        if count < 2:
            self._jump_squares.append(square)

        if self._redraw and self._canvas:
            self._draw_tiles(self._canvas)

    def _check_piece_crowning(self, destination_index: int):
        row = destination_index // 4

        match self._turn:
            case Player.Black:
                if row == 0:
                    self._board[destination_index] = _Square.BlackKing
            case Player.White:
                if row == 7:
                    self._board[destination_index] = _Square.WhiteKing

    def _change_turn(self):
        match self._turn:
            case Player.Black:
                self._turn = Player.White
            case Player.White:
                self._turn = Player.Black

    def _check_forty_move_rule(self, advancement: bool):
        if advancement:
            self._plies_without_advancement = 0
        else:
            self._plies_without_advancement += 1

            if self._plies_without_advancement == 80:
                self._game_over = GameOver.TieBetweenBothPlayers

    def _check_repetition(self, advancement: bool):
        current = _Position(copy.copy(self._board), self._turn)

        if advancement:
            self._repetition_positions.clear()
        else:
            if self._repetition_positions.count(current) == 2:
                self._game_over = GameOver.TieBetweenBothPlayers
                return

        # Insert current position even after advancement
        self._repetition_positions.append(current)

    def _check_legal_moves(self):
        # Must be called after changing the turn

        moves = CheckersBoard._generate_moves(self._board, self._turn)

        if not moves:
            # Either they have no pieces left or they are blocked
            self._game_over = GameOver.WinnerWhite if self._turn == Player.Black else GameOver.WinnerBlack

    def _clear(self):
        self._board.clear()
        self._turn = Player.Black
        self._plies_without_advancement = 0
        self._repetition_positions.clear()

        self._legal_moves.clear()
        self._selected_piece_square = NULL_INDEX
        self._jump_squares.clear()
        self._game_over = GameOver.None_

        if self._redraw and self._canvas:
            self._canvas.delete("pieces")
            self._canvas.delete("selection")
            self._canvas.delete("tiles")

    def _square_size(self, canvas: tk.Canvas) -> float:
        return float(canvas["width"]) / 8.0

    def _draw_pieces(self, canvas: tk.Canvas):
        canvas.delete("pieces")
        canvas.tag_raise("tiles")

        for i, square in enumerate(self._board):
            match square:
                case _Square.None_:
                    pass
                case _Square.Black:
                    CheckersBoard._create_piece(canvas, i, self._square_size(canvas), self.PIECE_BLACK, False)
                case _Square.BlackKing:
                    CheckersBoard._create_piece(canvas, i, self._square_size(canvas), self.PIECE_BLACK, True)
                case _Square.White:
                    CheckersBoard._create_piece(canvas, i, self._square_size(canvas), self.PIECE_WHITE, False)
                case _Square.WhiteKing:
                    CheckersBoard._create_piece(canvas, i, self._square_size(canvas), self.PIECE_WHITE, True)

        canvas.tag_raise("indices")

    def _draw_tiles(self, canvas: tk.Canvas):
        canvas.delete("tiles")

        for move in self._legal_moves:
            match move.type():
                case MoveType.Normal:
                    if move.normal().source_index == self._selected_piece_square:
                        CheckersBoard._create_tile(canvas, move.normal().destination_index, self._square_size(canvas), self.PINK)
                case MoveType.Capture:
                    if move.capture().source_index == self._selected_piece_square:
                        for index in move.capture().destination_indices:
                            CheckersBoard._create_tile(canvas, index, self._square_size(canvas), self.PINK)

        for square in self._jump_squares:
            color = self.INDIGO if self._jump_squares.count(square) < 2 else self.DARKER_INDIGO

            CheckersBoard._create_tile(canvas, square, self._square_size(canvas), color)

        canvas.tag_raise("indices")

    @staticmethod
    def _playable_normal_move(move: Move, source_square: int, destination_square: int) -> bool:
        if move.type() != MoveType.Normal:
            return False

        if move.normal().source_index != source_square:
            return False

        if move.normal().destination_index != destination_square:
            return False

        return True

    @staticmethod
    def _playable_capture_move(move: Move, source_square: int, squares: list[int]) -> bool:
        if move.type() != MoveType.Capture:
            return False

        if move.capture().source_index != source_square:
            return False

        if move.capture().destination_indices != squares:
            return False

        return True

    @staticmethod
    def _create_piece(canvas: tk.Canvas, square: int, square_size: float, color: str, king: bool):
        x0, y0, x1, y1 = CheckersBoard._piece_coordinates(square, square_size, 1.3)

        canvas.create_oval(x0, y0, x1, y1, fill=color, outline=color, tags=("all", "pieces"))

        x0, y0, x1, y1 = CheckersBoard._piece_coordinates(square, square_size, 2.4)

        if king:
            canvas.create_oval(x0, y0, x1, y1, fill=CheckersBoard.GOLD, outline=CheckersBoard.GOLD, tags=("all", "pieces"))

    @staticmethod
    def _create_selection(canvas: tk.Canvas, square: int, square_size: float):
        x0, y0, x1, y1 = CheckersBoard._piece_coordinates(square, square_size, 1.22)

        canvas.delete("selection")

        canvas.create_oval(x0, y0, x1, y1, fill=CheckersBoard.PINK, outline=CheckersBoard.PINK, tags=("all", "selection"))

        canvas.tag_raise("pieces")
        canvas.tag_raise("indices")

    @staticmethod
    def _create_tile(canvas: tk.Canvas, square: int, square_size: float, color: str):
        file, rank = CheckersBoard._get_square(common._1_32_to_0_64(common._0_31_to_1_32(square)))

        canvas.create_rectangle(
            file * square_size,
            rank * square_size,
            file * square_size + square_size,
            rank * square_size + square_size,
            fill=color,
            outline=color,
            tags=("all", "tiles")
        )

    @staticmethod
    def _piece_coordinates(square: int, square_size: float, division: float) -> tuple[float, float, float, float]:
        file, rank = CheckersBoard._get_square(common._1_32_to_0_64(common._0_31_to_1_32(square)))

        piece_size = square_size / division
        offset = (square_size - piece_size) / 2.0

        return (
            file * square_size + offset,
            rank * square_size + offset,
            file * square_size + piece_size + offset,
            rank * square_size + piece_size + offset
        )

    @staticmethod
    def _get_jumped_piece_index(index1: int, index2: int) -> int:
        # This works with indices in the range [1, 32]

        sum = index1 + index2

        assert sum % 2 == 1

        if (common._1_32_to_0_31(index1) // 4) % 2 == 0:
            return (sum + 1) // 2
        else:
            return (sum - 1) // 2

    @staticmethod
    def _remove_piece(board: _Board, source_index: int, index1: int, index2: int):
        # This assert doesn't always make sense to verify, but when it's not needed, it won't be false
        assert board[index2] == _Square.None_ or index2 == source_index

        index = CheckersBoard._get_jumped_piece_index(
            common._0_31_to_1_32(index1),
            common._0_31_to_1_32(index2)
        )

        assert board[common._1_32_to_0_31(index)] != _Square.None_

        board[common._1_32_to_0_31(index)] = _Square.None_

    @staticmethod
    def _generate_moves(board: _Board, player: Player) -> list[Move]:
        moves: list[Move] = []

        for i in range(32):
            king = bool(board[i].value & (1 << 2))
            piece = bool(board[i].value & player.value)

            if piece:
                CheckersBoard._generate_piece_capture_moves(board, moves, i, player, king)

        # If there are possible captures, force the player to play these moves
        if moves:
            return moves

        for i in range(32):
            king = bool(board[i].value & (1 << 2))
            piece = bool(board[i].value & player.value)

            if piece:
                CheckersBoard._generate_piece_moves(board, moves, i, player, king)

        return moves

    @staticmethod
    def _generate_piece_capture_moves(board: _Board, moves: list[Move], square_index: int, player: Player, king: bool):
        # Board must not be modified here
        # The context is local to these function calls
        ctx = _JumpCtx(copy.copy(board), square_index, [])

        # Call recursively
        CheckersBoard._check_piece_jumps(moves, square_index, player, king, ctx)

    @staticmethod
    def _generate_piece_moves(board: _Board, moves: list[Move], square_index: int, player: Player, king: bool):
        directions = CheckersBoard._get_directions(player, king)

        # Check the squares above or below in diagonal
        for direction in directions:
            target_index = CheckersBoard._offset_index(square_index, direction, _Diagonal.Short)

            if target_index == NULL_INDEX:
                continue

            if board[target_index] != _Square.None_:
                continue

            moves.append(Move(Move._Normal(square_index, target_index)))

    @staticmethod
    def _check_piece_jumps(moves: list[Move], square_index: int, player: Player, king: bool, ctx: _JumpCtx) -> bool:
        directions = CheckersBoard._get_directions(player, king)

        # We want an enemy piece
        piece_mask = CheckersBoard._opponent(player).value

        sequence_jumps_ended = True

        for direction in directions:
            enemy_index = CheckersBoard._offset_index(square_index, direction, _Diagonal.Short)
            target_index = CheckersBoard._offset_index(square_index, direction, _Diagonal.Long)

            if enemy_index == NULL_INDEX or target_index == NULL_INDEX:
                continue

            is_enemy_piece = ctx.board[enemy_index].value & piece_mask

            if not is_enemy_piece or ctx.board[target_index] != _Square.None_:
                continue

            sequence_jumps_ended = False

            ctx.destination_indices.append(target_index)

            # Remove the piece to avoid illegal jumps
            removed_enemy_piece = CheckersBoard._exchange(ctx.board, enemy_index, _Square.None_)

            # Jump this piece to avoid other illegal jumps
            CheckersBoard._swap(ctx.board, square_index, target_index)

            if CheckersBoard._check_piece_jumps(moves, target_index, player, king, ctx):
                # This means that it reached the end of a sequence of jumps; the piece can't jump anymore

                moves.append(Move(Move._Capture(ctx.source_index, copy.copy(ctx.destination_indices))))

            # Restore jumped piece
            CheckersBoard._swap(ctx.board, square_index, target_index)

            # Restore removed piece
            CheckersBoard._exchange(ctx.board, enemy_index, removed_enemy_piece)

            ctx.destination_indices.pop()

        return sequence_jumps_ended

    @staticmethod
    def _offset_index(square_index: int, direction: _Direction, diagonal: _Diagonal) -> int:
        result_index = square_index

        even_row = (square_index // 4) % 2 == 0

        match direction:
            case _Direction.NorthEast:
                result_index -= 3 if even_row else 4

                if diagonal == _Diagonal.Long:
                    result_index -= 4 if even_row else 3
            case _Direction.NorthWest:
                result_index -= 4 if even_row else 5

                if diagonal == _Diagonal.Long:
                    result_index -= 5 if even_row else 4
            case _Direction.SouthEast:
                result_index += 5 if even_row else 4

                if diagonal == _Diagonal.Long:
                    result_index += 4 if even_row else 5
            case _Direction.SouthWest:
                result_index += 4 if even_row else 3

                if diagonal == _Diagonal.Long:
                    result_index += 3 if even_row else 4

        # Check edge cases (literally)
        if abs(square_index // 4 - result_index // 4) != diagonal.value:
            return NULL_INDEX

        # Check out of bounds
        if result_index < 0 or result_index > 31:
            return NULL_INDEX

        return result_index

    @staticmethod
    def _get_directions(player: Player, king: bool) -> list[_Direction]:
        directions = []

        if king:
            directions.append(_Direction.NorthEast)
            directions.append(_Direction.NorthWest)
            directions.append(_Direction.SouthEast)
            directions.append(_Direction.SouthWest)
        else:
            match player:
                case Player.Black:
                    directions.append(_Direction.NorthEast)
                    directions.append(_Direction.NorthWest)
                case Player.White:
                    directions.append(_Direction.SouthEast)
                    directions.append(_Direction.SouthWest)

        return directions

    @staticmethod
    def _opponent(player: Player) -> Player:
        match player:
            case Player.Black:
                return Player.White
            case Player.White:
                return Player.Black

    @staticmethod
    def _exchange(board: _Board, index: int, new_value: _Square) -> _Square:
        old_value = board[index]
        board[index] = new_value
        return old_value

    @staticmethod
    def _swap(board: _Board, index1: int, index2: int):
        old_value = board[index1]
        board[index1] = board[index2]
        board[index2] = old_value

    @staticmethod
    def _valid_position_string(string: str) -> bool:
        return common.validate_position_string(string)

    @staticmethod
    def _parse_position_string(string: str) -> _Position:
        turn, player1, player2 = string.split(":")

        pieces1 = CheckersBoard._parse_player_pieces(player1)
        pieces2 = CheckersBoard._parse_player_pieces(player2)

        if player1[0] == player2[0]:
            raise RuntimeError(f"Invalid player types: {player1, player2}")

        board = _Board()

        for index, square in pieces1:
            board[common._1_32_to_0_31(index)] = square

        for index, square in pieces2:
            board[common._1_32_to_0_31(index)] = square

        return _Position(board, CheckersBoard._parse_player_type(turn))

    @staticmethod
    def _parse_player_type(string: str) -> Player:
        match string:
            case "B":
                return Player.Black
            case "W":
                return Player.White

        raise RuntimeError(f"Invalid player type: {string}")

    @staticmethod
    def _parse_player_pieces(string: str) -> list[tuple[int, _Square]]:
        player_type = CheckersBoard._parse_player_type(string[0])

        return list(map(
            lambda piece: CheckersBoard._parse_player_piece(piece, player_type),
            string[1:].split(",")
        ))

    @staticmethod
    def _parse_player_piece(string: str, player_type: Player) -> tuple[int, _Square]:
        king = string[0] == "K"

        match player_type:
            case Player.Black:
                if king:
                    return (int(string[1:]), _Square.BlackKing)
                else:
                    return (int(string), _Square.Black)
            case Player.White:
                if king:
                    return (int(string[1:]), _Square.WhiteKing)
                else:
                    return (int(string), _Square.White)

    @staticmethod
    def _valid_move_string(string: str) -> bool:
        return re.match("([0-9]+(x|-))+[0-9]+", string) is not None

    @staticmethod
    def _parse_move_string(string: str) -> Move:
        source_index, *destination_indices = CheckersBoard._parse_squares(string)

        if CheckersBoard._is_capture_move(source_index, destination_indices[0]):
            move = Move(Move._Capture(common._1_32_to_0_31(source_index), list(map(common._1_32_to_0_31, destination_indices))))
        else:
            move = Move(Move._Normal(common._1_32_to_0_31(source_index), common._1_32_to_0_31(destination_indices[0])))

        return move

    @staticmethod
    def _parse_squares(string: str) -> list[int]:
        tokens = string.replace("-", "x").split("x")

        squares = [int(token) for token in tokens]

        for square in squares:
            if not 1 <= square <= 32:
                raise RuntimeError(f"Invalid square: {square}")

        return squares

    @staticmethod
    def _is_capture_move(source_index: int, destination_index: int) -> bool:
        # Indices must be in the range [1, 32]

        distance = abs(common._1_32_to_0_31(source_index) - common._1_32_to_0_31(destination_index))

        if 3 <= distance <= 5:
            return False
        elif distance == 7 or distance == 9:
            return True
        else:
            assert False

    @staticmethod
    def _is_black_square(square: int) -> bool:
        x, y = CheckersBoard._get_square(square)

        if (x + y) % 2 == 0:
            return False
        else:
            return True

    @staticmethod
    def _get_square(square: int) -> tuple[int, int]:
        file = square % 8
        rank = square // 8

        return (file, rank)
