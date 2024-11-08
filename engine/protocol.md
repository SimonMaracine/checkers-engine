# Checkers Protocol

Communication is done through IO streams, usually stdin and stdout, through ASCII text messages.

All messages must end with a new line ("\n" character).

Messages from GUI to engine are also called *commands*.

A message can contain arbitrary whitespace around tokens.

If a received command is invalid in any way, it should be ignored completely by the engine. Exception:
move and position strings may not be validated.

Messages can consist of any number of characters and, as a consequence, any number of tokens. Both the GUI and the
engine should account for messages of any size.

Positions and moves are encoded as FEN strings using the standard
[Portable Draughts Notation format](https://en.wikipedia.org/wiki/Portable_Draughts_Notation).

The engine must be able to process commands at any time. They should be processed synchronously, in the order
that they are read from the stream.

## GUI -> Engine

### INIT

Tells the engine to initialize itself and get ready to play.

Must be sent at the beginning and only once. Only the **QUIT** command may be sent before **INIT**.

### NEWGAME [`start position`] [`setup move 1` `setup move 2` ...]

Tells the engine to prepare for a new game. It is not necessary to send this command right before the first game
(right after **INIT**), as if **INIT** automatically calls **NEWGAME**. Optionally tells it to start from a
specific position and play the setup moves.

It is GUI's responsability to send valid positions. The engine is not obligated to do error checking.

### MOVE `move`

Tells the engine to play the move on the internal board.

It is GUI's responsability to send valid moves. The engine is not obligated to do error checking.

### GO [dontplaymove]

Tells the engine to think, play and return the best move of its current internal position. It should optionally not
play the resulted move on its internal board, if the second token is equal to the string *dontplaymove*.

The GUI is not permitted to send the **GO** command while the engine is still thinking. It can only send another
**GO** command after it received a **BESTMOVE** message from engine.

The engine must not return from processing the **GO** command until it has a valid best move result.

### STOP

Tells the engine to stop thinking and return its best result calculated so far.

This command should do nothing, if the engine is not in the process of thinking.

The engine must have a valid result even if it was stopped from thinking very early.

### GETPARAMETERS

Asks the engine about its configurable parameters. The engine can have any number of parameters, even zero.
They must be initialized by the time the **INIT** command finishes processing.

The possible types are:

- **int**, a signed 32-bit integer
- **float**, a 32-bit floating point number
- **bool**, a boolean with values *true* and *false*
- **string**, an ASCII string of maximum 10 characters, with no spaces

The types must be spelled just like above.

Parameter names must be no longer than 10 ASCII characters.

The engine is encouraged to come with a small documentation that describes all of its parameters and their valid or
their recommended values.

### GETPARAMETER `name`

Asks the engine for that parameter value and type.

### SETPARAMETER `name` `value`

Tells the engine to set the parameter to that value. The value should pe parsed by the engine according to its type.

The GUI may first ask the engine about its parameters and their types in order to know which are available.

### QUIT

Tells the engine to shut down and exit gracefully.

This command should shut down the engine nicely, even if it is in the process of thinking.

### GETNAME

Asks the engine for its name.

## Engine -> GUI

### READY

Informs the GUI that the engine has started. Must be sent once at the very beginning. The GUI should wait for this
message before sending any other commands.

### BESTMOVE (`move` | none)

Responds with the best move calculated after a **GO** command, or with the string *none*, if the game is over.

### PARAMETERS (`name 1` `name 2` ...)

Responds with a list of configurable parameters the engine offers after a **GETPARAMETERS** command.
The list may be empty, if the engine has no parameters.

### PARAMETER `name` `type` `value`

Responds with the name, type and value of the requested parameter after a **GETPARAMETER** command.

### INFO [nodes `value`] [eval `value`] time `value`

Informs the GUI about its progress in calculating the best move. Can be sent at any time between the **GO**
command and the **BESTMOVE** response.

*nodes* represents the number of leaf nodes processed in the minimax algorithm. This value is optional,
as the engine may not use the minimax algorithm.

*eval* represents how much advantage does the current player have. It is an implementation defined signed integer.
It is optional.

*time* represents the total elapsed time in seconds as a floating point number since the thinking algorithm
started.

### NAME `name`

Responds with the name after a **GETNAME** command.

The name is an implementation-defined string of maximum 15 ASCII characters with no spaces. It may contain the
engine's actual name shortened plus its version.
