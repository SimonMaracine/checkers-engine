import enum
import time
import dataclasses

from common import checkers_engine
from common import board
from common import common
from . import error
from .print import print_status

type QueriedParams = list[tuple[str, str, str]]


@dataclasses.dataclass(slots=True, frozen=True)
class Move:
    move: str
    depth: int


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
        raise error.ComparatorError(f"Could not start engine {color}: {err}")

    try:
        common.wait_for_engine_to_start(engine, 3.0)
    except common.EngineWaitError as err:
        raise error.ComparatorError(f"Engine {color} did not respond: {err}")

    print_status(f"Started engine {color}", 1)


def initialize_engine(engine: checkers_engine.CheckersEngine, color: Color) -> QueriedParams:
    try:
        engine.send("INIT")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(f"Could not send INIT to engine {color}: {err}")

    print_status(f"Engine {color} parameters", 1)

    parameters = get_engine_parameters(engine, color)

    for name, type, value in parameters:
        print_status(f"{name}: {type} = {value}", 2)

    return parameters


def finalize_engine(engine: checkers_engine.CheckersEngine, color: Color):
    if not engine.running():
        return

    try:
        engine.send("QUIT")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(f"Could not send QUIT to engine {color}: {err}")

    engine.stop()

    print_status(f"Stopped engine {color}", 1)


def get_engine_parameters(engine: checkers_engine.CheckersEngine, color: Color) -> QueriedParams:
    try:
        engine.send("GETPARAMETERS")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(f"Could not send GETPARAMETERS to engine {color}: {err}")

    return _wait_for_engine_parameters(engine, color)


def setup_engine_parameters(engine: checkers_engine.CheckersEngine, parameters: list[str], queried_params: QueriedParams, color: Color):
    for parameter in parameters:
        tokens = parameter.split("=")

        if len(tokens) != 2:
            print_status(f"Invalid parameter `{parameter}` for engine {color}", 1)
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
            raise error.ComparatorError(f"Could not send SETPARAMETER to engine {color}: {err}")

        print_status(f"Set parameter `{name}` to `{value}` for engine {color}", 1)


def setup_engine_board(engine: checkers_engine.CheckersEngine, position: str, color: Color):
    try:
        engine.send(f"NEWGAME {position}")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(f"Could not send NEWGAME to engine {color}: {err}")


def play_engine_move(max_think_time: float, engine_current: checkers_engine.CheckersEngine, engine_next: checkers_engine.CheckersEngine, local_board: board.CheckersBoard, color_current: Color, color_next: Color) -> tuple[Move | None, bool]:
    # Return the played move and return true if the game is over

    try:
        engine_current.send("GO")
    except checkers_engine.CheckersEngineError as err:
        engine_current.stop(True)
        raise error.ComparatorError(f"Could not send GO to engine {color_current}: {err}")

    result = _wait_for_engine_move(engine_current, color_current, max_think_time)

    if result is None:
        try:
            engine_next.send("GO")
        except checkers_engine.CheckersEngineError as err:
            engine_next.stop(True)
            raise error.ComparatorError(f"Could not send GO to engine {color_next}: {err}")

        if _wait_for_engine_move(engine_next, color_next) is not None:
            raise error.ComparatorError("One engine says it's game over, but the other doesn't agree")

        if local_board.get_game_over() == board.GameOver.None_:
            raise error.ComparatorError("The engines say it's game over, but the GUI doesn't agree")

        return None, True

    try:
        engine_next.send(f"MOVE {result.move}")
    except checkers_engine.CheckersEngineError as err:
        engine_next.stop(True)
        raise error.ComparatorError(f"Could not send MOVE to engine {color_next}: {err}")

    local_board.play_move(result.move)

    return result, False


def get_engine_name(engine: checkers_engine.CheckersEngine, color: Color) -> str:
    try:
        engine.send("GETNAME")
    except checkers_engine.CheckersEngineError as err:
        engine.stop(True)
        raise error.ComparatorError(f"Could not send GETNAME to engine {color}: {err}")

    return _wait_for_engine_name(engine, color)


def _wait_for_engine_parameters(engine: checkers_engine.CheckersEngine, color: Color) -> QueriedParams:
    parameters = []

    while True:
        try:
            message = engine.receive(1.0)
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise error.ComparatorError(f"Could not receive message from engine {color}: {err}")

        if "PARAMETERS" in message:
            tokens = message.split()
            token_index = 1

            while token_index < len(tokens):
                name = tokens[token_index]

                try:
                    engine.send(f"GETPARAMETER {name}")
                except checkers_engine.CheckersEngineError as err:
                    engine.stop(True)
                    raise error.ComparatorError(f"Could not send GETPARAMETER to engine {color}: {err}")

                type, value = _wait_for_engine_parameter(engine, color)
                token_index += 1

                parameters.append((name, type, value))

            break

    return parameters


def _wait_for_engine_parameter(engine: checkers_engine.CheckersEngine, color: Color) -> tuple[str, str]:
    while True:
        try:
            message = engine.receive(1.0)
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise error.ComparatorError(f"Could not receive message from engine {color}: {err}")

        if "PARAMETER" in message:
            tokens = message.split()
            type = tokens[2]
            value = tokens[3]

            return type, value


def _wait_for_engine_move(engine: checkers_engine.CheckersEngine, color: Color, max_seconds: float | None = None) -> Move | None:
    # Wait indefinitely, if max_seconds is None

    # If the depth info is missing, it will just return 0 as depth
    max_depth = 0

    begin = time.time()

    while True:
        try:
            message = engine.receive(0.1)
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise error.ComparatorError(f"Could not receive message from engine {color}: {err}")

        if "BESTMOVE" in message:
            if "none" in message:
                return None
            else:
                return Move(message.split()[1], max_depth)
        elif "INFO" in message:
            tokens = message.split()

            for i, token in enumerate(tokens):
                if token == "depth":
                    max_depth = int(tokens[i + 1])

        now = time.time()

        if max_seconds is not None and now - begin > max_seconds:
            try:
                message = engine.send("STOP")
            except checkers_engine.CheckersEngineError as err:
                engine.stop(True)
                raise error.ComparatorError(f"Could not send STOP to engine {color}: {err}")


def _wait_for_engine_name(engine: checkers_engine.CheckersEngine, color: Color) -> str:
    while True:
        try:
            message = engine.receive(1.0)
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            raise error.ComparatorError(f"Could not receive message from engine {color}: {err}")

        if "NAME" in message:
            return message.split()[1]
