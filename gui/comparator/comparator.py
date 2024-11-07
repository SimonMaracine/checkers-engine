import dataclasses
import json
import enum
import sys

import jsonschema

from common import checkers_engine
from common import board
from common import common

type QueriedParams = dict[str, tuple[str, str]]


class MatchError(RuntimeError):
    pass


class Color(enum.Enum):
    Black = enum.auto()
    White = enum.auto()

    def __str__(self) -> str:
        match self:
            case self.Black:
                return "black"
            case self.White:
                return "white"

        assert False


@dataclasses.dataclass(slots=True, frozen=True)
class MatchFile:
    positions: list[str]
    black_engine_parameters: list[str]
    white_engine_parameters: list[str]


def parse_match_file(file_path: str) -> MatchFile:
    SCHEMA = {
        "type": "object",
        "properties": {
            "positions": {
                "type": "array",
                "items": { "type": "string" }
            },
            "black_engine_parameters": {
                "type": "array",
                "items": { "type": "string" }
            },
            "white_engine_parameters": {
                "type": "array",
                "items": { "type": "string" }
            }
        },
        "required": ["positions", "black_engine_parameters", "white_engine_parameters"]
    }

    try:
        with open(file_path, "r") as file:
            obj = json.load(file)
    except Exception as err:
        raise MatchError(f"Could not parse JSON file: {err}")

    try:
        jsonschema.validate(obj, SCHEMA)
    except jsonschema.ValidationError as err:
        raise MatchError(f"Invalid JSON file: {err}")

    return MatchFile(obj["positions"], obj["black_engine_parameters"], obj["white_engine_parameters"])


