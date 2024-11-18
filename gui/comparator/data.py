import json
import enum
import dataclasses
from typing import Any

import jsonschema

from common import saved_game

# https://json-schema.org/understanding-json-schema/reference/


class DataError(RuntimeError):
    pass


class MatchEnding(enum.Enum):
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
class MatchResult:
    position: str
    ending: MatchEnding
    plies: int
    played_moves: list[str]
    time: float


@dataclasses.dataclass(slots=True, frozen=True)
class SingleMatchReport:
    black_engine: EngineStats
    white_engine: EngineStats
    match: MatchResult
    rematch: MatchResult
    datetime: str


@dataclasses.dataclass(slots=True, frozen=True)
class MultipleMatchesReport:
    black_engine: EngineStats
    white_engine: EngineStats
    total_black_engine_wins: int
    total_white_engine_wins: int
    total_draws: int
    total_matches: int
    average_time: float
    average_plies: float
    matches: list[MatchResult]
    rematches: list[MatchResult]
    datetime: str


def generate_report(report: SingleMatchReport | MultipleMatchesReport):
    match report:
        case SingleMatchReport():
            _generate_single_match_report(report)
        case MultipleMatchesReport():
            _generate_multiple_matches_report(report)


def extract_replay_file(file_path: str, match: str, index: int | None):
    SCHEMA_SINGLE_MATCH = {
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
            "match": {
                "type": "object",
                "properties": {
                    "position": { "type": "string" },
                    "time": { "type": "number" },
                    "ending": { "type": "string" },
                    "plies": { "type": "integer" },
                    "played_moves": {
                        "type": "array",
                        "items": { "type": "string" }
                    }
                },
                "required": ["position", "time", "ending", "plies", "played_moves"]
            },
            "rematch": {
                "type": "object",
                "properties": {
                    "position": { "type": "string" },
                    "time": { "type": "number" },
                    "ending": { "type": "string" },
                    "plies": { "type": "integer" },
                    "played_moves": {
                        "type": "array",
                        "items": { "type": "string" }
                    }
                },
                "required": ["position", "time", "ending", "plies", "played_moves"]
            },
            "datetime": { "type": "string" },
            "type": { "type": "string" }
        },
        "required": ["black_engine", "white_engine", "match", "rematch"]
    }

    SCHEMA_MULTIPLE_MATCHES = {
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
            "total_black_engine_wins": { "type": "integer" },
            "total_white_engine_wins": { "type": "integer" },
            "total_draws": { "type": "integer" },
            "total_matches": { "type": "integer" },
            "average_time": { "type": "number" },
            "average_plies": { "type": "number" },
            "matches": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "position": { "type": "string" },
                        "time": { "type": "number" },
                        "ending": { "type": "string" },
                        "plies": { "type": "integer" },
                        "played_moves": {
                            "type": "array",
                            "items": { "type": "string" }
                        }
                    },
                    "required": ["position", "time", "ending", "plies", "played_moves"]
                }
            },
            "rematches": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "position": { "type": "string" },
                        "time": { "type": "number" },
                        "ending": { "type": "string" },
                        "plies": { "type": "integer" },
                        "played_moves": {
                            "type": "array",
                            "items": { "type": "string" }
                        }
                    },
                    "required": ["position", "time", "ending", "plies", "played_moves"]
                }
            },
            "datetime": { "type": "string" },
            "type": { "type": "string" }
        },
        "required": ["black_engine", "white_engine", "total_black_engine_wins", "total_white_engine_wins", "total_draws", "total_matches", "average_time", "average_plies", "matches", "rematches"]
    }

    try:
        with open(file_path, "r") as file:
            obj = json.load(file)
    except Exception as err:
        raise DataError(f"Could not parse JSON file: {err}")

    try:
        match obj["type"]:
            case "single":
                schema = SCHEMA_SINGLE_MATCH
            case "multiple":
                schema = SCHEMA_MULTIPLE_MATCHES
    except KeyError as err:
        raise DataError(f"Missing `type` field in JSON: {err}")

    try:
        jsonschema.validate(obj, schema)
    except jsonschema.ValidationError as err:
        raise DataError(f"Invalid JSON file: {err}")

    match_or_group = obj[match]

    if index is not None:
        extracted_match = match_or_group[index]
    else:
        extracted_match = match_or_group

    game = saved_game.Game(extracted_match["position"], extracted_match["played_moves"])

    try:
        saved_game.save_game(f"replay--{_format_datetime(obj["datetime"])}", game)
    except saved_game.SavedGameError as err:
        raise DataError(f"Could not write file: {err}")


def _generate_single_match_report(report: SingleMatchReport):
    obj = {
        "black_engine": {
            "name": report.black_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.black_engine.parameters]
        },

        "white_engine": {
            "name": report.white_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.white_engine.parameters]
        },

        "match": {
            "position": report.match.position,
            "time": report.match.time,
            "ending": str(report.match.ending),
            "plies": report.match.plies,
            "played_moves": [move for move in report.match.played_moves]
        },

        "rematch": {
            "position": report.rematch.position,
            "time": report.rematch.time,
            "ending": f"{report.rematch.ending} (swapped engines)",
            "plies": report.rematch.plies,
            "played_moves": [move for move in report.rematch.played_moves]
        },

        "datetime": report.datetime,
        "type": "single"
    }

    _write_report(f"report-single-match--{_format_datetime(report.datetime)}.json", obj)


def _generate_multiple_matches_report(report: MultipleMatchesReport):
    assert report.total_black_engine_wins + report.total_white_engine_wins + report.total_draws == report.total_matches

    obj = {
        "black_engine": {
            "name": report.black_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.black_engine.parameters]
        },

        "white_engine": {
            "name": report.white_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.white_engine.parameters]
        },

        "total_black_engine_wins": report.total_black_engine_wins,
        "total_white_engine_wins": report.total_white_engine_wins,
        "total_draws": report.total_draws,
        "total_matches": report.total_matches,
        "average_time": report.average_time,
        "average_plies": report.average_plies,

        "matches": [
            {
                "position": match.position,
                "time": match.time,
                "ending": str(match.ending),
                "plies": match.plies,
                "played_moves": [move for move in match.played_moves]
            }
            for match in report.matches
        ],

        "rematches": [
            {
                "position": rematch.position,
                "time": rematch.time,
                "ending": f"{rematch.ending} (swapped engines)",
                "plies": rematch.plies,
                "played_moves": [move for move in rematch.played_moves]
            }
            for rematch in report.matches
        ],

        "datetime": report.datetime,
        "type": "multiple"
    }

    _write_report(f"report-multiple-matches--{_format_datetime(report.datetime)}.json", obj)


def _write_report(name: str, obj: Any):
    try:
        with open(name, "w") as file:
            json.dump(obj, file, indent=4)
    except Exception as err:
        raise DataError(f"Could not write JSON file: {err}")


def _format_datetime(datetime: str) -> str:
    weekday, month, day, time, year = datetime.split(" ")
    hour, minute, second = time.split(":")

    return f"{weekday}-{month}-{day}--{hour}-{minute}-{second}--{year}"
