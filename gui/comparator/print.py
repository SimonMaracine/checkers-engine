import enum
from typing import Callable

import colorama


class Color(enum.Enum):
    Red = enum.auto()
    White = enum.auto()
    Cyan = enum.auto()


def print_status(value: str, indent: int = 0, end: str = "\n"):
    match indent:
        case 0:
            indentation = ""
        case 1:
            indentation = "  "
        case 2:
            indentation = "    "
        case 3:
            indentation = "      "
        case _:
            indentation = ""

    print(indentation + value, end=end, flush=True)


def _colorize(print_function: Callable, color: Color) -> Callable:
    def wrapper(value: str, indent: int = 0, end: str = "\n"):
        match color:
            case Color.Red:
                term_color = colorama.Fore.RED
            case Color.White:
                term_color = colorama.Fore.WHITE
            case Color.Cyan:
                term_color = colorama.Fore.CYAN

        print(term_color, end="")
        print_function(value, indent, end)
        print(colorama.Fore.RESET, end="")

    return wrapper


print_status_red = _colorize(print_status, Color.Red)
print_status_white = _colorize(print_status, Color.White)
print_status_cyan = _colorize(print_status, Color.Cyan)
