import dataclasses
import json
import time
import statistics

import jsonschema

from common import checkers_engine
from common import board
from common import common
from . import engine_control
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


def run_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    if not match_file.positions:
        raise error.ComparatorError("No positions provided")

    report = _run_multiple_rounds_match(match_file, path_engine_black, path_engine_white)

    try:
        data.generate_report(report)
    except data.DataError as err:
        raise error.ComparatorError(f"Could not generate report: {err}")

    print_status(f"Generated report at {report.datetime}")


def _run_multiple_rounds_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str) -> data.MatchReport:
    print_status("Running match")

    for position in match_file.positions:
        if not common.validate_position_string(position):
            raise error.ComparatorError(f"Invalid position string `{position}`")

    black_engine = checkers_engine.CheckersEngine()
    white_engine = checkers_engine.CheckersEngine()

    try:
        engine_control.start_engine(black_engine, path_engine_black, engine_control.Color.Black)
        engine_control.start_engine(white_engine, path_engine_white, engine_control.Color.White)

        black_queried_params = engine_control.initialize_engine(black_engine, engine_control.Color.Black)
        white_queried_params = engine_control.initialize_engine(white_engine, engine_control.Color.White)

        engine_control.setup_engine_parameters(black_engine, match_file.black_engine_parameters, black_queried_params, engine_control.Color.Black)
        engine_control.setup_engine_parameters(white_engine, match_file.white_engine_parameters, white_queried_params, engine_control.Color.White)

        black_engine_stats = _engine_stats(path_engine_black, black_engine)
        white_engine_stats = _engine_stats(path_engine_white, white_engine)

        match_results: list[data.RoundResult] = []
        rematch_results: list[data.RoundResult] = []

        for i, position in enumerate(match_file.positions):
            match_results.append(_run_round(position, black_engine, white_engine, i))

        for i, position in enumerate(match_file.positions):
            rematch_results.append(_run_round(position, white_engine, black_engine, i, True))

        engine_control.finalize_engine(black_engine, engine_control.Color.Black)
        engine_control.finalize_engine(white_engine, engine_control.Color.White)
    except KeyboardInterrupt:
        engine_control.finalize_engine(black_engine, engine_control.Color.Black)
        engine_control.finalize_engine(white_engine, engine_control.Color.White)
        raise
    except error.ComparatorError:  # If one engine fails, the other one might still be up and running
        black_engine.stop(True)
        white_engine.stop(True)
        raise

    win_black = lambda round: round.ending == data.RoundEnding.WinnerBlack
    win_white = lambda round: round.ending == data.RoundEnding.WinnerWhite
    tie = lambda round: round.ending == data.RoundEnding.TieBetweenBothPlayers

    return data.MatchReport(
        black_engine_stats,
        white_engine_stats,
        len(match_results) + len(rematch_results),
        sum(map(win_black, match_results)) + sum(map(win_white, rematch_results)),
        sum(map(win_white, match_results)) + sum(map(win_black, rematch_results)),
        sum(map(tie, match_results)) + sum(map(tie, rematch_results)),
        statistics.mean([round.time for round in match_results] + [round.time for round in rematch_results]),
        statistics.mean([len(round.played_moves) for round in match_results] + [len(round.played_moves) for round in rematch_results]),
        0.0,  # TODO
        0.0,
        match_results,
        rematch_results,
        time.ctime()
    )


def _run_round(position: str, black_engine: checkers_engine.CheckersEngine, white_engine: checkers_engine.CheckersEngine, index: int, rematch: bool = False) -> data.RoundResult:
    local_board = board.CheckersBoard(None, None)  # Used to follow the game of the two engines
    local_board.reset(position)

    engine_control.setup_engine_board(black_engine, position)
    engine_control.setup_engine_board(white_engine, position)

    moves_played: list[str] = []

    match position.split(":")[0]:
        case "B":
            current_player = black_engine
            next_player = white_engine
        case "W":
            current_player = white_engine
            next_player = black_engine

    print_status(f"Begin {"rematch round" if rematch else "round"} {index + 1}...", 1, " ")

    begin = time.time()

    while True:
        move, game_over = engine_control.play_engine_move(current_player, next_player, local_board)

        if move is not None:
            moves_played.append(move)

        if game_over:
            break

        current_player, next_player = next_player, current_player

    end = time.time()

    print_status("done")

    match local_board.get_game_over():
        case board.GameOver.None_:
            assert False
        case board.GameOver.WinnerBlack:
            print_status(f"Engine {"white" if rematch else "black"} won the game (color black)", 2)
            ending = data.RoundEnding.WinnerBlack
        case board.GameOver.WinnerWhite:
            print_status(f"Engine {"black" if rematch else "white"} won the game (color white)", 2)
            ending = data.RoundEnding.WinnerWhite
        case board.GameOver.TieBetweenBothPlayers:
            print_status("Tie between both players", 2)
            ending = data.RoundEnding.TieBetweenBothPlayers

    return data.RoundResult(index, position, ending, len(moves_played), moves_played, end - begin)


def _engine_stats(file_path: str, engine: checkers_engine.CheckersEngine) -> data.EngineStats:
    name = engine_control.get_engine_name(engine)
    queried_params = engine_control.get_engine_parameters(engine)

    return data.EngineStats(name, queried_params)
