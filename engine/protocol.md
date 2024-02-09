# Protocol

Communication is done through IO streams on stdin and stdout files, through ASCII text messages.

All messages received must end with a newline (the "\n" character).

Messages from GUI to engine are also called commands.

The string of a message can contain arbitrary whitespace around tokens.

If a received command is invalid in any way, it should be ignored completely and the receiver (engine) may respond with
the message **WARNING**.

Positions and moves are encoded as FEN strings using the standard
[Portable Draughts Notation format](https://en.wikipedia.org/wiki/Portable_Draughts_Notation).

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
the resulted move on its internal board, if the second token is equal to the string *dontplaymove*.

<!-- TODO what parameters must exist? -->
<!-- FIXME specify messages more rigorously -->

### GETPARAMETERS

Asks the engine about what its configurable parameters are. Possible types are:

- **int**, a signed 32-bit integer
<!-- - **bool**, a boolean with values *true* or *false* -->

The types must be spelled just like above.

<!-- TODO maybe string and float types -->

### GETPARAMETER \<parameter name\>

Asks the engine for that parameter value and type.

### SETPARAMETER \<parameter name\> \<parameter value\>

Tells the engine to set the parameter to that value. The value should pe parsed by the engine according to its type.
The GUI should first ask the engine about its parameters and their types.

### QUIT

Tells the engine to shut down and exit gracefully.

## Engine -> GUI

### WARNING [warning or error message]

Informs the GUI that it couldn't understand the last command, or it was invalid, or something went wrong. It can
optionally contain a message.

### BESTMOVE \<move string\>

Responds with the best move calculated after a **GO** command.

### PARAMETERS \<name 1\> \<type 1\> \<name 2\> \<type 2\>...

Responds with a list of configurable parameters the engine offers after a **GETPARAMETERS** command.

### PARAMETER \<parameter name\> \<parameter value\> \<parameter type\>

Responds with the name, value and type of the requested parameter after a **GETPARAMETER** command.
