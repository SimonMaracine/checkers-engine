import subprocess
import queue
import select
import copy

# https://docs.python.org/3.12/library/subprocess.html/
# https://docs.python.org/3.12/library/io.html/
# https://docs.python.org/3/library/select.html/


class CheckersEngineError(RuntimeError):
    pass


class CheckersEngine:
    def __init__(self):
        self._process: subprocess.Popen | None = None
        self._reading_queue: queue.Queue[str] = queue.Queue()
        self._read_buffer: bytearray = bytearray()
        self._running = False

    def start(self, file_path: str):
        try:
            self._process = subprocess.Popen([file_path], stdin=subprocess.PIPE, stdout=subprocess.PIPE, bufsize=0)
        except Exception as err:
            raise CheckersEngineError(f"Could not start process: {err}")

        self._running = True

    def stop(self, term: bool = False, timeout: float = 5.0):
        # This method must be called to cleanly stop the subprocess
        # May be called multiple times

        if not self._running:
            return

        assert self._process is not None
        assert self._process.stdin is not None
        assert self._process.stdout is not None

        if term:
            self._process.terminate()

        try:
            self._process.wait(timeout)
        except subprocess.TimeoutExpired:
            self._process.kill()

        self._process.stdin.close()
        self._process.stdout.close()

        self._running = False

    def running(self) -> bool:
        return self._running

    def send(self, command: str):
        assert self._process is not None
        assert self._process.stdin is not None

        try:
            self._process.stdin.write(bytes(command, encoding="ascii") + b"\n")
            self._process.stdin.flush()
        except BrokenPipeError as err:
            raise CheckersEngineError(f"Could not send command: {err}")
        except Exception as err:
            raise CheckersEngineError(f"Could not send command: {err}")

    def receive(self, timeout: float = 0.0) -> str:
        assert self._process is not None
        assert self._process.stdout is not None

        try:
            return self._reading_queue.get_nowait()
        except queue.Empty:
            pass

        result, *_ = select.select([self._process.stdout.fileno()], [], [], timeout)

        if not result:
            return ""

        try:
            data = self._process.stdout.read(32)
        except Exception as err:
            raise CheckersEngineError(f"Could not receive message: {err}")

        self._read_buffer += data

        if b"\n" not in data:
            return ""

        last_index = 0

        for i, byte in enumerate(self._read_buffer):
            if byte.to_bytes() == b"\n":
                self._reading_queue.put_nowait(self._read_buffer[last_index:i].decode(encoding="ascii"))
                last_index = i

        remainder = copy.copy(self._read_buffer[last_index:])
        self._read_buffer.clear()
        self._read_buffer += remainder

        return self._reading_queue.get_nowait()
