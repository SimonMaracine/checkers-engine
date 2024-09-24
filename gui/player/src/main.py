import tkinter as tk

import main_window

def main(args: list[str]) -> int:
    root = tk.Tk()
    window = main_window.MainWindow(root)
    root.mainloop()

    return window.code()
