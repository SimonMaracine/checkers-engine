import json
import dataclasses


class SavedGameError(RuntimeError):
    pass


@dataclasses.dataclass(slots=True, frozen=True)
class Game:
    position: str
    moves: list[str]


def save_game(file_path: str, game: Game):
    obj = {
        "position": game.position,
        "moves": game.moves
    }

    with open(file_path, "w") as file:
        json.dump(obj, file)


def load_game(file_path: str) -> Game:
    try:
        with open(file_path, "r") as file:
            obj = json.load(file)

        return Game(obj["position"], obj["moves"])
    except Exception as err:
        raise SavedGameError(f"Could not load game: {err}")
