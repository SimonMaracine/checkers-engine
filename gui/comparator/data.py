import json
import enum
import dataclasses


class DataError(RuntimeError):
    pass


class MatchEnding(enum.Enum):
    WinnerBlack = enum.auto()
    WinnerWhite = enum.auto()
    TieBetweenBothPlayers = enum.auto()

    def __str__(self) -> str:
        match self:
            case self.WinnerBlack:
                return "winner black"
            case self.WinnerWhite:
                return "winner white"
            case self.TieBetweenBothPlayers:
                return "tie between both players"

        assert False


@dataclasses.dataclass(slots=True)
class EngineStats:
    file_name: str
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
    black_engine: EngineStats
    white_engine: EngineStats

    match: MatchResult
    rematch: MatchResult

    datetime: str


def generate_report(report: SingleMatchReport):
    obj = {
        "black_engine": {
            "file_name": report.black_engine.file_name,
            "name": report.black_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.black_engine.parameters]
        },

        "white_engine": {
            "file_name": report.white_engine.file_name,
            "name": report.white_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.white_engine.parameters]
        },

        "match": {
            "position": report.match.position,
            "time": report.match.time,
            "ending": str(report.match.ending),
            "played_moves": [move for move in report.match.played_moves]
        },

        "rematch": {
            "position": report.rematch.position,
            "time": report.rematch.time,
            "ending": str(report.rematch.ending),
            "played_moves": [move for move in report.rematch.played_moves]
        },

        "datetime": report.datetime
    }

    _write_report(f"single_match {report.datetime}.json", obj)


def _write_report(name: str, obj: object):
    try:
        with open(name, "w") as file:
            json.dump(obj, file, indent=4)
    except Exception as err:
        raise DataError(f"Could not write JSON file: {err}")
