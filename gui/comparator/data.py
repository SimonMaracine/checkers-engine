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
    name: str
    parameters: list[tuple[str, str, str]]


@dataclasses.dataclass(slots=True)
class MatchResult:
    position: str
    ending: MatchEnding
    plies: int
    played_moves: list[str]
    time: float


@dataclasses.dataclass(slots=True)
class SingleMatchReport:
    black_engine: EngineStats
    white_engine: EngineStats
    match: MatchResult
    rematch: MatchResult
    datetime: str


@dataclasses.dataclass(slots=True)
class MultipleMatchesReport:
    black_engine: EngineStats
    white_engine: EngineStats
    matches: list[MatchResult]
    rematches: list[MatchResult]
    datetime: str


def generate_report(report: SingleMatchReport | MultipleMatchesReport):
    match report:
        case SingleMatchReport():
            _generate_single_match_report(report)
        case MultipleMatchesReport():
            _generate_multiple_matches_report(report)


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
            "ending": f"{report.rematch.ending} (swapped colors)",
            "plies": report.rematch.plies,
            "played_moves": [move for move in report.rematch.played_moves]
        },

        "datetime": report.datetime,
        "type": "single"
    }

    _write_report(f"report_single_match {report.datetime}.json", obj)


def _generate_multiple_matches_report(report: MultipleMatchesReport):
    obj = {
        "black_engine": {
            "name": report.black_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.black_engine.parameters]
        },

        "white_engine": {
            "name": report.white_engine.name,
            "parameters": [f"{parameter[0]} {parameter[1]} {parameter[2]}" for parameter in report.white_engine.parameters]
        },

        # TODO number of wins/losses/draws for the two engines, number of matches + rematches, average time, average plies

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
                "ending": f"{rematch.ending} (swapped colors)",
                "plies": rematch.plies,
                "played_moves": [move for move in rematch.played_moves]
            }
            for rematch in report.matches
        ],

        "datetime": report.datetime,
        "type": "multiple"
    }

    _write_report(f"report_multiple_matches {report.datetime}.json", obj)


def _write_report(name: str, obj: object):
    try:
        with open(name, "w") as file:
            json.dump(obj, file, indent=4)
    except Exception as err:
        raise DataError(f"Could not write JSON file: {err}")
