WHITE = "#c8c8c8"
BLACK = "#4C3D2F"
DEFAULT_BOARD_SIZE = 400
TXT_STATUS = "Status:"
TXT_PLAYER = "Player:"
TXT_PLIES_WITHOUT_ADVANCEMENT = "Plies without advancement:"


def _1_32_to_0_64(index: int) -> int:
    if ((index - 1) // 4) % 2 == 0:
        return index * 2 - 1
    else:
        return (index - 1) * 2


def _0_64_to_1_32(index: int) -> int:
    if index % 2 == 1:
        return (index + 1) // 2
    else:
        return (index // 2) + 1


def _1_32_to_0_31(index: int) -> int:
    return index - 1


def _0_31_to_1_32(index: int) -> int:
    return index + 1
