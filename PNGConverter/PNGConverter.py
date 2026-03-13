import os
import ctypes as c
import tkinter as tk
from tkinter import filedialog

EXT_TEXTURE = "DIHT"
EXT_FONT = "DIHF"
EXT_ANIM = "DIHA"

base_dir = os.path.abspath(".")
lib_dir = os.path.join(base_dir, "lib")
dll_path = os.path.join(lib_dir, "conv.dll")

dll = c.CDLL(dll_path)
dll.initialize.restype = None
dll.deinitialize.restype = None
dll.convert.argtypes = [c.c_char_p, c.c_char_p]
dll.convert.restype = None

def main():
    window = tk.Tk()
    window.withdraw()

    ext = input("Select file extension: DIHT/DIHF/DIHA: ")
    if ext == "DIHT" or ext == "1":
        ext = EXT_TEXTURE
    elif ext == "DIHF" or "2":
        ext = EXT_FONT
    elif ext == "DIHA" or "3":
        ext = EXT_ANIM
    else:
        print("Unknown file format")
        return

    files = filedialog.askopenfilenames(
        title="Select multiple PNG",
        filetypes=[("PNG", "*.png"), ("All files", "*.*")]
    )

    if len(files) == 0:
        print("Files not selected")
        return

    folder = filedialog.askdirectory(
        initialdir=base_dir + "..\\..\\assets\\", 
        title="Choose folder to save files", 
        mustexist=True
    )

    if len(files) < 1:
        print("Directory not selected")
        return

    for path in files:
        output_file_name = path.split("/")[-1].split(".")[0]
        output_path = f"{folder}/{output_file_name}.{ext}"
        dll.convert(path.encode("utf-8"), output_path.encode("utf-8"))

    print("Finished")

if __name__ == "__main__":
    dll.initialize()
    main()
    dll.deinitialize()
    input()