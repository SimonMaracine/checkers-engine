"""
    branched capture B:W1,3,8,9,10,16,17:B12,20,21,23,26,27,29,31
    longest capture W:WK4:B6,7,8,14,15,16,22,23,24

    winner white W:WK7,K8:B16
    winner black W:W8:BK12,K15
    winner white B:W1,2,K7:B5,6,15,9
    winner black W:W25,6:BK14,29,30

    tie W:WK2:BK30
"""

import enum
import dataclasses
import re
from typing import Callable

NULL_INDEX = -1


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
            case self._Normal:
                return MoveType.Normal
            case self._Capture:
                return MoveType.Capture

    data: _Normal | _Capture


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


class _Board:
    __slots__ = (
        "_0",
        "_1",
        "_2",
        "_3",
        "_4",
        "_5",
        "_6",
        "_7",
        "_8",
        "_9",
        "_10",
        "_11",
        "_12",
        "_13",
        "_14",
        "_15",
        "_16",
        "_17",
        "_18",
        "_19",
        "_20",
        "_21",
        "_22",
        "_23",
        "_24",
        "_25",
        "_26",
        "_27",
        "_28",
        "_29",
        "_30",
        "_31"
    )

    def __init__(self):
        self._0 = _Square.None_
        self._1 = _Square.None_
        self._2 = _Square.None_
        self._3 = _Square.None_
        self._4 = _Square.None_
        self._5 = _Square.None_
        self._6 = _Square.None_
        self._7 = _Square.None_
        self._8 = _Square.None_
        self._9 = _Square.None_
        self._10 = _Square.None_
        self._11 = _Square.None_
        self._12 = _Square.None_
        self._13 = _Square.None_
        self._14 = _Square.None_
        self._15 = _Square.None_
        self._16 = _Square.None_
        self._17 = _Square.None_
        self._18 = _Square.None_
        self._19 = _Square.None_
        self._20 = _Square.None_
        self._21 = _Square.None_
        self._22 = _Square.None_
        self._23 = _Square.None_
        self._24 = _Square.None_
        self._25 = _Square.None_
        self._26 = _Square.None_
        self._27 = _Square.None_
        self._28 = _Square.None_
        self._29 = _Square.None_
        self._30 = _Square.None_
        self._31 = _Square.None_

    def __getitem__(self, index: int) -> _Square:
        return getattr(self, f"_{index}")

    def __setitem__(self, index: int, value: _Square):
        setattr(self, f"_{index}", value)

    def clear(self):
        for i in range(32):
            setattr(self, f"_{i}", _Square.None_)


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
    def __init__(self, on_piece_move: Callable):
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
        self._on_piece_move = on_piece_move
        self._user_input = False

    def set_user_input(self, user_input: bool):
        self._user_input = user_input

    def press_square(self, square: int):
        if not self._user_input:
            return

        # FIXME check game over?

        if not CheckersBoard._is_black_square(square):
            return

        square = CheckersBoard._1_32_to_0_31(CheckersBoard._0_64_to_1_32(square))

        if CheckersBoard._select_piece(square):
            return

        # If we just deselected
        # if self._selected_piece_square == NULL_INDEX:
        #     return

        normal = all(map(lambda move: move.type() == MoveType.Normal, self._legal_moves))
        capture = all(map(lambda move: move.type() == MoveType.Capture, self._legal_moves))

        if normal:
            for move in self._legal_moves:
                if CheckersBoard._playable_normal_move(move, self._selected_piece_square, square):
                    self._play_normal_move(move)

                    self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)
                    self._selected_piece_square = NULL_INDEX

                    break
        elif capture:
            self._select_jump_square(square)

            for move in self._legal_moves:
                if CheckersBoard._playable_capture_move(move, self._selected_piece_square, self._jump_squares):
                    self._play_capture_move(move)

                    self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)
                    self._selected_piece_square = NULL_INDEX
                    self._jump_squares.clear()

                    break
        else:
            assert False

    def release_square(self, square: int):
        if not self._user_input:
            return

        # FIXME check game over?

        # if self._selected_piece_square == NULL_INDEX:
        #     return

        self._selected_piece_square = NULL_INDEX
        self._jump_squares.clear()

    def reset(self, position: str = None):
        self._clear()

        if position is not None:
            # Validate only the format
            if not CheckersBoard._valid_position_string(position):
                raise RuntimeError(f"Invalid position string: {position}")

            position: _Position = CheckersBoard._parse_position_string(position)

            self._board = position.board
            self._turn = position.turn
        else:
            for i in range(12):
                self._board[i] = _Square.White

            for i in range(20, 32):
                self._board[i] = _Square.Black

        self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)

    def play_move(self, move: str):
        # Validate only the format
        if not CheckersBoard._valid_move_string(move):
            raise RuntimeError(f"Invalid move string: {move}")

        move: Move = CheckersBoard._parse_move_string(move)

        # Play the move
        match move.type():
            case MoveType.Normal:
                self._play_normal_move(move)
            case MoveType.Capture:
                self._play_capture_move(move)

        # FIXME move to somewhere else
        self._legal_moves = CheckersBoard._generate_moves(self._board, self._turn)

    def get_game_over(self) -> GameOver:
        return self._game_over

    def get_turn(self) -> Player:
        return self._turn

    def get_plies_without_advancement(self) -> int:
        return self._plies_without_advancement

    def _select_piece(self, square: int) -> bool:
        if self._board[square] != _Square.None_ and self._selected_piece_square != square:
            self._selected_piece_square = square
            self._jump_squares.clear()

            return True

        return False

    def _play_normal_move(self, move: Move):
        assert move.type() == MoveType.Normal
        assert self._board[move.data.destination_index] == _Square.None_

        CheckersBoard._swap(self.board, move.data.source_index, move.data.destination_index)

        advancement = not self._board[move.data.destination_index].value & (1 << 2)

        self._check_piece_crowning(move.data.destination_index)
        self._check_forty_move_rule(advancement)
        self._check_repetition(advancement)
        self._change_turn()
        self._check_legal_moves()  # This sets game over and has the highest priority

        self._on_piece_move(move)

    def _play_capture_move(self, move: Move):
        assert move.type() == MoveType.Capture
        assert self._board[move.data.destination_indices[-1]] == _Square.None_ or move.data.source_index == move.data.destination_indices[-1]

        CheckersBoard._remove_piece(self._board, move.data.source_index, move.data.source_index, move.data.destination_indices[0])

        for i in range(len(move.data.destination_indices()) - 1):
            CheckersBoard._remove_piece(self._board, move.data.source_index, move.data.destination_indices[i], move.data.destination_indices[i + 1])

        CheckersBoard._swap(self._board, move.data.source_index, move.data.destination_index)

        self._check_piece_crowning(move.data.destination_index)
        self._check_forty_move_rule(True)
        self._check_repetition(True)
        self._change_turn()
        self._check_legal_moves()  # This sets game over and has the highest priority

        self._on_piece_move(move)

    def _select_jump_square(self, square: int):
        # A piece may jump on a square twice at most

        count = self._jump_squares.count(square)

        if count < 2:
            self._jump_squares.append(square)

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
        current = _Position(self._board, self._turn)

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

    @staticmethod
    def _playable_normal_move(move: Move, source_square: int, destination_square: int) -> bool:
        if move.type() != MoveType.Normal:
            return False

        if move.data.source_index != source_square:
            return False

        if move.data.destination_index != destination_square:
            return False

        return True

    @staticmethod
    def _playable_capture_move(move: Move, source_square: int, squares: list[int]) -> bool:
        if move.type() != MoveType.Capture:
            return False

        if move.data.source_index != source_square:
            return False

        if move.data.destination_indices != squares:
            return False

        return True

    @staticmethod
    def _get_jumped_piece_index(index1: int, index2: int) -> int:
        # This works with indices in the range [1, 32]

        sum = index1 + index2

        assert sum % 2 == 1

        if (CheckersBoard._1_32_to_0_31(index1) // 4) % 2 == 0:
            return (sum + 1) // 2
        else:
            return (sum - 1) // 2

    @staticmethod
    def _remove_piece(board: _Board, source_index: int, index1: int, index2: int):
        # This assert doesn't always make sense to verify, but when it's not needed, it won't be false
        assert board[index2] == _Square.None_ or index2 == source_index

        index = CheckersBoard._get_jumped_piece_index(
            CheckersBoard._0_31_to_1_32(index1),
            CheckersBoard._0_31_to_1_32(index2)
        )

        assert board[CheckersBoard._1_32_to_0_31(index)] != _Square.None_

        board[CheckersBoard._1_32_to_0_31(index)] = _Square.None_

    @staticmethod
    def _generate_moves(board: _Board, player: Player) -> list[Move]:
        moves = []

        for i in range(32):
            king = board[i].value & (1 << 2)
            piece = board[i].value & player.value

            if piece:
                CheckersBoard._generate_piece_capture_moves(board, moves, i, player, king)

        # If there are possible captures, force the player to play these moves
        if moves:
            return moves

        for i in range(32):
            king = board[i].value & (1 << 2)
            piece = board[i].value & player.value

            if piece:
                CheckersBoard._generate_piece_moves(board, moves, i, player, king)

        return moves

    @staticmethod
    def _generate_piece_capture_moves(board: _Board, moves: list[Move], square_index: int, player: Player, king: bool):
        # The context is local to these function calls
        ctx = _JumpCtx(board, square_index, [])

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

                moves.append(Move(Move._Capture(ctx.source_index, ctx.destination_indices)))

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
        return re.match("(W|B)(:(W|B)K?[0-9]+(,K?[0-9]+){0,11}){2}", string) is not None

    @staticmethod
    def _parse_position_string(string: str) -> _Position:
        turn, player1, player2 = string.split(":")

        pieces1 = CheckersBoard._parse_player_pieces(player1)
        pieces2 = CheckersBoard._parse_player_pieces(player2)

        if player1 == player2:
            raise RuntimeError(f"Invalid player types: {player1, player2}")

        board = _Board()

        for index, square in pieces1:
            board[index] = square

        for index, square in pieces2:
            board[index] = square

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
        return re.match("([0-9]+x)+[0-9]+", string) is not None

    @staticmethod
    def _parse_move_string(string: str) -> Move:
        source_index, *destination_indices = CheckersBoard._parse_squares(string)

        if CheckersBoard._is_capture_move(source_index, destination_indices[0]):
            move = Move(Move._Capture(CheckersBoard._1_32_to_0_31(source_index), list(map(CheckersBoard._1_32_to_0_31, destination_indices))))
        else:
            move = Move(Move._Normal(CheckersBoard._1_32_to_0_31(source_index), CheckersBoard._1_32_to_0_31(destination_indices[0])))

        return move

    @staticmethod
    def _parse_squares(string: str) -> list[int]:
        tokens = string.split("x")

        squares = [int(token) for token in tokens]

        for square in squares:
            if not 1 <= square <= 32:
                raise RuntimeError(f"Invalid square: {square}")

        return squares

    @staticmethod
    def _is_capture_move(source_index: int, destination_index: int) -> bool:
        # Indices must be in the range [1, 32]

        distance = abs(CheckersBoard._1_32_to_0_31(source_index) - CheckersBoard._1_32_to_0_31(destination_index))

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

    @staticmethod
    def _1_32_to_0_64(index: int) -> int:
        if ((index - 1) // 4) % 2 == 0:
            return index * 2 - 1
        else:
            return (index - 1) * 2

    @staticmethod
    def _0_64_to_1_32(index: int) -> int:
        if index % 2 == 1:
            return (index + 1) // 2
        else:
            return (index // 2) + 1

    @staticmethod
    def _1_32_to_0_31(index: int) -> int:
        return index - 1

    @staticmethod
    def _0_31_to_1_32(index: int) -> int:
        return index + 1
