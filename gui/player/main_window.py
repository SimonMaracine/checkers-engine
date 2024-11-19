import sys
import tkinter as tk
import tkinter.messagebox
import tkinter.filedialog

from common import base_main_window
from common import checkers_engine
from common import board
from common import common
from . import fen_string_window
from . import saved_data

# https://tkdocs.com/tutorial/canvas.html
# https://freesound.org/people/Samulis/sounds/375743/

# TODO
# valgrind --tool=callgrind --cache-sim=yes --dump-instr=yes --branch=yes
# kcachegrind
# perf stat -e cycles,instructions,L1-dcache-loads,L1-dcache-load-misses
# perf record -e cycles,instructions,L1-dcache-loads,L1-dcache-load-misses --call-graph=dwarf
# hotspot
# ldd


class MainWindow(base_main_window.BaseMainWindow):
    HUMAN = 1
    COMPUTER = 2
    CHECK_TIME_MS = 30
    WAIT_TIME_S = 3
    TXT_ENGINE = "Engine:"
    TXT_STOPPED = "Stopped:"

    def __init__(self, root: tk.Tk):
        self._stopped = True  # The user can stop the game (computer)

        super().__init__(root, "Checkers Player")

        self._board = board.CheckersBoard(self._on_piece_move, self._cvs_board)
        self._engine = checkers_engine.CheckersEngine()
        self._move_index = 1
        self._gui_game_over = False  # Set to true when the GUI says it's game over

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

        self._var_status = tk.StringVar(frm_status, common.TXT_STATUS + " game not started")

        lbl_status = tk.Label(frm_status, textvariable=self._var_status)
        lbl_status.pack(anchor="w")

        self._var_player = tk.StringVar(frm_status, common.TXT_PLAYER + " black")

        lbl_player = tk.Label(frm_status, textvariable=self._var_player)
        lbl_player.pack(anchor="w")

        self._var_plies_without_advancement = tk.StringVar(frm_status, common.TXT_PLIES_WITHOUT_ADVANCEMENT + " 0")

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

    def _exit_application(self):
        self._uninitialize_sound()

        if self._engine.running():
            try:
                self._engine.send("QUIT")
            except checkers_engine.CheckersEngineError as err:
                self._print_err_and_stop_engine(err, self._engine)
            else:
                self._engine.stop()

        self._tk.destroy()

    def _on_left_mouse_button_pressed(self, event):
        self._board.press_square_left_button(self._get_square(event.x, event.y))

    def _on_right_mouse_button_pressed(self, event):
        self._board.press_square_right_button()

    def _start_engine(self):
        if self._is_engine_busy() and not self._is_game_over():
            return

        saved_path = saved_data.load_engine_path()

        if saved_path is None:
            file_path = tkinter.filedialog.askopenfilename(parent=self, title="Start Engine")
        else:
            file_path = tkinter.filedialog.askopenfilename(parent=self, title="Start Engine", initialfile=saved_path)

        if file_path in ((), ""):  # Stupid
            return

        saved_data.save_engine_path(file_path)

        # This button can be used to reload an engine, if one is already running
        if self._engine.running():
            try:
                self._engine.send("QUIT")
            except checkers_engine.CheckersEngineError as err:
                self._print_err_and_stop_engine(err, self._engine)
                return

            self._engine.stop()

        # Any previous engine should be off by now

        try:
            self._engine.start(file_path)
        except checkers_engine.CheckersEngineError as err:
            self._print_err_and_stop_engine(err, self._engine)
            return

        if not self._wait_for_engine_to_start():
            return

        try:
            self._engine.send("INIT")
        except checkers_engine.CheckersEngineError as err:
            self._print_err_and_stop_engine(err, self._engine)
            return

        # Reset any current game, as the new engine has no knowledge of it
        # Must be done here after INIT
        self._reset_position()

        self._clear_gui_parameters()
        self._setup_gui_after_engine_start()

        self._get_engine_name()
        self._get_engine_parameters()

    def _reset_position(self):
        if not self._engine.running():
            return

        if self._is_engine_busy() and not self._is_game_over():
            return

        self._board.reset()
        self._reset_engine()
        self._reset_gui_status()
        self._clear_gui_moves()
        self._setup_gui_after_reset()

    def _set_position(self):
        top_level = tk.Toplevel(self)
        fen_string_window.FenStringWindow(top_level, self._set_position_string)

    def _set_position_string(self, string: str):
        if not self._engine.running():
            return

        if self._is_engine_busy() and not self._is_game_over():
            return

        self._board.reset(string)
        self._reset_engine(string)
        self._reset_gui_status()
        self._clear_gui_moves()
        self._setup_gui_after_reset()

    def _about(self):
        tkinter.messagebox.showinfo("About", "Checkers Player, an implementation of the game of checkers.")

    def _player_changed(self):
        match self._board.get_turn():
            case board.Player.Black:
                self._enable_or_disable_gui_user_input(self._var_player_black)
                self._enable_or_disable_gui_stop(self._var_player_black)
                self._enable_or_disable_gui_continue(self._var_player_black)
            case board.Player.White:
                self._enable_or_disable_gui_user_input(self._var_player_white)
                self._enable_or_disable_gui_stop(self._var_player_white)
                self._enable_or_disable_gui_continue(self._var_player_white)

    def _stop(self):
        # This button just stops the engine from thinking, forcing it to make a move
        # Changing players is not allowed mid-game, because the engine will be out of sync with the game

        if not self._stopped:
            self._stopped = True
            self._var_stopped.set(f"{self.TXT_STOPPED} {self._stopped}")

            try:
                self._engine.send("STOP")
            except checkers_engine.CheckersEngineError as err:
                self._print_err_and_stop_engine(err, self._engine)

            # This allows the game to continue
            match self._board.get_turn():
                case board.Player.Black:
                    self._enable_or_disable_gui_continue(self._var_player_black)
                case board.Player.White:
                    self._enable_or_disable_gui_continue(self._var_player_white)

    def _continue(self):
        # This button has one job: to control when the computer should begin or continue the game when it's their turn
        # At the beginning and every time the computer is stopped, the `stopped` flag is set

        if self._stopped:
            self._stopped = False
            self._var_stopped.set(f"{self.TXT_STOPPED} {self._stopped}")

            match self._board.get_turn():
                case board.Player.Black:
                    self._maybe_start_engine_thinking(self._var_player_black)
                case board.Player.White:
                    self._maybe_start_engine_thinking(self._var_player_white)

            # Do this here, even though it will be done after the move is played on the board, because the engine may
            # take quite some time to think
            self._btn_black_human.config(state="disabled")
            self._btn_black_computer.config(state="disabled")
            self._btn_white_human.config(state="disabled")
            self._btn_white_computer.config(state="disabled")

            # Pressing continue should be done only once
            self._btn_continue.config(state="disabled")

    def _setup_gui_after_reset(self):
        match self._board.get_turn():
            case board.Player.Black:
                self._enable_or_disable_gui_user_input(self._var_player_black)
                self._enable_or_disable_gui_stop(self._var_player_black)
                self._enable_or_disable_gui_continue(self._var_player_black)
            case board.Player.White:
                self._enable_or_disable_gui_user_input(self._var_player_white)
                self._enable_or_disable_gui_stop(self._var_player_white)
                self._enable_or_disable_gui_continue(self._var_player_black)
                self._record_gui_dummy_black_move()

        self._stopped = True
        self._var_stopped.set(f"{self.TXT_STOPPED} {self._stopped}")

        self._gui_game_over = False

        self._btn_black_human.config(state="normal")
        self._btn_black_computer.config(state="normal")
        self._btn_white_human.config(state="normal")
        self._btn_white_computer.config(state="normal")

    def _setup_gui_after_engine_start(self):
        match self._board.get_turn():
            case board.Player.Black:
                self._enable_or_disable_gui_user_input(self._var_player_black)
                self._enable_or_disable_gui_stop(self._var_player_black)
                self._enable_or_disable_gui_continue(self._var_player_black)
            case board.Player.White:
                self._enable_or_disable_gui_user_input(self._var_player_white)
                self._enable_or_disable_gui_stop(self._var_player_white)
                self._enable_or_disable_gui_continue(self._var_player_black)

        self._btn_black_human.config(state="normal")
        self._btn_black_computer.config(state="normal")
        self._btn_white_human.config(state="normal")
        self._btn_white_computer.config(state="normal")

    def _update_gui_status(self):
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

    def _reset_gui_status(self):
        self._var_status.set(common.TXT_STATUS + " game not started")
        self._var_player.set(common.TXT_PLAYER + " black")
        self._var_plies_without_advancement.set(common.TXT_PLIES_WITHOUT_ADVANCEMENT + " 0")

    def _record_gui_move(self, move: board.Move, player: board.Player):
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

    def _record_gui_dummy_black_move(self):
        assert self._move_index == 1

        tk.Label(self._frm_moves_index, text=f"{self._move_index}.").pack(anchor="nw")
        tk.Label(self._frm_moves_black, text="--/--").pack(anchor="nw")
        self._move_index += 1

        # Make canvas know that it just got new widgets
        self._cvs_moves.update()

    def _clear_gui_moves(self):
        for child in self._frm_moves_index.winfo_children():
            child.destroy()

        for child in self._frm_moves_black.winfo_children():
            child.destroy()

        for child in self._frm_moves_white.winfo_children():
            child.destroy()

        self._move_index = 1
        self._cvs_moves.yview_moveto(0.0)
        self._cvs_moves.xview_moveto(0.0)

    def _clear_gui_parameters(self):
        for child in self._frm_parameters.winfo_children():
            child.destroy()

    def _get_square(self, x: int, y: int) -> int:
        square_size = int(self._cvs_board["width"]) // 8

        file = x // square_size
        rank = y // square_size

        return rank * 8 + file

    def _reset_engine(self, position: str | None = None):
        try:
            if position is not None:
                self._engine.send(f"NEWGAME {position}")
            else:
                self._engine.send("NEWGAME")
        except checkers_engine.CheckersEngineError as err:
            self._print_err_and_stop_engine(err, self._engine)

    def _wait_for_engine_to_start(self) -> bool:
        # Wait in a tight loop
        # Return if the engine started successfully or not

        try:
            common.wait_for_engine_to_start(self._engine, self.WAIT_TIME_S)
        except common.EngineWaitError as err:
            self._print_err_and_stop_engine(err, self._engine)
            return False

        return True

    def _async_wait_for_engine_best_move(self):
        def check_for_engine_best_move():
            try:
                message = self._engine.receive()
            except checkers_engine.CheckersEngineError as err:
                self._print_err_and_stop_engine(err, self._engine)
                return

            if "BESTMOVE" in message:
                if "none" in message:
                    if not self._gui_game_over:
                        raise RuntimeError("GUI didn't detect game over")

                    print("Engine says it's game over", file=sys.stderr)
                else:
                    if self._gui_game_over:
                        raise RuntimeError("GUI detected game over")

                    self._board.play_move(message.split()[1])

                return
            elif "INFO" in message:
                print(" ".join(message.split()[1:]), file=sys.stderr)

            self._async_wait_for_engine_best_move()

        self.after(self.CHECK_TIME_MS, check_for_engine_best_move)

    def _get_engine_parameters(self):
        try:
            self._engine.send("GETPARAMETERS")
        except checkers_engine.CheckersEngineError as err:
            self._print_err_and_stop_engine(err, self._engine)
        else:
            self._wait_for_engine_parameters()

    def _wait_for_engine_parameters(self):
        while True:
            try:
                message = self._engine.receive(1.0)
            except checkers_engine.CheckersEngineError as err:
                self._print_err_and_stop_engine(err, self._engine)
                return

            if "PARAMETERS" in message:
                tokens = message.split()
                token_index = 1

                while token_index < len(tokens):
                    self._get_engine_parameter(tokens[token_index])
                    token_index += 1

                return

    def _get_engine_parameter(self, name: str):
        try:
            self._engine.send(f"GETPARAMETER {name}")
        except checkers_engine.CheckersEngineError as err:
            self._print_err_and_stop_engine(err, self._engine)
        else:
            self._wait_for_engine_parameter()

    def _wait_for_engine_parameter(self):
        while True:
            try:
                message = self._engine.receive(1.0)
            except checkers_engine.CheckersEngineError as err:
                self._print_err_and_stop_engine(err, self._engine)
                return

            if "PARAMETER" in message:
                tokens = message.split()
                name = tokens[1]
                type = tokens[2]
                value = tokens[3]

                match type:
                    case "int":
                        self._add_gui_parameter_int(name, value)
                    case "float":
                        self._add_gui_parameter_float(name, value)
                    case "bool":
                        self._add_gui_parameter_bool(name, value)
                    case "string":
                        self._add_gui_parameter_string(name, value)

                return

    def _get_engine_name(self):
        try:
            self._engine.send("GETNAME")
        except checkers_engine.CheckersEngineError as err:
            self._print_err_and_stop_engine(err, self._engine)
        else:
            self._wait_for_engine_name()

    def _wait_for_engine_name(self):
        while True:
            try:
                message = self._engine.receive(1.0)
            except checkers_engine.CheckersEngineError as err:
                self._print_err_and_stop_engine(err, self._engine)
                return

            if "NAME" in message:
                self._var_engine.set(f"{self.TXT_ENGINE} {message.split()[1]}")
                return

    def _add_gui_parameter_int(self, name: str, value: str):
        frm_parameter = self._make_gui_parameter_frame_label(name)

        var_parameter = tk.StringVar(frm_parameter, value=value)
        tk.Spinbox(
            frm_parameter,
            from_=-256,
            to=256,
            textvariable=var_parameter,
            command=lambda: self._set_engine_parameter(name, var_parameter.get())
        ).grid(row=0, column=1, sticky="ew")  # FIXME extend protocol to include possible values

    def _add_gui_parameter_float(self, name: str, value: str):
        frm_parameter = self._make_gui_parameter_frame_label(name)

        var_parameter = tk.StringVar(frm_parameter, value=value)
        tk.Entry(frm_parameter, textvariable=var_parameter).grid(row=0, column=1, sticky="ew")

        var_parameter.trace_add("write", lambda *args: self._set_engine_parameter(name, var_parameter.get()))

    def _add_gui_parameter_bool(self, name: str, value: str):
        frm_parameter = self._make_gui_parameter_frame_label(name)

        var_parameter = tk.IntVar(frm_parameter, value=int(value))
        tk.Checkbutton(
            frm_parameter,
            variable=var_parameter,
            command=lambda: self._set_engine_parameter(name, str(var_parameter.get()))
        ).grid(row=0, column=1, sticky="ew")

    def _add_gui_parameter_string(self, name: str, value: str):
        frm_parameter = self._make_gui_parameter_frame_label(name)

        var_parameter = tk.StringVar(frm_parameter, value=value)
        tk.Entry(frm_parameter, textvariable=var_parameter, ).grid(row=0, column=1, sticky="ew")

        var_parameter.trace_add("write", lambda *args: self._set_engine_parameter(name, var_parameter.get()))

    def _make_gui_parameter_frame_label(self, name: str) -> tk.Frame:
        frm_parameter = tk.Frame(self._frm_parameters)
        frm_parameter.pack(fill="x", expand=True, pady=(0, 8))
        frm_parameter.columnconfigure(1, weight=1)

        tk.Label(frm_parameter, text=name).grid(row=0, column=0, sticky="w")

        return frm_parameter

    def _set_engine_parameter(self, name: str, value: str):
        try:
            self._engine.send(f"SETPARAMETER {name} {value}")
        except checkers_engine.CheckersEngineError as err:
            self._print_err_and_stop_engine(err, self._engine)

    def _maybe_inform_engine_about_user_move(self, var_current_player: tk.IntVar, var_next_player: tk.IntVar, move: board.Move):
        if var_current_player.get() == self.HUMAN:
            if var_next_player.get() == self.COMPUTER:
                try:
                    self._engine.send(f"MOVE {move}")
                except checkers_engine.CheckersEngineError as err:
                    self._print_err_and_stop_engine(err, self._engine)

            self._stopped = False
            self._var_stopped.set(f"{self.TXT_STOPPED} {self._stopped}")

    def _maybe_start_engine_thinking(self, var_next_player: tk.IntVar):
        if var_next_player.get() == self.COMPUTER:
            if not self._stopped:
                try:
                    self._engine.send("GO")
                except checkers_engine.CheckersEngineError as err:
                    self._print_err_and_stop_engine(err, self._engine)
                else:
                    self._async_wait_for_engine_best_move()

    def _enable_or_disable_gui_user_input(self, var_next_player: tk.IntVar):
        match var_next_player.get():
            case self.HUMAN:
                self._board.set_user_input(True)
            case self.COMPUTER:
                self._board.set_user_input(False)

    def _enable_or_disable_gui_stop(self, var_next_player: tk.IntVar):
        match var_next_player.get():
            case self.HUMAN:
                self._btn_stop.config(state="disabled")
            case self.COMPUTER:
                self._btn_stop.config(state="normal")

    def _enable_or_disable_gui_continue(self, var_next_player: tk.IntVar):
        match var_next_player.get():
            case self.HUMAN:
                self._btn_continue.config(state="disabled")
            case self.COMPUTER:
                self._btn_continue.config(state="normal")

    def _is_engine_busy(self) -> bool:
        if not self._engine.running():
            return False

        match self._board.get_turn():
            case board.Player.Black:
                match self._var_player_black.get():
                    case self.HUMAN:
                        return False
                    case self.COMPUTER:
                        return not self._stopped
            case board.Player.White:
                match self._var_player_white.get():
                    case self.HUMAN:
                        return False
                    case self.COMPUTER:
                        return not self._stopped

        assert False

    def _is_game_over(self) -> bool:
        return self._board.get_game_over() != board.GameOver.None_

    def _on_piece_move(self, move: board.Move):
        self._play_sound()
        self._update_gui_status()
        self._record_gui_move(move, board.opponent(self._board.get_turn()))

        # Match on the current player
        match board.opponent(self._board.get_turn()):
            case board.Player.Black:
                self._maybe_inform_engine_about_user_move(self._var_player_black, self._var_player_white, move)
            case board.Player.White:
                self._maybe_inform_engine_about_user_move(self._var_player_white, self._var_player_black, move)

        # Do this primarily for the user move
        self._btn_black_human.config(state="disabled")
        self._btn_black_computer.config(state="disabled")
        self._btn_white_human.config(state="disabled")
        self._btn_white_computer.config(state="disabled")

        # Ensure that the game is locked, if it is over
        if self._is_game_over():
            self._board.set_user_input(False)
            self._btn_stop.config(state="disabled")
            self._btn_continue.config(state="disabled")

            print("GUI says it's game over", file=sys.stderr)

            self._gui_game_over = True

            # Have the engine confirm that the game is over
            match self._board.get_turn():
                case board.Player.Black:
                    self._maybe_start_engine_thinking(self._var_player_black)
                case board.Player.White:
                    self._maybe_start_engine_thinking(self._var_player_white)

            return

        # Match on the next player
        match self._board.get_turn():
            case board.Player.Black:
                self._enable_or_disable_gui_user_input(self._var_player_black)
                self._maybe_start_engine_thinking(self._var_player_black)
            case board.Player.White:
                self._enable_or_disable_gui_user_input(self._var_player_white)
                self._maybe_start_engine_thinking(self._var_player_white)

    @staticmethod
    def _print_err_and_stop_engine(err: checkers_engine.CheckersEngineError | common.EngineWaitError, engine: checkers_engine.CheckersEngine):
        print(err, file=sys.stderr)
        engine.stop(True)
