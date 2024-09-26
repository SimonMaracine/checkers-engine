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

    def type(self) -> MoveType:
        match self.move:
            case self._Normal:
                return MoveType.Normal
            case self._Capture:
                return MoveType.Captre

    move: _Normal | _Capture


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
    def _generate_moves(board: _Board, turn: Player) -> list[Move]:
        moves = []

        for i in range(32):
            king = board[i].value & (1 << 2)
            piece = board[i].value & turn.value

            if piece:
                CheckersBoard._generate_piece_capture_moves(board, moves, i, turn, king)

        # If there are possible captures, force the player to play these moves
        if moves:
            return moves

        for i in range(32):
            king = board[i].value & (1 << 2)
            piece = board[i].value & turn.value

            if piece:
                CheckersBoard._generate_piece_moves(board, moves, i, turn, king)

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
            removed_enemy_piece = ctx.board[enemy_index]  # FIXME
            ctx.board[enemy_index] = _Square.None_

            # Jump this piece to avoid other illegal jumps
            temporary = ctx.board[square_index]  # FIXME
            ctx.board[square_index] = ctx.board[target_index]
            ctx.board[target_index] = temporary

            if CheckersBoard._check_piece_jumps(moves, target_index, player, king, ctx):
                # This means that it reached the end of a sequence of jumps; the piece can't jump anymore

                moves.append(Move(Move._Capture(ctx.source_index, ctx.destination_indices)))

            # Restore jumped piece
            temporary = ctx.board[square_index]  # FIXME
            ctx.board[square_index] = ctx.board[target_index]
            ctx.board[target_index] = temporary

            # Restore removed piece
            ctx.board[enemy_index] = removed_enemy_piece

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
