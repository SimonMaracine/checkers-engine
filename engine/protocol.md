# Protocol

Communication is done through IO streams on stdin and stdout files, through ASCII text messages.

All messages received must end with a newline (the "\n" character).

Messages from GUI to engine are also called commands.

The string of a command can contain arbitrary whitespace around tokens.

If a received command is invalid in any way, it should be ignored completely and the receiver may respond with
the message **WARNING**.

Positions and moves are encoded as FEN strings using the Portable Draughts Notation format.

## GUI -> Engine

### INIT

Tells the engine to get ready to play.

Must be called at the beginning and only once.

### NEWGAME [start position] [setup moves]

Tells the engine to prepare for a new game. It is not necessary to send this command right before the first game
(right after **INIT**), as if **INIT** automatically calls **NEWGAME**. Optionally tells it to start from a specific
position and then play the setup moves.

### MOVE \<move string\>

Tells the engine to play the move on the internal board.

It is GUI's responsability to send correct move commands. The engine is not obligated to do error checking. If it
does check for invalid move commands, it should immediately respond with the message **WARNING**.

### GO [don't play move]

Tells the engine to think and return the best move of its current internal position. It should optionally not play
the resulted move on its internal board.

### SETPARAMETER \<parameter name\> \<parameter value\>

Tells the engine to set the parameter to that value.

### GETPARAMETER \<parameter name\>

Asks the engine for that parameter value.

### QUIT

Tells the engine to shut down and exit gracefully.

## Engine -> GUI

### WARNING [warning or error message]

Informs the GUI that it couldn't understand the last command, or it was invalid, or something went wrong. It can
optionally contain an error message.

### BESTMOVE \<move string\>

Responds with the best move calculated after a **GO** command.
