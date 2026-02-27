import os
import ctypes as c
import tkinter as tk
from tkinter import filedialog

EXTENSION_TEX = "DIHT"
EXTENSION_ANIM = "DIHA"

base_dir = os.path.abspath(".")
lib_dir = os.path.join(base_dir, "lib")
dll_path = os.path.join(lib_dir, "conv.dll")

dll = c.CDLL(dll_path)

dll.initialize.restype = None
dll.deinitialize.restype = None

dll.convert.argtypes = [c.c_char_p, c.c_char_p]
dll.convert.restype = None

dll.initialize()

window = tk.Tk()
window.withdraw()

files = filedialog.askopenfilenames(
    title="Select multiple PNG",
    filetypes=[("PNG", "*.png"), ("All files", "*.*")]
)

folder = filedialog.askdirectory(initialdir=base_dir + "..\\..", title="Choose folder to save files", mustexist=True);

for path in files:
    output_file_name = path.split("/")[-1].split(".")[0]
    output_path = f"{folder}/{output_file_name}.{EXTENSION_TEX}"
    dll.convert(path.encode("utf-8"), output_path.encode("utf-8"))

print("Finished")
dll.deinitialize()
input()