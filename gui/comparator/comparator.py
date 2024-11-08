import dataclasses
import json
import time

import jsonschema

from common import checkers_engine
from common import board
from common import common
from . import engine_setup
from . import data
from . import error
from .print import *


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
        raise error.ComparatorError(f"Could not parse JSON file: {err}")

    try:
        jsonschema.validate(obj, SCHEMA)
    except jsonschema.ValidationError as err:
        raise error.ComparatorError(f"Invalid JSON file: {err}")

    return MatchFile(obj["positions"], obj["black_engine_parameters"], obj["white_engine_parameters"])


def run(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    if not match_file.positions:
        raise error.ComparatorError("No positions provided")

    if len(match_file.positions) == 1:
        report = _single_match(match_file, path_engine_black, path_engine_white)
    else:
        report = _multiple_matches(match_file, path_engine_black, path_engine_white)

    try:
        data.generate_report(report)
    except data.DataError as err:
        raise error.ComparatorError(f"Could not generate report: {err}")


def _single_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str) -> data.SingleMatchReport:
    assert len(match_file.positions) == 1

    print_status("Running single match")

    position = match_file.positions[0]

    if not common.validate_position_string(position):
        raise error.ComparatorError("Invalid position string")

    black_engine = checkers_engine.CheckersEngine()
    white_engine = checkers_engine.CheckersEngine()

    try:
        engine_setup.start_engine(black_engine, path_engine_black, engine_setup.Color.Black)
        engine_setup.start_engine(white_engine, path_engine_white, engine_setup.Color.White)

        black_queried_params = engine_setup.initialize_engine(black_engine, engine_setup.Color.Black)
        white_queried_params = engine_setup.initialize_engine(white_engine, engine_setup.Color.White)

        engine_setup.setup_engine_parameters(black_engine, match_file.black_engine_parameters, black_queried_params, engine_setup.Color.Black)
        engine_setup.setup_engine_parameters(white_engine, match_file.white_engine_parameters, white_queried_params, engine_setup.Color.White)

        black_engine_stats = _engine_stats(path_engine_black, black_engine)
        white_engine_stats = _engine_stats(path_engine_white, white_engine)

        match_result = _run_match(position, black_engine, white_engine)
        rematch_result = _run_match(position, white_engine, black_engine, True)

        engine_setup.finalize_engine(black_engine, engine_setup.Color.Black)
        engine_setup.finalize_engine(white_engine, engine_setup.Color.White)
    except (error.ComparatorError, KeyboardInterrupt):  # If one engine fails, the other one might still be up and running, or the user might quit
        if black_engine.running():
            black_engine.stop(True)

        if white_engine.running():
            white_engine.stop(True)

        raise

    return data.SingleMatchReport(black_engine_stats, white_engine_stats, match_result, rematch_result, time.ctime())


def _multiple_matches(match_file: MatchFile, path_engine_black: str, path_engine_white: str) -> data.MultipleMatchesReport:
    print_status("Running multiple matches")


def _run_match(position: str, black_engine: checkers_engine.CheckersEngine, white_engine: checkers_engine.CheckersEngine, rematch: bool = False) -> data.MatchResult:
    local_board = board.CheckersBoard(None, None)  # Used to follow the game of the two engines
    local_board.reset(position)

    engine_setup.setup_engine_board(black_engine, position)
    engine_setup.setup_engine_board(white_engine, position)

    moves_played: list[str] = []

    match position.split(":")[0]:
        case "B":
            current_player = black_engine
            next_player = white_engine
        case "W":
            current_player = white_engine
            next_player = black_engine

    print_status("Begin rematch" if rematch else "Begin match", 1)

    begin = time.time()

    while True:
        move, game_over = engine_setup.play_engine_move(current_player, next_player, local_board)

        if move is not None:
            moves_played.append(move)

        if game_over:
            break

        current_player, next_player = next_player, current_player

    end = time.time()

    print_status("Rematch is over" if rematch else "Match is over", 1)

    match local_board.get_game_over():
        case board.GameOver.None_:
            assert False
        case board.GameOver.WinnerBlack:
            print_status("Black won the game (original white)" if rematch else "Black won the game", 2)
            ending = data.MatchEnding.WinnerBlack
        case board.GameOver.WinnerWhite:
            print_status("White won the game (original black)" if rematch else "White won the game", 2)
            ending = data.MatchEnding.WinnerWhite
        case board.GameOver.TieBetweenBothPlayers:
            print_status("Tie between both players", 2)
            ending = data.MatchEnding.TieBetweenBothPlayers

    return data.MatchResult(position, ending, len(moves_played), moves_played, end - begin)


def _engine_stats(file_path: str, engine: checkers_engine.CheckersEngine) -> data.EngineStats:
    name = engine_setup.get_engine_name(engine)
    queried_params = engine_setup.get_engine_parameters(engine)

    return data.EngineStats(name, queried_params)
