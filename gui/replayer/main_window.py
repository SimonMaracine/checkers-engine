import tkinter as tk
import tkinter.messagebox
import tkinter.filedialog

import pygame as pyg


class MainWindow(tk.Frame):
    WHITE = "#c8c8c8"
    BLACK = "#503c28"
    HUMAN = 1
    COMPUTER = 2
    DEFAULT_BOARD_SIZE = 400
    CHECK_TIME = 30
    TXT_ENGINE = "Engine:"
    TXT_STOPPED = "Stopped:"
    TXT_STATUS = "Status:"
    TXT_PLAYER = "Player:"
    TXT_PLIES_WITHOUT_ADVANCEMENT = "Plies without advancement:"

    def __init__(self, root: tk.Tk):
        super().__init__(root)

        self._tk = root
        self._return_code = 0
        self._indices = False

        self._setup_widgets()

        pyg.mixer.init()
        self._sound = pyg.mixer.Sound("wood_click.wav")

    def code(self) -> int:
        return self._return_code

    def _setup_widgets(self):
        self._tk.option_add("*tearOff", False)
        self._tk.title("Checkers Player")
        self._tk.protocol("WM_DELETE_WINDOW", self._exit_application)
        self._tk.minsize(512, 288)

        self.pack(fill="both", expand=True)
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        self._setup_widgets_menubar()
        self._setup_widgets_board()
        self._setup_widgets_center()
        self._setup_widgets_right()

    def _setup_widgets_menubar(self):
        men_replayer = tk.Menu(self)
        men_replayer.add_command(label="Load Game", command=self._load_game)
        men_replayer.add_checkbutton(label="Show Indices", command=self._show_indices)
        men_replayer.add_command(label="Exit", command=self._exit_application)

        men_help = tk.Menu(self)
        men_help.add_command(label="About", command=self._about)

        men_main = tk.Menu(self)
        men_main.add_cascade(label="Replayer", menu=men_replayer)
        men_main.add_cascade(label="Help", menu=men_help)

        self._tk.config(menu=men_main)

    def _setup_widgets_board(self):
        self._frm_left = tk.Frame(self)
        self._frm_left.grid(row=0, column=0, sticky="nsew")

        self._square_size = self.DEFAULT_BOARD_SIZE / 8.0

        self._cvs_board = tk.Canvas(self._frm_left, width=self.DEFAULT_BOARD_SIZE, height=self.DEFAULT_BOARD_SIZE, background="gray75")
        self._cvs_board.pack(expand=True)

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
        frm_center.grid(row=0, column=1, sticky="ns", padx=10, pady=10)
        frm_center.rowconfigure(3, weight=1)

        frm_status = tk.Frame(frm_center)
        frm_status.grid(row=0, column=0, sticky="ew", padx=10, pady=(10, 5))

        self._var_engine = tk.StringVar(frm_status, self.TXT_ENGINE + " [none]")

        lbl_engine = tk.Label(frm_status, textvariable=self._var_engine)
        lbl_engine.pack(anchor="w")

        self._var_stopped = tk.StringVar(frm_status, f"{self.TXT_STOPPED} {self._stopped}")

        lbl_stopped = tk.Label(frm_status, textvariable=self._var_stopped)
        lbl_stopped.pack(anchor="w")

        self._var_status = tk.StringVar(frm_status, self.TXT_STATUS + " game not started")

        lbl_status = tk.Label(frm_status, textvariable=self._var_status)
        lbl_status.pack(anchor="w")

        self._var_player = tk.StringVar(frm_status, self.TXT_PLAYER + " black")

        lbl_player = tk.Label(frm_status, textvariable=self._var_player)
        lbl_player.pack(anchor="w")

        self._var_plies_without_advancement = tk.StringVar(frm_status, self.TXT_PLIES_WITHOUT_ADVANCEMENT + " 0")

        lbl_plies_without_advancement = tk.Label(frm_status, textvariable=self._var_plies_without_advancement)
        lbl_plies_without_advancement.pack(anchor="w")

        frm_players = tk.Frame(frm_center)
        frm_players.grid(row=1, column=0, sticky="ew", padx=10, pady=5)

        frm_players_black = tk.Frame(frm_players)
        frm_players_black.grid(row=0, column=0)

        tk.Label(frm_players_black, text="Black").pack()
        self._var_player_black = tk.IntVar(frm_players_black, value=self.HUMAN)
        self._btn_black_human = tk.Radiobutton(
            frm_players_black,
            text="Human",
            variable=self._var_player_black,
            command=self._player_changed,
            value=self.HUMAN
        )
        self._btn_black_human.pack(anchor="w")
        self._btn_black_computer = tk.Radiobutton(
            frm_players_black,
            text="Computer",
            variable=self._var_player_black,
            command=self._player_changed,
            value=self.COMPUTER
        )
        self._btn_black_computer.pack(anchor="w")

        frm_players_white = tk.Frame(frm_players)
        frm_players_white.grid(row=0, column=1, sticky="ew")

        tk.Label(frm_players_white, text="White").pack()
        self._var_player_white = tk.IntVar(frm_players_white, value=self.COMPUTER)
        self._btn_white_human = tk.Radiobutton(
            frm_players_white,
            text="Human",
            variable=self._var_player_white,
            command=self._player_changed,
            value=self.HUMAN
        )
        self._btn_white_human.pack(anchor="w")
        self._btn_white_computer = tk.Radiobutton(
            frm_players_white,
            text="Computer",
            variable=self._var_player_white,
            command=self._player_changed,
            value=self.COMPUTER
        )
        self._btn_white_computer.pack(anchor="w")

        frm_buttons = tk.Frame(frm_center)
        frm_buttons.grid(row=2, column=0, sticky="ew", padx=10, pady=5)
        frm_buttons.columnconfigure(0, weight=1)
        frm_buttons.columnconfigure(1, weight=1)

        self._btn_stop = tk.Button(frm_buttons, text="Stop", command=self._stop)
        self._btn_stop.grid(row=0, column=0, sticky="ew")
        self._btn_continue = tk.Button(frm_buttons, text="Continue", command=self._continue)
        self._btn_continue.grid(row=0, column=1, sticky="ew")

        frm_parameters = tk.Frame(frm_center)
        frm_parameters.grid(row=3, column=0, sticky="nsew", padx=10, pady=(5, 10))

        bar_parameters = tk.Scrollbar(frm_parameters, orient="vertical")
        bar_parameters.pack(side="right", fill="y")

        self._cvs_parameters = tk.Canvas(frm_parameters, width=240, yscrollcommand=bar_parameters.set)
        self._cvs_parameters.pack(side="left", fill="both", expand=True)

        bar_parameters.config(command=self._cvs_parameters.yview)

        self._frm_parameters = tk.Frame(self._cvs_parameters)
        self._frm_parameters.bind("<Configure>", lambda _: self._cvs_parameters.config(scrollregion=self._cvs_parameters.bbox("all")))
        self._cvs_parameters.create_window(0.0, 0.0, window=self._frm_parameters, anchor="nw", width=240)

        self._btn_black_human.config(state="disabled")
        self._btn_black_computer.config(state="disabled")
        self._btn_white_human.config(state="disabled")
        self._btn_white_computer.config(state="disabled")

        self._btn_stop.config(state="disabled")
        self._btn_continue.config(state="disabled")

    def _setup_widgets_right(self):
        frm_right = tk.Frame(self, relief="solid", borderwidth=1)
        frm_right.grid(row=0, column=2, sticky="nse", padx=10, pady=10)

        frm_moves = tk.Frame(frm_right)
        frm_moves.pack(fill="both", expand=True, padx=10, pady=10)

        bar_moves_vertical = tk.Scrollbar(frm_moves, orient="vertical")
        bar_moves_vertical.pack(side="right", fill="y")

        bar_moves_horizontal = tk.Scrollbar(frm_moves, orient="horizontal")
        bar_moves_horizontal.pack(side="bottom", fill="x")

        self._cvs_moves = tk.Canvas(frm_moves, width=200, yscrollcommand=bar_moves_vertical.set, xscrollcommand=bar_moves_horizontal.set)
        self._cvs_moves.pack(side="left", fill="both", expand=True)

        bar_moves_vertical.config(command=self._cvs_moves.yview)
        bar_moves_horizontal.config(command=self._cvs_moves.xview)

        frm_moves_main = tk.Frame(self._cvs_moves)
        frm_moves_main.bind("<Configure>", lambda _: self._cvs_moves.config(scrollregion=self._cvs_moves.bbox("all")))
        self._cvs_moves.create_window(0.0, 0.0, window=frm_moves_main, anchor="nw")

        self._frm_moves_index = tk.Frame(frm_moves_main)
        self._frm_moves_index.grid(row=0, column=0, sticky="n")

        self._frm_moves_black = tk.Frame(frm_moves_main)
        self._frm_moves_black.grid(row=0, column=1, sticky="n")

        self._frm_moves_white = tk.Frame(frm_moves_main)
        self._frm_moves_white.grid(row=0, column=2, sticky="n")

    def _on_window_resized(self, event):
        size = self._calculate_board_size()
        new_square_size = float(size) / 8.0
        scale = new_square_size / self._square_size

        self._square_size = new_square_size
        self._cvs_board.config(width=size, height=size)
        self._cvs_board.scale("all", 0, 0, scale, scale)

    def _load_game(self):
        pass

    def _show_indices(self):
        if not self._indices:
            self._draw_indices()
        else:
            self._cvs_board.delete("indices")

        self._indices = not self._indices

    def _exit_application(self):
        pyg.mixer.quit()
        self._tk.destroy()

    def _about(self):
        tkinter.messagebox.showinfo("About", "Checkers Replayer, replay games of checkers.")

    def _stop(self):
        pass

    def _continue(self):
        pass

    def _calculate_board_size(self) -> int:
        self._frm_left.update()  # This makes things work... I'm very happy that it does
        size = self._frm_left.winfo_geometry().split("+")[0].split("x")

        PADDING = 40

        return max(min(int(size[0]), int(size[1])) - PADDING, self.DEFAULT_BOARD_SIZE)

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

    def _update_status(self):
        match self._board.get_game_over():
            case board.GameOver.None_:
                self._var_status.set(self.TXT_STATUS + " game in progress")
            case board.GameOver.WinnerBlack:
                self._var_status.set(self.TXT_STATUS + " black player won the game")
            case board.GameOver.WinnerWhite:
                self._var_status.set(self.TXT_STATUS + " white player won the game")
            case board.GameOver.TieBetweenBothPlayers:
                self._var_status.set(self.TXT_STATUS + " tie between both players")

        match self._board.get_turn():
            case board.Player.Black:
                self._var_player.set(self.TXT_PLAYER + " black")
            case board.Player.White:
                self._var_player.set(self.TXT_PLAYER + " white")

        self._var_plies_without_advancement.set(f"{self.TXT_PLIES_WITHOUT_ADVANCEMENT} {self._board.get_plies_without_advancement()}")

    def _reset_status(self):
        self._var_status.set(self.TXT_STATUS + " game not started")
        self._var_player.set(self.TXT_PLAYER + " black")
        self._var_plies_without_advancement.set(self.TXT_PLIES_WITHOUT_ADVANCEMENT + " 0")
