import dataclasses
import json
import time
import statistics

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

    match_runner = _run_single_match if len(match_file.positions) == 1 else _run_multiple_matches
    report = match_runner(match_file, path_engine_black, path_engine_white)

    try:
        data.generate_report(report)
    except data.DataError as err:
        raise error.ComparatorError(f"Could not generate report: {err}")

    print_status(f"Generated report at {report.datetime}")


def _run_single_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str) -> data.SingleMatchReport:
    assert len(match_file.positions) == 1

    print_status("Running single match")

    position = match_file.positions[0]

    if not common.validate_position_string(position):
        raise error.ComparatorError(f"Invalid position string `{position}`")

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
    except KeyboardInterrupt:
        engine_setup.finalize_engine(black_engine, engine_setup.Color.Black)
        engine_setup.finalize_engine(white_engine, engine_setup.Color.White)
        raise
    except error.ComparatorError:  # If one engine fails, the other one might still be up and running
        black_engine.stop(True)
        white_engine.stop(True)
        raise

    return data.SingleMatchReport(black_engine_stats, white_engine_stats, match_result, rematch_result, time.ctime())


def _run_multiple_matches(match_file: MatchFile, path_engine_black: str, path_engine_white: str) -> data.MultipleMatchesReport:
    print_status("Running multiple matches")

    for position in match_file.positions:
        if not common.validate_position_string(position):
            raise error.ComparatorError(f"Invalid position string `{position}`")

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

        matches: list[data.MatchResult] = []
        rematches: list[data.MatchResult] = []

        for position in match_file.positions:
            matches.append(_run_match(position, black_engine, white_engine))

        for position in match_file.positions:
            rematches.append(_run_match(position, white_engine, black_engine, True))

        engine_setup.finalize_engine(black_engine, engine_setup.Color.Black)
        engine_setup.finalize_engine(white_engine, engine_setup.Color.White)
    except KeyboardInterrupt:
        engine_setup.finalize_engine(black_engine, engine_setup.Color.Black)
        engine_setup.finalize_engine(white_engine, engine_setup.Color.White)
        raise
    except error.ComparatorError:  # If one engine fails, the other one might still be up and running
        black_engine.stop(True)
        white_engine.stop(True)
        raise

    win_black = lambda match_or_rematch: match_or_rematch.ending == data.MatchEnding.WinnerBlack
    win_white = lambda match_or_rematch: match_or_rematch.ending == data.MatchEnding.WinnerWhite
    tie = lambda match_or_rematch: match_or_rematch.ending == data.MatchEnding.TieBetweenBothPlayers

    return data.MultipleMatchesReport(
        black_engine_stats,
        white_engine_stats,
        sum(map(win_black, matches)) + sum(map(win_white, rematches)),
        sum(map(win_white, matches)) + sum(map(win_black, rematches)),
        sum(map(tie, matches)) + sum(map(tie, rematches)),
        len(matches) + len(rematches),
        statistics.mean([match.time for match in matches] + [rematch.time for rematch in rematches]),
        statistics.mean([len(match.played_moves) for match in matches] + [len(rematch.played_moves) for rematch in rematches]),
        matches,
        rematches,
        time.ctime()
    )


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

    print_status(f"Begin {"rematch" if rematch else "match"}...", 1, " ")

    begin = time.time()

    while True:
        move, game_over = engine_setup.play_engine_move(current_player, next_player, local_board)

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
            ending = data.MatchEnding.WinnerBlack
        case board.GameOver.WinnerWhite:
            print_status(f"Engine {"black" if rematch else "white"} won the game (color white)", 2)
            ending = data.MatchEnding.WinnerWhite
        case board.GameOver.TieBetweenBothPlayers:
            print_status("Tie between both players", 2)
            ending = data.MatchEnding.TieBetweenBothPlayers

    return data.MatchResult(position, ending, len(moves_played), moves_played, end - begin)


def _engine_stats(file_path: str, engine: checkers_engine.CheckersEngine) -> data.EngineStats:
    name = engine_setup.get_engine_name(engine)
    queried_params = engine_setup.get_engine_parameters(engine)

    return data.EngineStats(name, queried_params)