def run(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    if not match_file.positions:
        raise MatchError("No positions provided")

    if len(match_file.positions) == 1:
        _single_match(match_file, path_engine_black, path_engine_white)
    else:
        _multiple_matches(match_file, path_engine_black, path_engine_white)


def _single_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    assert len(match_file.positions) == 1

    print("Running single match")

    position = match_file.positions[0]

    if not common.validate_position_string(position):
        raise MatchError("Invalid position string")

    black_engine = _start_engine(path_engine_black)
    white_engine = _start_engine(path_engine_white)

    black_queried_params = _initialize_engine(black_engine, Color.Black)
    white_queried_params = _initialize_engine(white_engine, Color.White)

    _setup_engine_parameters(black_engine, match_file.black_engine_parameters, black_queried_params, Color.Black)
    _setup_engine_parameters(white_engine, match_file.white_engine_parameters, white_queried_params, Color.White)

    _run_match(position, black_engine, white_engine)

    _finalize_engine(black_engine, Color.Black)
    _finalize_engine(white_engine, Color.White)


def _multiple_matches(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    print("Running multiple matches")


def _start_engine(engine_file_path: str) -> checkers_engine.CheckersEngine:
    engine = checkers_engine.CheckersEngine()

    try:
        engine.start(engine_file_path)
    except checkers_engine.CheckersEngineError as err:
        raise MatchError(f"Could not start engine: {err}")

    result = common.wait_for_engine_to_start(engine, 3.0)

    if not result[0]:
        raise MatchError(f"Engine did not respond: {result[1]}")

    print(result[1])

    return engine


def _run_match(position: str, black_engine: checkers_engine.CheckersEngine, white_engine: checkers_engine.CheckersEngine):
    # Used to follow the game of the two engines
    local_board = board.CheckersBoard(None, None)
    local_board.reset(position)

    try:
        black_engine.send(f"NEWGAME {position}")
    except checkers_engine.CheckersEngineError as err:
        black_engine.stop(True)
        raise MatchError(err)

    try:
        white_engine.send(f"NEWGAME {position}")
    except checkers_engine.CheckersEngineError as err:
        white_engine.stop(True)
        raise MatchError(err)

    print("Begin match")

    match position.split(":")[0]:
        case "B":
            current_player = black_engine
            next_player = white_engine
        case "W":
            current_player = white_engine
            next_player = black_engine

    while True:
        if not _play_move(current_player, next_player, local_board):
            break

        current_player, next_player = next_player, current_player

    print("Match is over")

    match local_board.get_game_over():
        case board.GameOver.None_:
            assert False
        case board.GameOver.WinnerBlack:
            print("    Black won the game")
        case board.GameOver.WinnerWhite:
            print("    White won the game")
        case board.GameOver.TieBetweenBothPlayers:
            print("    Tie between both players")


def _play_move(engine_current: checkers_engine.CheckersEngine, engine_next: checkers_engine.CheckersEngine, local_board: board.CheckersBoard) -> bool:
    # Return false if the game is over

    try:
        engine_current.send("GO")
    except checkers_engine.CheckersEngineError as err:
        engine_current.stop(True)
        raise MatchError(err)

    result = _wait_for_engine_move(engine_current)

    if result is None:
        try:
            engine_next.send("GO")
        except checkers_engine.CheckersEngineError as err:
            engine_next.stop(True)
            raise MatchError(err)

        if _wait_for_engine_move(engine_next) is not None:
            raise MatchError("One engine says it's game over, but the other doesn't agree")

        if local_board.get_game_over() == board.GameOver.None_:
            raise MatchError("The engines say it's game over, but the GUI doesn't agree")

        return False

    try:
        engine_next.send(f"MOVE {result}")
    except checkers_engine.CheckersEngineError as err:
        engine_next.stop(True)
        raise MatchError(err)

    local_board.play_move(result)

    return True


def _initialize_engine(engine: checkers_engine.CheckersEngine, color: Color) -> QueriedParams:
    try:
        engine.send("INIT")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise MatchError(err)

    try:
        engine.send("GETPARAMETERS")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise MatchError(err)

    print(f"{str(color).capitalize()} engine parameters:")

    parameters = _wait_for_engine_parameters(engine)

    for name, type_and_value in parameters.items():
        print(f"    {name}: {type_and_value[0]} = {type_and_value[1]}")

    return parameters


def _finalize_engine(engine: checkers_engine.CheckersEngine, color: Color):
    try:
        engine.send("QUIT")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise MatchError(err)

    engine.stop()

    print(f"Stopped engine {color}")


def _setup_engine_parameters(engine: checkers_engine.CheckersEngine, parameters: list[str], queried_params: QueriedParams, color: Color):
    for parameter in parameters:
        tokens = parameter.split("=")

        if len(tokens) != 2:
            continue

        name = tokens[0]
        value = tokens[1]

        if name not in queried_params:
            print(f"Parameter `{name}` is invalid for engine {color}", file=sys.stderr)
            continue

        try:
            engine.send(f"SETPARAMETER {name} {value}")
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise MatchError(err)

        print(f"Set parameter `{name}` to `{value}` for engine {color}")


def _wait_for_engine_parameters(engine: checkers_engine.CheckersEngine) -> QueriedParams:
    parameters = {}

    while True:
        try:
            message = engine.receive()
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise MatchError(err)

        if "PARAMETERS" in message:
            tokens = message.split()
            token_index = 1

            while token_index < len(tokens):
                name = tokens[token_index]

                try:
                    engine.send(f"GETPARAMETER {name}")
                except checkers_engine.CheckersEngineError as err:
                    engine.stop(True)
                    raise MatchError(err)

                type, value = _wait_for_engine_parameter(engine)
                token_index += 1

                parameters[name] = (type, value)

            break

    return parameters


def _wait_for_engine_parameter(engine: checkers_engine.CheckersEngine) -> tuple[str, str]:
    while True:
        try:
            message = engine.receive()
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise MatchError(err)

        if "PARAMETER" in message:
            tokens = message.split()
            type = tokens[2]
            value = tokens[3]

            return type, value


def _wait_for_engine_move(engine: checkers_engine.CheckersEngine) -> str | None:
    while True:
        try:
            message = engine.receive()
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise MatchError(err)

        if "BESTMOVE" in message:
            if "none" in message:
                return None
            else:
                return message.split()[1]
