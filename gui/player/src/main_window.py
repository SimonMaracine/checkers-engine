import tkinter as tk

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
    GOLD = "#a0a00a" #wxColour(160, 160, 10)
    # GREEN = wxColour(70, 140, 70)
    # PURPLE = wxColour(170, 140, 170)
    WHITE = "#c8c8c8" #wxColour(200, 200, 200)
    BLACK = "#503c28" #wxColour(80, 60, 40)
    PIECE_WHITE = "#8c0a0a" #wxColour(140, 10, 10)
    PIECE_BLACK = "#0a0a0a" #wxColour(10, 10, 10)

    HUMAN = 1
    COMPUTER = 2

    def __init__(self, root: tk.Tk):
        super().__init__(root)

        self._tk = root
        self._return_code = 0
        self._board = board.CheckersBoard()

        self._setup_widgets()

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
        self._cvs_board.bind("<Button-1>", self._on_mouse_pressed)
        self._cvs_board.bind("<ButtonRelease-1>", self._on_mouse_released)

    def _setup_widgets_menubar(self):
        men_player = tk.Menu(self)
        men_player.add_command(label="Start Engine", command=None)  # TODO
        men_player.add_command(label="Reset Position", command=None)
        men_player.add_command(label="Set Position", command=None)
        men_player.add_checkbutton(label="Show Indices", command=None)
        men_player.add_command(label="Exit", command=self._exit_application)

        men_help = tk.Menu(self)
        men_help.add_command(label="About", command=None)

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
                self._cvs_board.create_rectangle(
                    i * self._square_size,
                    j * self._square_size,
                    i * self._square_size + self._square_size,
                    j * self._square_size + self._square_size,
                    fill=self.WHITE if (i + j) % 2 != 0 else self.BLACK
                )

        self._cvs_board.addtag_all("all")

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

    def _on_mouse_pressed(self, event):
        print(event)

    def _on_mouse_released(self, event):
        print(event)

    def _exit_application(self):
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
