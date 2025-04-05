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
from .print import print_status, print_status_red, print_status_white, print_status_cyan


@dataclasses.dataclass(slots=True, frozen=True)
class MatchFile:
    positions: list[str]
    black_engine_parameters: list[str]
    white_engine_parameters: list[str]
    max_think_time: float
    force_max_think_time: bool


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
            },
            "max_think_time": { "type": "number" },
            "force_max_think_time": { "type": "boolean" }
        },
        "required": ["positions", "black_engine_parameters", "white_engine_parameters", "max_think_time", "force_max_think_time"]
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

    return MatchFile(
        obj["positions"],
        obj["black_engine_parameters"],
        obj["white_engine_parameters"],
        obj["max_think_time"],
        obj["force_max_think_time"]
    )


def run_improve_session(match_file: MatchFile, path_engine: str):
    if not match_file.positions:
        raise error.ComparatorError("No positions provided")

    for position in match_file.positions:
        if not common.validate_position_string(position):
            raise error.ComparatorError(f"Invalid position string `{position}`")

    start_time = time.ctime()

    print_status(f"Starting improve session at {start_time}")

    black_engine = checkers_engine.CheckersEngine()
    white_engine = checkers_engine.CheckersEngine()  # Parameters are changed first in this one

    try:
        engine_control.start_engine(black_engine, path_engine, engine_control.Color.Black)
        engine_control.start_engine(white_engine, path_engine, engine_control.Color.White)

        black_queried_params = engine_control.initialize_engine(black_engine, engine_control.Color.Black)
        white_queried_params = engine_control.initialize_engine(white_engine, engine_control.Color.White)

        engine_control.setup_engine_parameters(black_engine, match_file.black_engine_parameters, black_queried_params, engine_control.Color.Black)
        engine_control.setup_engine_parameters(white_engine, match_file.white_engine_parameters, white_queried_params, engine_control.Color.White)

        black_engine_stats = _engine_stats(black_engine, engine_control.Color.Black)
        white_engine_stats = _engine_stats(white_engine, engine_control.Color.White)

        if black_engine_stats != white_engine_stats:
            raise error.ComparatorError("Engines start with different parameters")

        print_status(f"Max thinking time: {match_file.max_think_time}", 1)
        print_status(f"Force max thinking time: {match_file.force_max_think_time}", 1)

        win_epsilon = 10 * (len(match_file.positions) * 2) // 100

        print_status(f"Win epsilon: {win_epsilon}", 1)

        current_parameters = dict(map(
            lambda parameter: (parameter[0], int(parameter[2])),
            filter(lambda parameter: parameter[1] == "int", white_engine_stats.parameters)
        ))

        for epoch in range(100):
            print_status(f"Epoch {epoch + 1}", 2)

            some_success = False

            for parameter in current_parameters:
                succeeded = False
                first_no_success = False
                current_int_adjustment = 1

                while True:
                    print_status(f"Checking parameter `{parameter}`, adjustment = {current_int_adjustment}", 3)

                    # Adjust white and compare it to black
                    current_parameters[parameter] += current_int_adjustment
                    _set_parameter(white_engine, parameter, current_parameters[parameter], engine_control.Color.White)

                    if not _run_test(match_file, black_engine, white_engine, win_epsilon):
                        print_status_red(f"Adjustment = {current_int_adjustment} for `{parameter}` was negative", 4)

                        # Undo adjustment
                        current_parameters[parameter] += -current_int_adjustment
                        _set_parameter(white_engine, parameter, current_parameters[parameter], engine_control.Color.White)

                        if succeeded:
                            # Next parameter
                            break

                        if not first_no_success:
                            # Try the other way around
                            current_int_adjustment = -current_int_adjustment
                            first_no_success = True
                            continue

                        # Next parameter
                        break

                    print_status_white(f"Adjustment = {current_int_adjustment} for `{parameter}` was positive", 4)

                    try:
                        data.save_parameters(current_parameters, start_time)
                    except data.DataError as err:
                        raise error.ComparatorError(f"Could not save parameters: {err}")

                    # White won, adjust black too in the same way and try another adjustment
                    _set_parameter(black_engine, parameter, current_parameters[parameter], engine_control.Color.Black)

                    # This direction looks promising
                    succeeded = True
                    some_success = True

            if not some_success:
                # Nothing seems to improve anymore
                break

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

    print_status(f"Stopped improve session at {time.ctime()}")


