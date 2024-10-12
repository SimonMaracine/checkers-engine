import time
import sys
import pathlib
import tkinter as tk
import tkinter.messagebox
import tkinter.filedialog
from typing import Optional

import pygame as pyg

import fen_string_window
import board
import checkers_engine

# https://tkdocs.com/tutorial/canvas.html
# https://freesound.org/people/Samulis/sounds/375743/

# TODO
# valgrind --tool=callgrind --cache-sim=yes --dump-instr=yes --branch=yes
# kcachegrind
# perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-load-misses
# perf record -e cycles,instructions,L1-dcache-loads,L1-dcache-load-misses --call-graph=dwarf
# hotspot
# ldd

class MainWindow(tk.Frame):
    WHITE = "#c8c8c8"
    BLACK = "#503c28"
    HUMAN = 1
    COMPUTER = 2
    DEFAULT_BOARD_SIZE = 400
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
        self._move_index = 1
        self._engine = checkers_engine.CheckersEngine()
        self._stopped = True

        self._setup_widgets()

        self._board = board.CheckersBoard(self._on_piece_move, self._cvs_board)

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

        # Do this after all widgets are configured
        self.bind("<Configure>", self._on_window_resized)
        self._cvs_board.bind("<Button-1>", self._on_left_mouse_button_pressed)
        self._cvs_board.bind("<Button-3>", self._on_right_mouse_button_pressed)

    def _setup_widgets_menubar(self):
        men_player = tk.Menu(self)
        men_player.add_command(label="Start Engine", command=self._start_engine)
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
        self._frm_parameters.bind("<Configure>", lambda _: self._frame_parameters_configure())
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

        bar_moves = tk.Scrollbar(frm_moves, orient="vertical")  # TODO maybe a horizontal scrollbar will be needed
        bar_moves.pack(side="right", fill="y")

        self._cvs_moves = tk.Canvas(frm_moves, width=200, yscrollcommand=bar_moves.set)
        self._cvs_moves.pack(side="left", fill="both", expand=True)

        bar_moves.config(command=self._cvs_moves.yview)

        frm_moves_main = tk.Frame(self._cvs_moves)
        frm_moves_main.bind("<Configure>", lambda _: self._frame_moves_configure())
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

    def _on_left_mouse_button_pressed(self, event):
        self._board.press_square_left_button(self._get_square(event.x, event.y))

    def _on_right_mouse_button_pressed(self, event):
        self._board.press_square_right_button(self._get_square(event.x, event.y))

    def _start_engine(self):
        file_path = tkinter.filedialog.askopenfilename(parent=self, title="Start Engine")

        if file_path == ():
            return

        # This button can be used to reload an engine, if one is already running
        if self._engine.running():
            self._engine.send("QUIT")
            self._engine.stop()

        # Wait for the previous engine to stop; might be dangerous
        while self._engine.running():
            pass

        try:
            self._engine.start(file_path)
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            return

        if not self._wait_for_engine_to_start():
            return

        try:
            self._engine.send("INIT")
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            self._engine.stop(True)
            return

        # Reset any current game, as the new engine has no knowledge of it
        # Must be done here after INIT
        self._reset_position()

        self._clear_parameters()

        if not self._get_engine_parameters():
            return

        self._var_engine.set(f"{self.TXT_ENGINE} {pathlib.PurePath(file_path).name}")

        if self._var_player_black.get() == self.HUMAN:
            self._board.set_user_input(True)

        self._btn_black_human.config(state="active")
        self._btn_black_computer.config(state="active")
        self._btn_white_human.config(state="active")
        self._btn_white_computer.config(state="active")

        self._btn_stop.config(state="active")
        self._btn_continue.config(state="active")

    def _reset_position(self):
        self._board.reset()
        self._reset_engine()
        self._reset_status()
        self._clear_moves()

    def _set_position(self):
        top_level = tk.Toplevel(self)
        fen_string_window.FenStringWindow(top_level, self._set_position_string)

    def _set_position_string(self, string: str):
        self._board.reset(string)
        self._reset_engine(string)
        self._reset_status()
        self._clear_moves()

    def _show_indices(self):
        if not self._indices:
            self._draw_indices()
        else:
            self._cvs_board.delete("indices")

        self._indices = not self._indices

    def _exit_application(self):
        pyg.mixer.quit()

        if self._engine.running():
            self._engine.send("QUIT")
            self._engine.stop()

        self._tk.destroy()

    def _about(self):
        tkinter.messagebox.showinfo("About", "Checkers Player, an implementation of the game of checkers.")

    def _player_changed(self):
        match self._board.get_turn():
            case board.Player.Black:
                self._enable_or_disable_user_input(self._var_player_black)
            case board.Player.White:
                self._enable_or_disable_user_input(self._var_player_white)

    def _stop(self):
        # This button just stops the engine from thinking, forcing it to make a move
        # Changing players is not allowed mid-game, because the engine will be out of sync with the game

        if not self._stopped:
            self._stopped = True
            self._var_stopped.set(f"{self.TXT_STOPPED} {self._stopped}")

            try:
                self._engine.send("STOP")
            except checkers_engine.CheckersEngineError as err:
                print(err, file=sys.stderr)
                self._engine.stop(True)

    def _continue(self):
        # This button has one job: to control when the computer should begin or continue the game when it's their turn
        # At the beginning and every time the computer is stopped, the `stopped` flag is set

        if self._stopped:
            self._stopped = False
            self._var_stopped.set(f"{self.TXT_STOPPED} {self._stopped}")

            match self._board.get_turn():
                case board.Player.Black:
                    if self._var_player_black.get() == self.COMPUTER:
                        try:
                            self._engine.send("GO")
                        except checkers_engine.CheckersEngineError as err:
                            print(err, file=sys.stderr)
                            self._engine.stop(True)
                        else:
                            self._wait_for_engine_best_move()
                case board.Player.White:
                    if self._var_player_white.get() == self.COMPUTER:
                        try:
                            self._engine.send("GO")
                        except checkers_engine.CheckersEngineError as err:
                            print(err, file=sys.stderr)
                            self._engine.stop(True)
                        else:
                            self._wait_for_engine_best_move()

            # Do this here, even though it will be done after the move is played on the board, because the engine may
            # take quite some time to think
            self._btn_black_human.config(state="disabled")
            self._btn_black_computer.config(state="disabled")
            self._btn_white_human.config(state="disabled")
            self._btn_white_computer.config(state="disabled")

    def _frame_parameters_configure(self):
        self._cvs_parameters.config(scrollregion=self._cvs_parameters.bbox("all"))

    def _frame_moves_configure(self):
        self._cvs_moves.config(scrollregion=self._cvs_moves.bbox("all"))

    def _calculate_board_size(self) -> int:
        self._frm_left.update()  # This makes things work... I'm very happy that it does
        size = self._frm_left.winfo_geometry().split("+")[0].split("x")

        PADDING = 40

        return max(min(int(size[0]), int(size[1])) - PADDING, self.DEFAULT_BOARD_SIZE)

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

    def _record_move(self, move: board.Move, player: board.Player):
        match player:
            case board.Player.Black:
                tk.Label(self._frm_moves_index, text=f"{self._move_index}.").pack(anchor="nw")
                tk.Label(self._frm_moves_black, text=str(move)).pack(anchor="nw")
                self._move_index += 1
            case board.Player.White:
                tk.Label(self._frm_moves_white, text=str(move)).pack(anchor="nw")

        # Make canvas know that it just got new widgets
        self._cvs_moves.update()

        # Don't make the scrollbar jump for the first few widgets
        if self._move_index > 4:
            self._cvs_moves.yview_moveto(1.0)

    def _clear_moves(self):
        for child in self._frm_moves_index.winfo_children():
            child.destroy()

        for child in self._frm_moves_black.winfo_children():
            child.destroy()

        for child in self._frm_moves_white.winfo_children():
            child.destroy()

        self._move_index = 1
        self._cvs_moves.yview_moveto(0.0)

    def _clear_parameters(self):
        for child in self._frm_parameters.winfo_children():
            child.destroy()

    def _reset_engine(self, position: Optional[str] = None):
        try:
            if position is not None:
                self._engine.send(f"NEWGAME {position}")  # FIXME not working correctly
            else:
                self._engine.send("NEWGAME")
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            self._engine.stop(True)

    def _wait_for_engine_to_start(self) -> bool:
        # Return if the engine started successfully or not

        time_begin = time.time()

        while True:
            try:
                message = self._engine.receive()
            except checkers_engine.CheckersEngineError as err:
                print(err, file=sys.stderr)
                self._engine.stop(True)
                return False

            if "READY" in message:
                print("Engine started successfully", file=sys.stderr)
                return True

            time_now = time.time()

            if time_begin - time_now > 3.0:
                print("Engine failed to respond in a timely manner", file=sys.stderr)
                self._engine.stop(True)
                return False

    def _wait_for_engine_best_move(self):
        self.after(25, self._check_for_engine_best_move)

    def _check_for_engine_best_move(self):
        try:
            message = self._engine.receive()
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            self._engine.stop(True)
            return

        if "BESTMOVE" in message:
            if "none" in message:
                print("Engine thinks it's game over", file=sys.stderr)
            else:
                self._board.play_move(message.split()[1])

            return
        elif "INFO" in message:
            print(" ".join(message.split()[1:]), file=sys.stderr)

        self._wait_for_engine_best_move()

    def _wait_for_engine_parameters(self):
        self.after(25, self._check_for_engine_parameters)

    def _check_for_engine_parameters(self):
        try:
            message = self._engine.receive()
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            self._engine.stop(True)
            return

        if "PARAMETERS" in message:
            tokens = message.split()
            token_index = 1

            while token_index < len(tokens):
                self._get_engine_parameter(tokens[token_index])
                token_index += 1

            return

        self._wait_for_engine_parameters()

    def _get_engine_parameter(self, name: str):
        try:
            self._engine.send(f"GETPARAMETER {name}")
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            self._engine.stop(True)
            return

        self._wait_for_engine_parameter()

    def _wait_for_engine_parameter(self):
        self.after(25, self._check_for_engine_parameter)

    def _check_for_engine_parameter(self):
        try:
            message = self._engine.receive()
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            self._engine.stop(True)
            return

        if "PARAMETER" in message:
            tokens = message.split()
            name = tokens[1]
            type = tokens[2]
            value = tokens[3]

            match type:
                case "int":
                    self._add_engine_parameter_int(name, value)
                case "float":
                    self._add_engine_parameter_float(name, value)
                case "bool":
                    self._add_engine_parameter_bool(name, value)
                case "string":
                    self._add_engine_parameter_string(name, value)

            return

        self._wait_for_engine_parameter()

    def _get_engine_parameters(self) -> bool:
        # Return if the engine successfully started getting parameters or not

        try:
            self._engine.send("GETPARAMETERS")
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            self._engine.stop(True)
            return False

        self._wait_for_engine_parameters()
        return True

    def _add_engine_parameter_int(self, name: str, value: str):
        frm_parameter = self._engine_parameter_frame_label(name)

        var_parameter = tk.StringVar(frm_parameter, value=value)
        tk.Spinbox(
            frm_parameter,
            from_=-256,
            to=256,
            textvariable=var_parameter,
            command=lambda: self._set_engine_parameter(name, var_parameter.get())
        ).grid(row=0, column=1, sticky="ew")  # FIXME extend protocol to include possible values

    def _add_engine_parameter_float(self, name: str, value: str):
        frm_parameter = self._engine_parameter_frame_label(name)

        var_parameter = tk.StringVar(frm_parameter, value=value)
        tk.Entry(frm_parameter, textvariable=var_parameter).grid(row=0, column=1, sticky="ew")

        var_parameter.trace_add("write", lambda *args: self._set_engine_parameter(name, var_parameter.get()))

    def _add_engine_parameter_bool(self, name: str, value: str):
        frm_parameter = self._engine_parameter_frame_label(name)

        var_parameter = tk.IntVar(frm_parameter, value=value)
        tk.Checkbutton(
            frm_parameter,
            variable=var_parameter,
            command=lambda: self._set_engine_parameter(name, str(var_parameter.get()))
        ).grid(row=0, column=1, sticky="ew")

    def _add_engine_parameter_string(self, name: str, value: str):
        frm_parameter = self._engine_parameter_frame_label(name)

        var_parameter = tk.StringVar(frm_parameter, value=value)
        tk.Entry(frm_parameter, textvariable=var_parameter, ).grid(row=0, column=1, sticky="ew")

        var_parameter.trace_add("write", lambda *args: self._set_engine_parameter(name, var_parameter.get()))

    def _engine_parameter_frame_label(self, name: str) -> tk.Frame:
        frm_parameter = tk.Frame(self._frm_parameters)
        frm_parameter.pack(fill="x", expand=True, pady=(0, 8))
        frm_parameter.columnconfigure(1, weight=1)

        tk.Label(frm_parameter, text=name).grid(row=0, column=0, sticky="w")

        return frm_parameter

    def _set_engine_parameter(self, name: str, value: str):
        try:
            self._engine.send(f"SETPARAMETER {name} {value}")
        except checkers_engine.CheckersEngineError as err:
            print(err, file=sys.stderr)
            self._engine.stop(True)

    def _inform_engine_about_user_move(self, var_current_player: tk.IntVar, var_next_player: tk.IntVar, move: board.Move):
        if var_current_player.get() == self.HUMAN:
            if var_next_player.get() == self.COMPUTER:
                try:
                    self._engine.send(f"MOVE {move}")
                except checkers_engine.CheckersEngineError as err:
                    print(err, file=sys.stderr)
                    self._engine.stop(True)

            self._stopped = False
            self._var_stopped.set(f"{self.TXT_STOPPED} {self._stopped}")

    def _start_engine_thinking(self, var_next_player: tk.IntVar):
        if var_next_player.get() == self.COMPUTER:
            if not self._stopped:
                try:
                    self._engine.send("GO")
                except checkers_engine.CheckersEngineError as err:
                    print(err, file=sys.stderr)
                    self._engine.stop(True)
                else:
                    self._wait_for_engine_best_move()

    def _enable_or_disable_user_input(self, var_next_player: tk.IntVar):
        match var_next_player.get():
            case self.HUMAN:
                self._board.set_user_input(True)
            case self.COMPUTER:
                self._board.set_user_input(False)

    def _on_piece_move(self, move: board.Move):
        self._sound.play()
        self._update_status()
        self._record_move(move, board.CheckersBoard._opponent(self._board.get_turn()))

        # Match on the current player
        match board.CheckersBoard._opponent(self._board.get_turn()):
            case board.Player.Black:
                self._inform_engine_about_user_move(self._var_player_black, self._var_player_white, move)
            case board.Player.White:
                self._inform_engine_about_user_move(self._var_player_white, self._var_player_black, move)

        # Match on the next player
        match self._board.get_turn():
            case board.Player.Black:
                self._enable_or_disable_user_input(self._var_player_black)
                self._start_engine_thinking(self._var_player_black)
            case board.Player.White:
                self._enable_or_disable_user_input(self._var_player_white)
                self._start_engine_thinking(self._var_player_white)

        # Ensure that the game is locked, if it is over
        if self._board.get_game_over() != board.GameOver.None_:
            self._board.set_user_input(False)
            self._btn_stop.config(state="disabled")
            self._btn_continue.config(state="disabled")

        # Do this primarily for the user move
        self._btn_black_human.config(state="disabled")
        self._btn_black_computer.config(state="disabled")
        self._btn_white_human.config(state="disabled")
        self._btn_white_computer.config(state="disabled")
