import json

# Don't use JSON schema validation

SAVED_DATA_FILE = "saved_data.json"


def save_engine_path(engine_path: str):
    try:
        with open(SAVED_DATA_FILE, "w") as file:
            json.dump({"engine_path": engine_path}, file)
    except Exception:
        pass


def load_engine_path() -> str | None:
    try:
        with open(SAVED_DATA_FILE, "r") as file:
            obj = json.load(file)

        return obj["engine_path"]
    except Exception:
        return None
