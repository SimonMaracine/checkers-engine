import tkinter as tk

class MainWindow(tk.Frame):
    def __init__(self, root: tk.Tk):
        super().__init__(root)

        self._root = root
        self._return_code = 0

        self._setup_widgets()

    def code(self) -> int:
        return self._return_code

    def _setup_widgets(self):
        # self.root.option_add("*tearOff", False)
        self._root.title("Checkers Player")
        self._root.geometry("768x432")
        self._root.protocol("WM_DELETE_WINDOW", self._on_window_closed)
        self._root.minsize(512, 288)

        self.pack(fill="both", expand=True, padx=20, pady=20)

        # TODO temp
        self.configure(bg="red")

        self._setup_widgets_board()

    def _setup_widgets_board(self):
        tk.Label(self, text="Hello, world!").pack()

    def _on_window_closed(self):
        self.destroy()
        self._root.destroy()
