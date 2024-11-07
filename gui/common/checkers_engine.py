import subprocess
import threading
import queue

# https://docs.python.org/3.12/library/subprocess.html/
# https://docs.python.org/3.12/library/io.html/


class CheckersEngineError(RuntimeError):
    pass


class CheckersEngine:
    READ_ERROR = object()

    def __init__(self):
        self._process: subprocess.Popen | None = None
        self._reading_queue: queue.Queue[str | object] = queue.Queue()
        self._running = False

    def start(self, file_path: str):
        try:
            self._process = subprocess.Popen(
                [file_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                text=True,
                bufsize=1
            )

            self._running = True
        except Exception as err:
            raise CheckersEngineError(f"Could not start process: {err}")

        threading.Thread(target=self._read).start()

    def stop(self, force=False):
        # This method must be called

        assert self._process is not None

        if force:
            self._process.terminate()

        try:
            self._process.wait(timeout=10)
        except subprocess.TimeoutExpired as err:
            self._process.kill()

        self._running = False

    def running(self) -> bool:
        return self._running

    def send(self, command: str):
        assert self._process is not None
        assert self._process.stdin is not None

        try:
            self._process.stdin.write(command + "\n")
            self._process.stdin.flush()
        except BrokenPipeError as err:
            raise CheckersEngineError(f"Could not send command: {err}")
        except Exception as err:
            raise CheckersEngineError(f"Could not send command: {err}")

    def receive(self) -> str:
        try:
            item = self._reading_queue.get_nowait()
        except queue.Empty:
            return ""

        if item is self.READ_ERROR:
            raise CheckersEngineError(self._reading_queue.get())

        assert isinstance(item, str)

        return item

    def _read(self):
        while self._running:
            assert self._process is not None
            assert self._process.stdout is not None

            try:
                data = self._process.stdout.readline()
            except Exception as err:
                self._reading_queue.put_nowait(self.READ_ERROR)
                self._reading_queue.put_nowait(f"Could not receive message: {err}")
            else:
                self._reading_queue.put_nowait(data)
