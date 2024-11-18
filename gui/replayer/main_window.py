import sys
import tkinter as tk
import tkinter.messagebox
import tkinter.filedialog
from typing import Optional

from common import base_main_window
from common import board
from common import common
from common import saved_game


class MainWindow(base_main_window.BaseMainWindow):
    def __init__(self, root: tk.Tk):
        super().__init__(root, "Checkers Replayer")

        self._board = board.CheckersBoard(None, self._cvs_board, False)
        self._game: Optional[saved_game.Game] = None
        self._move_index = 0

        # Do this after all widgets are configured
        self.bind("<Configure>", self._on_window_resized)

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

    def _setup_widgets_center(self):
        frm_center = tk.Frame(self, relief="solid", borderwidth=1)
        frm_center.grid(row=0, column=1, sticky="ns", padx=10, pady=10)
        frm_center.rowconfigure(3, weight=1)

        frm_status = tk.Frame(frm_center)
        frm_status.grid(row=0, column=0, sticky="ew", padx=10, pady=(10, 5))

        self._var_status = tk.StringVar(frm_status, common.TXT_STATUS + " game not started")

        lbl_status = tk.Label(frm_status, textvariable=self._var_status)
        lbl_status.pack(anchor="w")

        self._var_player = tk.StringVar(frm_status, common.TXT_PLAYER + " black")

        lbl_player = tk.Label(frm_status, textvariable=self._var_player)
        lbl_player.pack(anchor="w")

        self._var_plies_without_advancement = tk.StringVar(frm_status, common.TXT_PLIES_WITHOUT_ADVANCEMENT + " 0")

        lbl_plies_without_advancement = tk.Label(frm_status, textvariable=self._var_plies_without_advancement)
        lbl_plies_without_advancement.pack(anchor="w")

        frm_buttons = tk.Frame(frm_center)
        frm_buttons.grid(row=2, column=0, sticky="ew", padx=10, pady=5)
        frm_buttons.columnconfigure(0, weight=1)
        frm_buttons.columnconfigure(1, weight=1)

        self._btn_previous = tk.Button(frm_buttons, text="Previous", command=self._previous)
        self._btn_previous.grid(row=0, column=0, sticky="ew")
        self._btn_next = tk.Button(frm_buttons, text="Next", command=self._next)
        self._btn_next.grid(row=0, column=1, sticky="ew")

        self._btn_previous.config(state="disabled")
        self._btn_next.config(state="disabled")

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

    def _exit_application(self):
        self._uninitialize_sound()
        self._tk.destroy()

    def _load_game(self):
        file_path = tkinter.filedialog.askopenfilename(parent=self, title="Load Game")

        if file_path in ((), ""):
            return

        try:
            self._game = saved_game.load_game(file_path)
        except saved_game.SavedGameError as err:
            print(err, file=sys.stderr)
            return

        self._move_index = 0
        self._clear_moves()
        self._board.reset(self._game.position)
        self._board.redraw()
        self._check_control_buttons_for_previous()
        self._check_control_buttons_for_next()

        if self._board.get_turn() == board.Player.White:
            self._record_dummy_black_move(1)

        print("Successfully loaded game", file=sys.stderr)

    def _about(self):
        tkinter.messagebox.showinfo("About", "Checkers Replayer, replay games of checkers.")

    def _previous(self):
        assert self._game is not None

        self._disable_control_buttons()

        self._move_index -= 1
        self._clear_moves()
        self._setup_position_and_play_moves(self._game.position, self._game.moves[0:self._move_index])
        self._update_status()
        self._play_sound()

        self._enable_control_buttons()
        self._check_control_buttons_for_previous()

    def _next(self):
        assert self._game is not None

        self._disable_control_buttons()

        self._move_index += 1
        self._clear_moves()
        self._setup_position_and_play_moves(self._game.position, self._game.moves[0:self._move_index])
        self._update_status()
        self._play_sound()

        self._enable_control_buttons()
        self._check_control_buttons_for_next()

    def _update_status(self):
        match self._board.get_game_over():
            case board.GameOver.None_:
                self._var_status.set(common.TXT_STATUS + " game in progress")
            case board.GameOver.WinnerBlack:
                self._var_status.set(common.TXT_STATUS + " black player won the game")
            case board.GameOver.WinnerWhite:
                self._var_status.set(common.TXT_STATUS + " white player won the game")
            case board.GameOver.TieBetweenBothPlayers:
                self._var_status.set(common.TXT_STATUS + " tie between both players")

        match self._board.get_turn():
            case board.Player.Black:
                self._var_player.set(common.TXT_PLAYER + " black")
            case board.Player.White:
                self._var_player.set(common.TXT_PLAYER + " white")

        self._var_plies_without_advancement.set(f"{common.TXT_PLIES_WITHOUT_ADVANCEMENT} {self._board.get_plies_without_advancement()}")

    def _record_move(self, move: str, player: board.Player, index: int):
        match player:
            case board.Player.Black:
                tk.Label(self._frm_moves_index, text=f"{index}.").pack(anchor="nw")
                tk.Label(self._frm_moves_black, text=move).pack(anchor="nw")
            case board.Player.White:
                tk.Label(self._frm_moves_white, text=move).pack(anchor="nw")

        # Make canvas know that it just got new widgets
        self._cvs_moves.update()

        # Don't make the scrollbar jump for the first few widgets
        if index > 4:
            self._cvs_moves.yview_moveto(1.0)

    def _record_dummy_black_move(self, index: int):
        assert index == 1

        tk.Label(self._frm_moves_index, text=f"{index}.").pack(anchor="nw")
        tk.Label(self._frm_moves_black, text="--/--").pack(anchor="nw")

        # Make canvas know that it just got new widgets
        self._cvs_moves.update()

    def _clear_moves(self):
        for child in self._frm_moves_index.winfo_children():
            child.destroy()

        for child in self._frm_moves_black.winfo_children():
            child.destroy()

        for child in self._frm_moves_white.winfo_children():
            child.destroy()

        self._cvs_moves.yview_moveto(0.0)
        self._cvs_moves.xview_moveto(0.0)

    def _setup_position_and_play_moves(self, position: str, moves: list[str]):
        self._board.reset(position)

        for i, move in enumerate(moves):
            self._record_move(move, self._board.get_turn(), i // 2 + 1)
            self._board.play_move(move)

        self._board.redraw()

    def _check_control_buttons_for_previous(self):
        assert self._game is not None

        if self._move_index == 0:
            self._btn_previous.config(state="disabled")

        if self._move_index < len(self._game.moves):
            self._btn_next.config(state="normal")

    def _check_control_buttons_for_next(self):
        assert self._game is not None

        if self._move_index == len(self._game.moves):
            self._btn_next.config(state="disabled")

        if self._move_index > 0:
            self._btn_previous.config(state="normal")

    def _disable_control_buttons(self):
        self._btn_previous.config(state="disabled")
        self._btn_next.config(state="disabled")

    def _enable_control_buttons(self):
        self._btn_previous.config(state="normal")
        self._btn_next.config(state="normal")
