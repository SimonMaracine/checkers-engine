# Checkers Protocol

Communication is done through IO streams, usually stdin and stdout, through ASCII text messages.

All messages must end with a new line ("\n" character).

Messages from GUI to engine are also called *commands*.

The string of a message can contain arbitrary whitespace around tokens.

If a received command is invalid in any way, it should be ignored completely and the receiver (engine) may respond
with the message **ALERT**.

Messages can consist of any number of characters and, as a consequence, any number of tokens. Both the GUI and the
engine should account for messages of any size.

Positions and moves are encoded as FEN strings using the standard
[Portable Draughts Notation format](https://en.wikipedia.org/wiki/Portable_Draughts_Notation).

The engine must process the commands synchronously, in the order that they are read from the stream.

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
does check for invalid move commands, it is encouraged to immediately respond with the message **ALERT**.

### GO [don't play move]

Tells the engine to think, play and return the best move of its current internal position. It should optionally not
play the resulted move on its internal board, if the second token is equal to the string *dontplaymove*.

The GUI is not permitted to send the **GO** command while the engine is still thinking. It can only send another **GO**
command after it received a **BESTMOVE** message from engine.

The engine must not return from processing the **GO** command until it has a valid best move result.

### STOP

Tells the engine to stop thinking and return its best result calculated so far.

This command should do nothing, if the engine is not in the process of thinking.

The engine must have a valid result even if it was stopped from thinking very early.

### GETPARAMETERS

Asks the engine about its configurable parameters. The engine can have any number of parameters, even zero. They must
be initialized by the time the **INIT** command finishes processing.

The possible types are:

- **int**, a signed 32-bit integer
- **float**, a 32-bit floating point number
- **bool**, a boolean with values *true* or *false*
- **string**, an ASCII string of maximum 64 characters

The types must be spelled just like above.

Parameter names must be no longer than 64 ASCII characters.

The engine is encouraged to come with a small documentation that describes all of its parameters and their valid or
their recommended values.

### GETPARAMETER \<name\>

Asks the engine for that parameter value.

### SETPARAMETER \<name\> \<value\>

Tells the engine to set the parameter to that value. The value should pe parsed by the engine according to its type.

The GUI may first ask the engine about its parameters and their types in order to know which are available.

### QUIT

Tells the engine to shut down and exit gracefully.

This command should shut down the engine nicely, even if it is in the process of thinking.

## Engine -> GUI

### READY

Informs the GUI that the engine has started. Must be sent once at the very beginning. The GUI should wait for this
message before sending any other commands.

### ALERT \<warning or error message\>

Informs the GUI that it couldn't understand the last command, or it was invalid, or something went wrong.
It must contain a message.

The engine is free to never send this message to GUI. It exists solely as a debug facility. It can, however, send it
at any time, for any sensible reason. For example: it couldn't process a command, the requested parameter
doesn't exist, the parameter value is invalid etc.

### BESTMOVE \<move\>

Responds with the best move calculated after a **GO** command.

### PARAMETERS \<(name 1) (type 1) (name 2) (type 2) ...\>

Responds with a list of configurable parameters the engine offers after a **GETPARAMETERS** command. Each entry
consists of the parameter name and its type. The list can be empty, if the engine has no parameters.

### PARAMETER \<name\> \<value\>

Responds with the name and value of the requested parameter after a **GETPARAMETER** command.

### INFO [nodes \<value\>] [eval \<value\>] time \<value\>

Informs the GUI about its progress in calculating the best move. Can be sent at any time between the **GO**
command and the **BESTMOVE** response.

*nodes* represents the number of leaf nodes processed in the minimax algorithm. This value is optional,
as the engine may not use the minimax algorithm.

*eval* represents how much advantage does the current player have. It is implementation defined and optional.

*time* represents the total elapsed time since the thinking algorithm started (be it minimax or anything else).
