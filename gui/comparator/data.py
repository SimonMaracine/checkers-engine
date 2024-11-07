import json
import enum
import dataclasses


class EngineFileType(enum.Enum):
    Binary = enum.auto()
    Text = enum.auto()


class MatchEnding(enum.Enum):
    WinnerBlack = enum.auto()
    WinnerWhite = enum.auto()
    TieBetweenBothPlayers = enum.auto()


@dataclasses.dataclass(slots=True)
class Engine:
    file_name: str
    file_type: EngineFileType
    file_size: int
    name: str
    parameters: list[tuple[str, str, str]]


@dataclasses.dataclass(slots=True)
class MatchResult:
    position: str
    ending: MatchEnding
    played_moves: list[str]
    time: float


@dataclasses.dataclass(slots=True)
class SingleMatchReport:
    black_engine: Engine
    white_engine: Engine

    match: MatchResult
    rematch: MatchResult

    datetime: str


def collect_data(engine_file_path: str):
    pass


def generate_report(report: SingleMatchReport):
    obj = {
        "black_engine_file_name": report.black_engine.file_name,
        "white_engine_file_name": report.white_engine.file_name,

        "black_engine_file_type": str(report.black_engine.file_type),
        "white_engine_file_type": str(report.white_engine.file_type),

        "black_engine_file_size": report.black_engine.file_size,
        "white_engine_file_size": report.white_engine.file_size,

        "black_engine_name": report.black_engine.name,
        "white_engine_name": report.white_engine.name,

        "black_engine_parameters": [f"{parameter[0]}, {parameter[1]}, {parameter[2]}" for parameter in report.black_engine.parameters],
        "white_engine_parameters": [f"{parameter[0]}, {parameter[1]}, {parameter[2]}" for parameter in report.white_engine.parameters],

        "match_position": report.match.position,
        "match_ending": str(report.match.ending),
        "match_played_moves": [move for move in report.match.played_moves],
        "match_time": report.match.time,

        "rematch_position": report.rematch.position,
        "rematch_ending": str(report.rematch.ending),
        "rematch_played_moves": [move for move in report.rematch.played_moves],
        "rematch_time": report.rematch.time,

        "datetime": report.datetime
    }

    _write_report(f"single_match_{report.datetime}.json", obj)


def _write_report(name: str, obj: object):
    with open(name, "w") as file:
        json.dump(obj, file)
