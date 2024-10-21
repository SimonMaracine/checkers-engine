import tkinter as tk
from typing import Callable

class FenStringWindow(tk.Frame):
    def __init__(self, top_level: tk.Toplevel, on_ok: Callable):
        super().__init__(top_level)

        self._top_level = top_level
        self._on_ok = on_ok

        self._setup_widgets()

    def _setup_widgets(self):
        self._top_level.title("FEN String")

        self.pack(fill="both", expand=True)
        self.rowconfigure(0, weight=1)
        self.rowconfigure(1, weight=1)
        self.columnconfigure(0, weight=1)
        self.columnconfigure(1, weight=1)

        self._var_fen_string = tk.StringVar(self)

        tk.Entry(self, textvariable=self._var_fen_string).grid(row=0, column=0, columnspan=2, sticky="ew", padx=50, pady=10)

        tk.Button(self, text="Cancel", command=self._cancel).grid(row=1, column=0, sticky="sew", padx=25, pady=(0, 10))
        tk.Button(self, text="Ok", command=self._ok).grid(row=1, column=1, sticky="sew", padx=25, pady=(0, 10))

    def _cancel(self):
        self._top_level.destroy()

    def _ok(self):
        self._on_ok(self._var_fen_string.get())
        self._top_level.destroy()