def run_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    if not match_file.positions:
        raise error.ComparatorError("No positions provided")

    for position in match_file.positions:
        if not common.validate_position_string(position):
            raise error.ComparatorError(f"Invalid position string `{position}`")

    report = _run_multiple_rounds_match(match_file, path_engine_black, path_engine_white)

    try:
        data.generate_match_report(report)
    except data.DataError as err:
        raise error.ComparatorError(f"Could not generate report: {err}")

    print_status(f"Generated report at {report.end_time}")


def _run_multiple_rounds_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str) -> data.MatchReport:
    start_time = time.ctime()

    print_status(f"Starting match at {start_time}")

    black_engine = checkers_engine.CheckersEngine()
    white_engine = checkers_engine.CheckersEngine()

    try:
        engine_control.start_engine(black_engine, path_engine_black, engine_control.Color.Black)
        engine_control.start_engine(white_engine, path_engine_white, engine_control.Color.White)

        black_queried_params = engine_control.initialize_engine(black_engine, engine_control.Color.Black)
        white_queried_params = engine_control.initialize_engine(white_engine, engine_control.Color.White)

        engine_control.setup_engine_parameters(black_engine, match_file.black_engine_parameters, black_queried_params, engine_control.Color.Black)
        engine_control.setup_engine_parameters(white_engine, match_file.white_engine_parameters, white_queried_params, engine_control.Color.White)

        black_engine_stats = _engine_stats(black_engine, engine_control.Color.Black)
        white_engine_stats = _engine_stats(white_engine, engine_control.Color.White)

        print_status(f"Max thinking time: {match_file.max_think_time}", 1)
        print_status(f"Force max thinking time: {match_file.force_max_think_time}", 1)

        match_results: list[data.RoundResult] = []
        rematch_results: list[data.RoundResult] = []

        for i, position in enumerate(match_file.positions):
            match_results.append(_run_round(
                position,
                match_file.max_think_time,
                match_file.force_max_think_time,
                black_engine,
                white_engine,
                i
            ))

        for i, position in enumerate(match_file.positions):
            rematch_results.append(_run_round(
                position,
                match_file.max_think_time,
                match_file.force_max_think_time,
                white_engine,
                black_engine,
                i,
                True
            ))

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

    played_moves_black = lambda round, rematch: round.played_moves[::2] if round.position[0] == ("W" if rematch else "B") else round.played_moves[1::2]
    played_moves_white = lambda round, rematch: round.played_moves[::2] if round.position[0] == ("B" if rematch else "W") else round.played_moves[1::2]

    return data.MatchReport(
        black_engine_stats,
        white_engine_stats,
        match_file.max_think_time,
        match_file.force_max_think_time,
        len(match_results) + len(rematch_results),
        sum(map(win_black, match_results)) + sum(map(win_white, rematch_results)),
        sum(map(win_white, match_results)) + sum(map(win_black, rematch_results)),
        sum(map(tie, match_results)) + sum(map(tie, rematch_results)),
        statistics.mean([round.time for round in match_results] + [round.time for round in rematch_results]),
        statistics.mean([len(round.played_moves) for round in match_results] + [len(round.played_moves) for round in rematch_results]),
        statistics.mean([move[1] for round in match_results for move in played_moves_black(round, False)] + [move[1] for round in rematch_results for move in played_moves_black(round, True)]),
        statistics.mean([move[1] for round in match_results for move in played_moves_white(round, False)] + [move[1] for round in rematch_results for move in played_moves_white(round, True)]),
        match_results,
        rematch_results,
        start_time,
        time.ctime()
    )


