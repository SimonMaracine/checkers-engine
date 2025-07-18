#! /usr/bin/env python3

import sys
import tkinter as tk

from replayer import main_window


def main(args: list[str]) -> int:
    root = tk.Tk()
    window = main_window.MainWindow(root)
    root.mainloop()

    return window.code()


if __name__ == "__main__":
    sys.exit(main(sys.argv))
