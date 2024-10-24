import abc
import tkinter as tk

import pygame as pyg

from . import common


class BaseMainWindow(abc.ABC, tk.Frame):
    def __init__(self, root: tk.Tk, title: str):
        super().__init__(root)

        self._tk = root
        self._return_code = 0
        self._indices = False

        self._setup_widgets(title)
        self._initialize_sound()

    def code(self) -> int:
        return self._return_code

    @abc.abstractmethod
    def _setup_widgets_menubar(self):
        ...

    @abc.abstractmethod
    def _setup_widgets_center(self):
        ...

    @abc.abstractmethod
    def _setup_widgets_right(self):
        ...

    @abc.abstractmethod
    def _exit_application(self):
        ...

    def _setup_widgets(self, title: str):
        self._tk.option_add("*tearOff", False)
        self._tk.title(title)
        self._tk.protocol("WM_DELETE_WINDOW", self._exit_application)
        self._tk.minsize(512, 288)

        self.pack(fill="both", expand=True)
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        self._setup_widgets_menubar()
        self._setup_widgets_board()
        self._setup_widgets_center()
        self._setup_widgets_right()

    def _setup_widgets_board(self):
        self._frm_left = tk.Frame(self)
        self._frm_left.grid(row=0, column=0, sticky="nsew")

        self._square_size = common.DEFAULT_BOARD_SIZE / 8.0

        self._cvs_board = tk.Canvas(self._frm_left, width=common.DEFAULT_BOARD_SIZE, height=common.DEFAULT_BOARD_SIZE, background="gray75")
        self._cvs_board.pack(expand=True)

        for i in range(8):
            for j in range(8):
                color = common.BLACK if (i + j) % 2 != 0 else common.WHITE

                self._cvs_board.create_rectangle(
                    i * self._square_size,
                    j * self._square_size,
                    i * self._square_size + self._square_size,
                    j * self._square_size + self._square_size,
                    fill=color,
                    outline=color,
                    tags="all"
                )

    def _initialize_sound(self):
        pyg.mixer.init()
        self._sound = pyg.mixer.Sound("common/wood_click.wav")

    def _uninitialize_sound(self):
        pyg.mixer.quit()

    def _play_sound(self):
        self._sound.play()

    def _on_window_resized(self, event):
        size = self._calculate_board_size()
        new_square_size = float(size) / 8.0
        scale = new_square_size / self._square_size

        self._square_size = new_square_size
        self._cvs_board.config(width=size, height=size)
        self._cvs_board.scale("all", 0, 0, scale, scale)

    def _show_indices(self):
        if not self._indices:
            self._draw_indices()
        else:
            self._cvs_board.delete("indices")

        self._indices = not self._indices

    def _calculate_board_size(self) -> int:
        self._frm_left.update()  # This makes things work... I'm very happy that it does
        size = self._frm_left.winfo_geometry().split("+")[0].split("x")

        PADDING = 40

        return max(min(int(size[0]), int(size[1])) - PADDING, common.DEFAULT_BOARD_SIZE)

    def _draw_indices(self):
        index = 1

        for i in range(8):
            for j in range(8):
                if (i + j) % 2 != 0:
                    self._cvs_board.create_text(
                        j * self._square_size + self._square_size / 2.0,
                        i * self._square_size + self._square_size / 2.0,
                        fill="white",
                        text=str(index),
                        tags=("all", "indices")
                    )
                    index += 1
