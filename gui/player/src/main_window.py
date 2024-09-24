import tkinter as tk

class MainWindow(tk.Frame):
    def __init__(self, root: tk.Tk):
        super().__init__(root)

        self._tk = root
        self._return_code = 0

        self._setup_widgets()

    def code(self) -> int:
        return self._return_code

    def _setup_widgets(self):
        # self.root.option_add("*tearOff", False)
        self._tk.title("Checkers Player")
        self._tk.geometry("768x432")
        self._tk.protocol("WM_DELETE_WINDOW", self._on_window_closed)
        self._tk.minsize(512, 288)

        self.pack(fill="both", expand=True)

        # TODO temp
        self.config(bg="blue")

        self._setup_widgets_board()
        self._setup_widgets_right()

        # Do this after all widgets are configured
        self.bind("<Configure>", self._on_window_resized)

    def _setup_widgets_board(self):
        size = self._calculate_board_size()

        self._cvs_board = tk.Canvas(self, width=size, height=size, background="gray75")
        self._cvs_board.pack(expand=True)

        width = self._cvs_board["width"]
        square_size = float(width) / 8

        for i in range(8):
            for j in range(8):
                self._cvs_board.create_rectangle(i * square_size, j * square_size, i * square_size + square_size, j * square_size + square_size, fill="red" if (i + j) % 2 != 0 else "black")

    def _setup_widgets_right(self):
        pass

    def _on_window_resized(self, event):
        size = self._calculate_board_size((event.width, event.height))
        self._cvs_board.config(width=size, height=size)

    def _on_window_closed(self):
        self.destroy()
        self._tk.destroy()

    def _calculate_board_size(self, window_size=None) -> int:
        if window_size is None:
            self._tk.update()
            geometry = self._tk.geometry()
            size = geometry.split("+")[0].split("x")

            window_width = int(size[0])
            window_height = int(size[1])
        else:
            window_width = window_size[0]
            window_height = window_size[1]

        return min(window_width, window_height) - 40
