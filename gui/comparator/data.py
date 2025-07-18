import json
import enum
import dataclasses
from typing import Any

import jsonschema

from common import saved_game

# https://json-schema.org/understanding-json-schema/reference/


class DataError(RuntimeError):
    pass


class RoundEnding(enum.Enum):
    WinnerBlack = enum.auto()
    WinnerWhite = enum.auto()
    TieBetweenBothPlayers = enum.auto()

    def __str__(self) -> str:
        match self:
            case self.WinnerBlack:
                return "winner color black"
            case self.WinnerWhite:
                return "winner color white"
            case self.TieBetweenBothPlayers:
                return "tie between both players"

        assert False


@dataclasses.dataclass(slots=True, frozen=True)
class EngineStats:
    name: str
    parameters: list[tuple[str, str, str]]


@dataclasses.dataclass(slots=True, frozen=True)
class RoundResult:
    index: int
    position: str
    ending: RoundEnding
    plies: int
    played_moves: list[tuple[str, int]]
    time: float


@dataclasses.dataclass(slots=True, frozen=True)
class MatchReport:
    black_engine: EngineStats
    white_engine: EngineStats
    max_think_time: float
    force_max_think_time: bool
    total_rounds: int
    total_black_engine_wins: int
    total_white_engine_wins: int
    total_draws: int
    average_round_time: float
    average_round_plies: float
    average_black_engine_depth: float
    average_white_engine_depth: float
    match: list[RoundResult]
    rematch: list[RoundResult]
    start_time: str
    end_time: str


def generate_match_report(report: MatchReport):
    assert report.total_black_engine_wins + report.total_white_engine_wins + report.total_draws == report.total_rounds

    obj = {
        "black_engine": {
            "name": report.black_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.black_engine.parameters]
        },

        "white_engine": {
            "name": report.white_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.white_engine.parameters]
        },

        "max_think_time": report.max_think_time,
        "force_max_think_time": report.force_max_think_time,
        "total_rounds": report.total_rounds,
        "total_black_engine_wins": report.total_black_engine_wins,
        "total_white_engine_wins": report.total_white_engine_wins,
        "total_draws": report.total_draws,
        "average_round_time": report.average_round_time,
        "average_round_plies": report.average_round_plies,
        "average_black_engine_depth": float(report.average_black_engine_depth),
        "average_white_engine_depth": float(report.average_white_engine_depth),

        "match": [
            {
                "index": match.index,
                "position": match.position,
                "time": match.time,
                "ending": str(match.ending),
                "plies": match.plies,
                "played_moves": [move[0] for move in match.played_moves]
            }
            for match in report.match
        ],

        "rematch": [
            {
                "index": rematch.index,
                "position": rematch.position,
                "time": rematch.time,
                "ending": f"{rematch.ending} (swapped engines)",
                "plies": rematch.plies,
                "played_moves": [move[0] for move in rematch.played_moves]
            }
            for rematch in report.rematch
        ],

        "start_time": report.start_time,
        "end_time": report.end_time
    }

    _write_report(f"match--{_format_datetime(report.end_time)}.json", obj)


def extract_replay_file(file_path: str, match: str, index: int):
    SCHEMA = {
        "type": "object",
        "properties": {
            "black_engine": {
                "type": "object",
                "properties": {
                    "name": { "type": "string" },
                    "parameters": {
                        "type": "array",
                        "items": { "type": "string" }
                    }
                },
                "required": ["name", "parameters"]
            },
            "white_engine": {
                "type": "object",
                "properties": {
                    "name": { "type": "string" },
                    "parameters": {
                        "type": "array",
                        "items": { "type": "string" }
                    }
                },
                "required": ["name", "parameters"]
            },
            "max_think_time": { "type": "number" },
            "force_max_think_time": { "type": "boolean" },
            "total_rounds": { "type": "integer" },
            "total_black_engine_wins": { "type": "integer" },
            "total_white_engine_wins": { "type": "integer" },
            "total_draws": { "type": "integer" },
            "average_round_time": { "type": "number" },
            "average_round_plies": { "type": "number" },
            "average_black_engine_depth": { "type": "number" },
            "average_white_engine_depth": { "type": "number" },
            "match": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "index": { "type": "integer" },
                        "position": { "type": "string" },
                        "time": { "type": "number" },
                        "ending": { "type": "string" },
                        "plies": { "type": "integer" },
                        "played_moves": {
                            "type": "array",
                            "items": { "type": "string" }
                        }
                    },
                    "required": ["index", "position", "time", "ending", "plies", "played_moves"]
                }
            },
            "rematch": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "index": { "type": "integer" },
                        "position": { "type": "string" },
                        "time": { "type": "number" },
                        "ending": { "type": "string" },
                        "plies": { "type": "integer" },
                        "played_moves": {
                            "type": "array",
                            "items": { "type": "string" }
                        }
                    },
                    "required": ["index", "position", "time", "ending", "plies", "played_moves"]
                }
            },
            "start_time": { "type": "string" },
            "end_time": { "type": "string" }
        },
        "required": [
            "black_engine",
            "white_engine",
            "max_think_time",
            "force_max_think_time",
            "total_rounds",
            "total_black_engine_wins",
            "total_white_engine_wins",
            "total_draws",
            "average_round_time",
            "average_round_plies",
            "average_black_engine_depth",
            "average_white_engine_depth",
            "match",
            "rematch",
            "start_time",
            "end_time"
        ]
    }

    try:
        with open(file_path, "r") as file:
            obj = json.load(file)
    except Exception as err:
        raise DataError(f"Could not parse JSON file: {err}")

    try:
        jsonschema.validate(obj, SCHEMA)
    except jsonschema.ValidationError as err:
        raise DataError(f"Invalid JSON file: {err}")

    try:
        extracted_match = obj[match][index]
    except (KeyError, IndexError) as err:
        raise DataError(f"Invalid match or index: {err}")

    game = saved_game.Game(extracted_match["position"], extracted_match["played_moves"])

    try:
        saved_game.save_game(f"replay--{_format_datetime(obj["end_time"])}--{match}-{index}.json", game)
    except saved_game.SavedGameError as err:
        raise DataError(f"Could not write file: {err}")


def save_parameters(parameters: dict[str, int], datetime: str):
    try:
        with open(f"parameters--{_format_datetime(datetime)}.txt", "w") as file:
            json.dump(parameters, file, indent=4)
    except Exception as err:
        raise DataError(f"Could not write parameters: {err}")


def _write_report(name: str, obj: Any):
    try:
        with open(name, "w") as file:
            json.dump(obj, file, indent=4)
    except Exception as err:
        raise DataError(f"Could not write JSON file: {err}")


def _format_datetime(datetime: str) -> str:
    _, month, day, time, year = datetime.split()
    hour, minute, second = time.split(":")

    return f"{month}-{day}--{hour}-{minute}-{second}--{year}"
