import subprocess
from typing import Optional

# https://docs.python.org/3.12/library/subprocess.html/
# https://docs.python.org/3.12/library/io.html/

class CheckersEngine:
    def __init__(self):
        self._process: Optional[subprocess.Popen] = None

    def start(self, path: str):
        try:
            self._process = subprocess.Popen(
                ["python", path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                text=True,
                bufsize=1
            )
        except Exception as err:
            print(f"Could not start process: {err}")
            raise

    def send(self, command: str):
        assert self._process is not None
        assert self._process.stdin is not None

        try:
            self._process.stdin.write(command + "\n")
            self._process.stdin.flush()
        except BrokenPipeError as err:
            print(f"Could not send command: {err}")

        except Exception as err:
            print(f"Could not send command: {err}")
            raise

    def receive(self) -> str:
        assert self._process is not None
        assert self._process.stdout is not None

        try:
            data = self._process.stdout.readline()
        except Exception as err:
            print(f"Could not receive message: {err}")
            raise

        return data

    def stop(self):
        assert self._process is not None

        self._process.terminate()

        try:
            self._process.wait(timeout=5)
        except subprocess.TimeoutExpired as err:
            print(f"Could not wait for process: {err}")
            self._process.kill()


if __name__ == "__main__":
    engine = CheckersEngine()

    try:
        engine.start("engine.py")

        while True:
            message = engine.receive().strip()
            print(message)
            if "READY" in message:
                break

        print("Engine is ready")

        engine.send("GETPARAMETERS")

        while True:
            message = engine.receive().strip()
            print(message)
            if "PARAMETERS" in message:
                break

        engine.send("GO")

        while True:
            message = engine.receive().strip()
            print(message)
            if "BESTMOVE" in message:
                break

        engine.send("STOP")

        while True:
            message = engine.receive().strip()
            print(message)
            if "BESTMOVE" in message:
                break

        engine.send("QUIT")

        engine.stop()
    except Exception:
        print("Really bad")