def _run_round(
    position: str,
    max_think_time: float,
    force_max_think_time: bool,
    black_engine: checkers_engine.CheckersEngine,
    white_engine: checkers_engine.CheckersEngine,
    index: int,
    rematch: bool = False,
    output: bool = True
) -> data.RoundResult:
    local_board = board.CheckersBoard(None, None)  # Used to follow the game of the two engines
    local_board.reset(position)

    engine_control.setup_engine_board(black_engine, position, engine_control.Color.Black)
    engine_control.setup_engine_board(white_engine, position, engine_control.Color.White)

    played_moves: list[tuple[str, int]] = []

    match position[0]:
        case "B":
            current_player = black_engine
            next_player = white_engine
            current_color = engine_control.Color.Black
            next_color = engine_control.Color.White
        case "W":
            current_player = white_engine
            next_player = black_engine
            current_color = engine_control.Color.White
            next_color = engine_control.Color.Black

    if output:
        print_status(f"Begin {"rematch" if rematch else "match"} round {index + 1}...", 1, " ")

    begin = time.time()

    while True:
        try:
            move, game_over = engine_control.play_engine_move(
                max_think_time,
                force_max_think_time,
                current_player,
                next_player,
                local_board,
                current_color,
                next_color
            )
        except error.ComparatorError:
            print_status(f"\nPlayed these moves {played_moves} on this position {position}", 4)
            raise

        if move is not None:
            played_moves.append((move.move, move.depth))

        if game_over:
            break

        current_player, next_player = next_player, current_player
        current_color, next_color = next_color, current_color

    end = time.time()

    if output:
        print_status("done")

    match local_board.get_game_over():
        case board.GameOver.None_:
            assert False
        case board.GameOver.WinnerBlack:
            if output:
                print_status_red(f"Winner engine {"white" if rematch else "black"} (color black)", 2)
            ending = data.RoundEnding.WinnerBlack
        case board.GameOver.WinnerWhite:
            if output:
                print_status_white(f"Winner engine {"black" if rematch else "white"} (color white)", 2)
            ending = data.RoundEnding.WinnerWhite
        case board.GameOver.TieBetweenBothPlayers:
            if output:
                print_status_cyan("Tie between both players", 2)
            ending = data.RoundEnding.TieBetweenBothPlayers

    return data.RoundResult(index, position, ending, len(played_moves), played_moves, end - begin)


def _engine_stats(engine: checkers_engine.CheckersEngine, color: engine_control.Color) -> data.EngineStats:
    name = engine_control.get_engine_name(engine, color)
    queried_params = engine_control.get_engine_parameters(engine, color)

    print_status(f"Engine {color} name: {name}", 1)

    return data.EngineStats(name, queried_params)


def _set_parameter(engine: checkers_engine.CheckersEngine, parameter: str, value: int, color: engine_control.Color):
    engine_control.set_engine_int_parameter(engine, parameter, value, color)


def _run_test(
    match_file: MatchFile,
    black_engine: checkers_engine.CheckersEngine,
    white_engine: checkers_engine.CheckersEngine,
    win_epsilon: int
) -> bool:
    match_results: list[data.RoundResult] = []
    rematch_results: list[data.RoundResult] = []

    for i, position in enumerate(match_file.positions):
        match_results.append(_run_round(
            position,
            match_file.max_think_time,
            match_file.force_max_think_time,
            black_engine,
            white_engine,
            i,
            False,
            False
        ))

    for i, position in enumerate(match_file.positions):
        rematch_results.append(_run_round(
            position,
            match_file.max_think_time,
            match_file.force_max_think_time,
            white_engine,
            black_engine,
            i,
            True,
            False
        ))

    win_black = lambda round: round.ending == data.RoundEnding.WinnerBlack
    win_white = lambda round: round.ending == data.RoundEnding.WinnerWhite

    black_engine_wins = sum(map(win_black, match_results)) + sum(map(win_white, rematch_results))
    white_engine_wins = sum(map(win_white, match_results)) + sum(map(win_black, rematch_results))

    return white_engine_wins - black_engine_wins >= win_epsilon
