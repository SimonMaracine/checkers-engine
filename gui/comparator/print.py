def print_status(value: str, indent: int = 0, end_: str = "\n"):
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

    print(indentation + value, end=end_, flush=True)
