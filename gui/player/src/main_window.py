import tkinter as tk
import tkinter.messagebox

import fen_string_window
import board

# https://tkdocs.com/tutorial/canvas.html

# TODO
# valgrind --tool=callgrind --cache-sim=yes --dump-instr=yes --branch=yes
# kcachegrind
# perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-load-misses
# perf record -e cycles,instructions,L1-dcache-loads,L1-dcache-load-misses --call-graph=dwarf
# hotspot
# ldd

class MainWindow(tk.Frame):
    # ORANGE = wxColour(240, 180, 80)
    # REDDISH = wxColour(255, 140, 60)
    # DARKER_REDDISH = wxColour(255, 100, 40)
    # GREEN = wxColour(70, 140, 70)
    # PURPLE = wxColour(170, 140, 170)
    WHITE = "#c8c8c8"
    BLACK = "#503c28"

    HUMAN = 1
    COMPUTER = 2

    def __init__(self, root: tk.Tk):
        super().__init__(root)

        self._tk = root
        self._return_code = 0
        self._indices = False

        self._setup_widgets()

        self._board = board.CheckersBoard(self._on_piece_move, self._cvs_board)
        self._board.set_user_input(True)  # TODO temp

    def code(self) -> int:
        return self._return_code

    def _setup_widgets(self):
        self._tk.option_add("*tearOff", False)
        self._tk.title("Checkers Player")
        self._tk.geometry("768x432")
        self._tk.protocol("WM_DELETE_WINDOW", self._exit_application)
        self._tk.minsize(512, 288)

        self.pack(fill="both", expand=True)
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        # TODO temp
        self.config(bg="blue")

        self._setup_widgets_menubar()
        self._setup_widgets_board()
        self._setup_widgets_center()
        self._setup_widgets_right()

        # Do this after all widgets are configured
        self.bind("<Configure>", self._on_window_resized)
        self._cvs_board.bind("<Button-1>", self._on_left_mouse_button_pressed)
        self._cvs_board.bind("<Button-3>", self._on_right_mouse_button_pressed)

    def _setup_widgets_menubar(self):
        men_player = tk.Menu(self)
        men_player.add_command(label="Start Engine", command=None)  # TODO
        men_player.add_command(label="Reset Position", command=self._reset_position)
        men_player.add_command(label="Set Position", command=self._set_position)
        men_player.add_checkbutton(label="Show Indices", command=self._show_indices)
        men_player.add_command(label="Exit", command=self._exit_application)

        men_help = tk.Menu(self)
        men_help.add_command(label="About", command=self._about)

        men_main = tk.Menu(self)
        men_main.add_cascade(label="Player", menu=men_player)
        men_main.add_cascade(label="Help", menu=men_help)

        self._tk.config(menu=men_main)

    def _setup_widgets_board(self):
        size = self._calculate_board_size()
        self._square_size = float(size) / 8.0

        self._cvs_board = tk.Canvas(self, width=size, height=size, background="gray75")
        self._cvs_board.grid(row=0, column=0)

        for i in range(8):
            for j in range(8):
                color = self.BLACK if (i + j) % 2 != 0 else self.WHITE

                self._cvs_board.create_rectangle(
                    i * self._square_size,
                    j * self._square_size,
                    i * self._square_size + self._square_size,
                    j * self._square_size + self._square_size,
                    fill=color,
                    outline=color,
                    tags="all"
                )

    def _setup_widgets_center(self):
        frm_center = tk.Frame(self, relief="solid", borderwidth=1)
        frm_center.grid(row=0, column=1, sticky="ns")

        # TODO temp
        frm_center.config(bg="red")

        frm_status = tk.Frame(frm_center)
        frm_status.grid(row=0, column=0, sticky="ew")

        self._lbl_engine = tk.Label(frm_status, text="Engine:")
        self._lbl_engine.pack(anchor="w")

        self._lbl_status = tk.Label(frm_status, text="Status: game no started")
        self._lbl_status.pack(anchor="w")

        self._lbl_player = tk.Label(frm_status, text="Player: black")
        self._lbl_player.pack(anchor="w")

        self._lbl_plies_without_advancement = tk.Label(frm_status, text="Plies without advancement: 0")
        self._lbl_plies_without_advancement.pack(anchor="w")

        frm_players = tk.Frame(frm_center)
        frm_players.grid(row=1, column=0, sticky="ew")

        frm_players_black = tk.Frame(frm_players)
        frm_players_black.grid(row=0, column=0)

        self._var_player_black = tk.IntVar(frm_players_black, value=self.HUMAN)
        tk.Label(frm_players_black, text="Black").pack()
        tk.Radiobutton(frm_players_black, text="Human", variable=self._var_player_black, value=self.HUMAN, command=None).pack(anchor="w")  # TODO
        tk.Radiobutton(frm_players_black, text="Computer", variable=self._var_player_black, value=self.COMPUTER, command=None).pack(anchor="w")

        frm_players_white = tk.Frame(frm_players)
        frm_players_white.grid(row=0, column=1, sticky="ew")

        self._var_player_white = tk.IntVar(frm_players_white, value=self.COMPUTER)
        tk.Label(frm_players_white, text="White").pack()
        tk.Radiobutton(frm_players_white, text="Human", variable=self._var_player_white, value=self.HUMAN, command=None).pack(anchor="w")  # TODO
        tk.Radiobutton(frm_players_white, text="Computer", variable=self._var_player_white, value=self.COMPUTER, command=None).pack(anchor="w")

        frm_buttons = tk.Frame(frm_center)
        frm_buttons.grid(row=2, column=0, sticky="ew")
        frm_buttons.columnconfigure(0, weight=1)
        frm_buttons.columnconfigure(1, weight=1)

        tk.Button(frm_buttons, text="Stop", command=None).grid(row=0, column=0, sticky="ew")  # TODO
        tk.Button(frm_buttons, text="Continue", command=None).grid(row=0, column=1, sticky="ew")

        frm_parameters = tk.Frame(frm_center)
        frm_parameters.grid(row=3, column=0, sticky="ew")

        tk.Label(frm_parameters, text="params").pack(anchor="w")

    def _setup_widgets_right(self):
        frm_right = tk.Frame(self)
        frm_right.grid(row=0, column=2, sticky="nse")

        # TODO temp
        frm_right.config(bg="green")

        self._frm_moves = tk.Frame(frm_right)
        self._frm_moves.grid(row=0, column=0)

        # TODO temp
        tk.Label(self._frm_moves, text="1.").pack()

        self._frm_moves_black = tk.Frame(frm_right)
        self._frm_moves_black.grid(row=0, column=1)

        # TODO temp
        tk.Label(self._frm_moves_black, text="24x19").pack()

        self._frm_moves_white = tk.Frame(frm_right)
        self._frm_moves_white.grid(row=0, column=2)

        # TODO temp
        tk.Label(self._frm_moves_white, text="9x14").pack()

    def _on_window_resized(self, event):
        size = self._calculate_board_size((event.width, event.height))
        new_square_size = float(size) / 8.0
        scale = new_square_size / self._square_size
        self._square_size = new_square_size
        self._cvs_board.config(width=size, height=size)
        self._cvs_board.scale("all", 0, 0, scale, scale)

    def _on_left_mouse_button_pressed(self, event):
        self._board.press_square_left_button(self._get_square(event.x, event.y))

    def _on_right_mouse_button_pressed(self, event):
        self._board.press_square_right_button(self._get_square(event.x, event.y))

    def _reset_position(self):
        self._board.reset()

    def _set_position(self):
        top_level = tk.Toplevel(self)
        fen_string_window.FenStringWindow(top_level, self._set_position_string)

    def _set_position_string(self, string: str):
        self._board.reset(string)

    def _show_indices(self):
        if not self._indices:
            self._draw_indices()
        else:
            self._cvs_board.delete("indices")

        self._indices = not self._indices

    def _exit_application(self):
        self.destroy()
        self._tk.destroy()

    def _about(self):
        tkinter.messagebox.showinfo("About", "Checkers Player, an implementation of the game of checkers.")

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

    def _get_square(self, x: int, y: int) -> int:
        square_size = int(self._cvs_board["width"]) // 8

        file = x // square_size
        rank = y // square_size

        return rank * 8 + file

    def _draw_indices(self):
        index = 1

        for i in range(8):
            for j in range(8):
                if (i + j) % 2 != 0:
                    self._cvs_board.create_text(
                        i * self._square_size + self._square_size / 2.0,
                        j * self._square_size + self._square_size / 2.0,
                        fill="white",
                        text=str(index),
                        tags=("all", "indices")
                    )
                    index += 1

    def _on_piece_move(self, move: board.Move):
        print(move)
