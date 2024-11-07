import json
import dataclasses

import jsonschema


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
    SCHEMA = {
        "type": "object",
        "properties": {
            "position": {
                "type": "string"
            },
            "moves": {
                "type": "array",
                "items": { "type": "string" }
            }
        },
        "required": ["position", "moves"]
    }

    try:
        with open(file_path, "r") as file:
            obj = json.load(file)
    except Exception as err:
        raise SavedGameError(f"Could not parse JSON file: {err}")

    try:
        jsonschema.validate(obj, SCHEMA)
    except jsonschema.ValidationError as err:
        raise SavedGameError(f"Invalid JSON file: {err}")

    return Game(obj["position"], obj["moves"])
