import enum

from common import checkers_engine
from common import board
from common import common
from . import error
from .print import *

type QueriedParams = list[tuple[str, str, str]]


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


def start_engine(engine: checkers_engine.CheckersEngine, engine_file_path: str, color: Color):
    try:
        engine.start(engine_file_path)
    except checkers_engine.CheckersEngineError as err:
        raise error.ComparatorError(f"Could not start engine: {err}")

    result = common.wait_for_engine_to_start(engine, 3.0)

    if not result[0]:
        raise error.ComparatorError(f"Engine did not respond: {result[1]}")

    print_status(f"Started engine {color}", 1)


def initialize_engine(engine: checkers_engine.CheckersEngine, color: Color) -> QueriedParams:
    try:
        engine.send("INIT")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(err)

    print_status(f"Engine {color} parameters", 1)

    parameters = get_engine_parameters(engine)

    for name, type, value in parameters:
        print_status(f"{name}: {type} = {value}", 2)

    return parameters


def finalize_engine(engine: checkers_engine.CheckersEngine, color: Color):
    try:
        engine.send("QUIT")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(err)

    engine.stop()

    print_status(f"Stopped engine {color}", 1)


def get_engine_parameters(engine: checkers_engine.CheckersEngine) -> QueriedParams:
    try:
        engine.send("GETPARAMETERS")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(err)

    return _wait_for_engine_parameters(engine)


def setup_engine_parameters(engine: checkers_engine.CheckersEngine, parameters: list[str], queried_params: QueriedParams, color: Color):
    for parameter in parameters:
        tokens = parameter.split("=")

        if len(tokens) != 2:
            continue

        name = tokens[0]
        value = tokens[1]

        if name not in [param[0] for param in queried_params]:
            print_status(f"Parameter `{name}` is invalid for engine {color}", 1)
            continue

        try:
            engine.send(f"SETPARAMETER {name} {value}")
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise error.ComparatorError(err)

        print_status(f"Set parameter `{name}` to `{value}` for engine {color}", 1)


def setup_engine_board(engine: checkers_engine.CheckersEngine, position: str):
    try:
        engine.send(f"NEWGAME {position}")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(err)


def play_engine_move(engine_current: checkers_engine.CheckersEngine, engine_next: checkers_engine.CheckersEngine, local_board: board.CheckersBoard) -> tuple[str | None, bool]:
    # Return the played move and return true if the game is over

    try:
        engine_current.send("GO")
    except checkers_engine.CheckersEngineError as err:
        engine_current.stop(True)
        raise error.ComparatorError(err)

    result = _wait_for_engine_move(engine_current)

    if result is None:
        try:
            engine_next.send("GO")
        except checkers_engine.CheckersEngineError as err:
            engine_next.stop(True)
            raise error.ComparatorError(err)

        if _wait_for_engine_move(engine_next) is not None:
            raise error.ComparatorError("One engine says it's game over, but the other doesn't agree")

        if local_board.get_game_over() == board.GameOver.None_:
            raise error.ComparatorError("The engines say it's game over, but the GUI doesn't agree")

        return result, True

    try:
        engine_next.send(f"MOVE {result}")
    except checkers_engine.CheckersEngineError as err:
        engine_next.stop(True)
        raise error.ComparatorError(err)

    local_board.play_move(result)

    return result, False


def get_engine_name(engine: checkers_engine.CheckersEngine) -> str:
    try:
        engine.send("GETNAME")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(err)

    return _wait_for_engine_name(engine)


def _wait_for_engine_parameters(engine: checkers_engine.CheckersEngine) -> QueriedParams:
    parameters = []

    while True:
        try:
            message = engine.receive()
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise error.ComparatorError(err)

        if "PARAMETERS" in message:
            tokens = message.split()
            token_index = 1

            while token_index < len(tokens):
                name = tokens[token_index]

                try:
                    engine.send(f"GETPARAMETER {name}")
                except checkers_engine.CheckersEngineError as err:
                    engine.stop(True)
                    raise error.ComparatorError(err)

                type, value = _wait_for_engine_parameter(engine)
                token_index += 1

                parameters.append((name, type, value))

            break

    return parameters


def _wait_for_engine_parameter(engine: checkers_engine.CheckersEngine) -> tuple[str, str]:
    while True:
        try:
            message = engine.receive()
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise error.ComparatorError(err)

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
            raise error.ComparatorError(err)

        if "BESTMOVE" in message:
            if "none" in message:
                return None
            else:
                return message.split()[1]


def _wait_for_engine_name(engine: checkers_engine.CheckersEngine) -> str:
    while True:
        try:
            message = engine.receive()
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise error.ComparatorError(err)

        if "NAME" in message:
            return message.split()[1]
