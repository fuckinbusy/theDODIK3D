import os
import ctypes as c
import tkinter as tk
from tkinter import filedialog

EXTENSION_TEX = "DIHT"
EXTENSION_FONT = "DIHF"
EXTENSION_ANIM = "DIHA"

base_dir = os.path.abspath(".")
lib_dir = os.path.join(base_dir, "lib")
dll_path = os.path.join(lib_dir, "conv.dll")

dll = c.CDLL(dll_path)

dll.initialize.restype = None
dll.deinitialize.restype = None

dll.convert.argtypes = [c.c_char_p, c.c_char_p]
dll.convert.restype = c.c_int

dll.convert_many.argtypes = [c.POINTER(c.c_char_p), c.c_uint32, c.c_char_p]
dll.convert_many.restype = c.c_int


def export_dihf(png_paths: tuple[str], output_path: str):
    if len(png_paths) != 1:
        raise ValueError("DIHF поддерживает только одну текстуру (выберите один PNG).")

    result = dll.convert(png_paths[0].encode("utf-8"), output_path.encode("utf-8"))
    if result == 0:
        raise RuntimeError("Ошибка конвертации DIHF")


def export_diht(png_paths: tuple[str], output_path: str):
    encoded_paths = [path.encode("utf-8") for path in png_paths]
    inputs = (c.c_char_p * len(encoded_paths))(*encoded_paths)

    result = dll.convert_many(inputs, len(encoded_paths), output_path.encode("utf-8"))
    if result == 0:
        raise RuntimeError("Ошибка конвертации DIHT")


def main():
    dll.initialize()

    window = tk.Tk()
    window.withdraw()

    files = filedialog.askopenfilenames(
        title="Select multiple PNG",
        filetypes=[("PNG", "*.png"), ("All files", "*.*")],
    )

    target_type = input("Формат выходного файла (DIHT/DIHF/DIHA): ").strip().upper()

    if target_type == EXTENSION_ANIM:
        print("DIHA пока не реализовано")
        return

    if target_type not in (EXTENSION_TEX, EXTENSION_FONT):
        print("Неизвестный формат. Поддерживаются только DIHT и DIHF")
        return

    output_path = filedialog.asksaveasfilename(
        initialdir=base_dir + "..\\..",
        title="Choose output file",
        defaultextension=f".{target_type}",
        filetypes=[(target_type, f"*.{target_type}"), ("All files", "*.*")],
    )

    if not files or not output_path:
        print("Конвертация отменена")
        return

    if target_type == EXTENSION_TEX:
        export_diht(files, output_path)
    else:
        export_dihf(files, output_path)

    print(f"Finished: {output_path}")


if __name__ == "__main__":
    try:
        main()
    finally:
        dll.deinitialize()
    input()
