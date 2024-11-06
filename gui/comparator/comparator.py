import dataclasses
import json


class MatchError(RuntimeError):
    pass


@dataclasses.dataclass(slots=True, frozen=True)
class MatchFile:
    positions: list[str]


def parse_match_file(file_path: str) -> MatchFile:
    pass


def run_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    if not match_file.positions:
        raise MatchError("No positions")

    if len(match_file.positions) == 1:
        _single_match(match_file, path_engine_black, path_engine_white)
    else:
        _multiple_match(match_file, path_engine_black, path_engine_white)


def _single_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    pass


def _multiple_match(match_file: MatchFile, path_engine_black: str, path_engine_white: str):
    pass
