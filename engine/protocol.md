# Checkers Protocol

Communication is done through IO streams on stdin and stdout files, through ASCII text messages.

All messages received must end with a new line ("\n" character).

Messages from GUI to engine are also called *commands*.

The string of a message can contain arbitrary whitespace around tokens.

If a received command is invalid in any way, it should be ignored completely and the receiver (engine) may respond
with the message **WARNING**.

Messages can consist of any number of characters and, as a consequence, any number of tokens. Both the GUI and the
engine should account for messages of any size.

Positions and moves are encoded as FEN strings using the standard
[Portable Draughts Notation format](https://en.wikipedia.org/wiki/Portable_Draughts_Notation).

## GUI -> Engine

### INIT

Tells the engine to initialize itself and get ready to play.

Must be sent at the beginning and only once.

### NEWGAME [start position] [(setup move 1) (setup move 2) ...]

Tells the engine to prepare for a new game. It is not necessary to send this command right before the first game
(right after **INIT**), as if **INIT** automatically calls **NEWGAME**. Optionally tells it to start from a
specific position or/and play the setup moves.

### MOVE \<move\>

Tells the engine to play the move on the internal board.

It is GUI's responsability to send correct move commands. The engine is not obligated to do error checking. If it
does check for invalid move commands, it should immediately respond with the message **WARNING**.

### GO [don't play move]

Tells the engine to think and return the best move of its current internal position. It should optionally not play
the resulted move on its internal board, if the second token is equal to the string *dontplaymove*.

### GETPARAMETERS

Asks the engine about its configurable parameters. The engine can have any number of parameters, even zero. They must
be initialized by the time the **INIT** command finishes processing.

The possible types are:

- **int**, a signed 32-bit integer
- **float**, a 32-bit floating point number
- **bool**, a boolean with values *true* or *false*
- **string**, a string of maximum 64 characters

The types must be spelled just like above.

### GETPARAMETER \<name\>

Asks the engine for that parameter value.

### SETPARAMETER \<name\> \<value\>

Tells the engine to set the parameter to that value. The value should pe parsed by the engine according to its type.

The GUI may first ask the engine about its parameters and their types in order to know which are available.

### QUIT

Tells the engine to shut down and exit gracefully.

## Engine -> GUI

### WARNING [warning or error message]

Informs the GUI that it couldn't understand the last command, or it was invalid, or something went wrong. It can
optionally contain a message.

### BESTMOVE \<move\>

Responds with the best move calculated after a **GO** command.

### PARAMETERS \<(name 1) (type 1) (name 2) (type 2) ...\>

Responds with a list of configurable parameters the engine offers after a **GETPARAMETERS** command. Each entry
consists of the parameter name and its type.

### PARAMETER \<name\> \<value\>

Responds with the name and value of the requested parameter after a **GETPARAMETER** command.
