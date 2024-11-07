import time
import re

from . import checkers_engine

WHITE = "#c8c8c8"
BLACK = "#4C3D2F"
DEFAULT_BOARD_SIZE = 400
TXT_STATUS = "Status:"
TXT_PLAYER = "Player:"
TXT_PLIES_WITHOUT_ADVANCEMENT = "Plies without advancement:"


def _1_32_to_0_64(index: int) -> int:
    if ((index - 1) // 4) % 2 == 0:
        return index * 2 - 1
    else:
        return (index - 1) * 2


def _0_64_to_1_32(index: int) -> int:
    if index % 2 == 1:
        return (index + 1) // 2
    else:
        return (index // 2) + 1


def _1_32_to_0_31(index: int) -> int:
    return index - 1


def _0_31_to_1_32(index: int) -> int:
    return index + 1


def wait_for_engine_to_start(engine: checkers_engine.CheckersEngine, wait_time: float) -> tuple[bool, str]:
    time_begin = time.time()

    while True:
        try:
            message = engine.receive()
        except checkers_engine.CheckersEngineError as err:
            engine.stop(True)
            return False, str(err)

        if "READY" in message:
            return True, "Engine started successfully"

        time_now = time.time()

        if time_now - time_begin > wait_time:
            engine.stop(True)
            return False, "Engine failed to respond in a timely manner"


def validate_position_string(string: str) -> bool:
    return re.match("(W|B)(:(W|B)K?[0-9]+(,K?[0-9]+){0,11}){2}", string) is not None
