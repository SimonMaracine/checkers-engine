import enum
import dataclasses
import re

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
        # Validate only the format
        assert CheckersBoard._valid_move_string(move)

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

    def _play_normal_move(self, move: Move):
        assert move.type() == MoveType.Normal
        assert self._board[move.data.destination_index] == _Square.None_

        CheckersBoard._swap(self.board, move.data.source_index, move.data.destination_index)

        advancement = not self._board[move.data.destination_index].value & (1 << 2)

        self._check_piece_crowning(move.data.destination_index)
        self._change_turn()
        self._check_forty_move_rule(advancement)
        self._check_repetition(advancement)
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
        self._change_turn()
        self._check_forty_move_rule(True)
        self._check_repetition(True)
        self._check_legal_moves()  # This sets game over and has the highest priority

        self._on_piece_move(move)

    def _check_piece_crowning(self, destination_index: int):
        pass

    def _change_turn(self):
        match self._turn:
            case Player.Black:
                self._turn = Player.White
            case Player.White:
                self._turn = Player.Black

    def _check_forty_move_rule(self, advancement: bool):
        pass

    def _check_repetition(self, advancement: bool):
        pass

    def _check_legal_moves(self):
        pass

    @staticmethod
    def _get_jumped_piece_index(index1: int, index2: int) -> int:
        # This works with indices in the range [1, 32]

        sum = index1 + index2

        assert sum % 2 == 1

        if (CheckersBoard._to_0_31(index1) // 4) % 2 == 0:
            return (sum + 1) // 2
        else:
            return (sum - 1) // 2

    @staticmethod
    def _remove_piece(board: _Board, source_index: int, index1: int, index2: int):
        # This assert doesn't always make sense to verify, but when it's not needed, it won't be false
        assert board[index2] == _Square.None_ or index2 == source_index

        index = CheckersBoard._get_jumped_piece_index(
            CheckersBoard._to_1_32(index1),
            CheckersBoard._to_1_32(index2)
        )

        assert board[CheckersBoard._to_0_31(index)] != _Square.None_

        board[CheckersBoard._to_0_31(index)] = _Square.None_

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
    def _valid_move_string(string: str) -> bool:
        return re.match("([0-9]+x)+[0-9]+", string) is not None

    @staticmethod
    def _parse_move_string(string: str) -> Move:
        source_index, *destination_indices = CheckersBoard._parse_squares(string)

        if CheckersBoard._is_capture_move(source_index, destination_indices[0]):
            move = Move(Move._Capture(CheckersBoard._to_0_31(source_index), list(map(lambda index: CheckersBoard._to_0_31(index), destination_indices))))
        else:
            move = Move(Move._Normal(CheckersBoard._to_0_31(source_index), CheckersBoard._to_0_31(destination_indices[0])))

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

        distance = abs(CheckersBoard._to_0_31(source_index) - CheckersBoard._to_0_31(destination_index))

        if 3 <= distance <= 5:
            return False
        elif distance == 7 or distance == 9:
            return True
        else:
            assert False

    @staticmethod
    def _translate_1_32_to_0_64(index: int) -> int:
        if ((index - 1) / 4) % 2 == 0:
            return index * 2 - 1
        else:
            return (index - 1) * 2

    @staticmethod
    def _translate_0_64_to_1_32(index: int) -> int:
        if index % 2 == 1:
            return (index + 1) / 2
        else:
            return (index / 2) + 1

    @staticmethod
    def _to_0_31(index: int) -> int:
        return index - 1

    @staticmethod
    def _to_1_32(index: int) -> int:
        return index + 1
