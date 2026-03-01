import os
import ctypes as c
import tkinter as tk
from tkinter import filedialog

EXTENSION_TEX = "DIHT"
EXTENSION_FONT = "DIHF"
EXTENSION_ANIM = "DIHA"

base_dir = os.path.dirname(os.path.abspath(__file__))
lib_dir = os.path.join(base_dir, "lib")
dll_path = os.path.join(lib_dir, "conv.dll")

if not os.path.exists(dll_path):
    raise FileNotFoundError(f"conv.dll not found at {dll_path}")

dll = c.CDLL(dll_path)

dll.initialize.restype = c.c_int
dll.deinitialize.restype = None

dll.convert.argtypes = [c.c_char_p, c.c_char_p]
dll.convert.restype = c.c_int

dll.convert_many.argtypes = [c.POINTER(c.c_char_p), c.c_uint32, c.c_char_p]
dll.convert_many.restype = c.c_int


def export_dihf(png_paths: tuple[str, ...], output_path: str):
    if len(png_paths) != 1:
        raise ValueError("DIHF поддерживает только одну текстуру")

    result = dll.convert(png_paths[0].encode("utf-8"), output_path.encode("utf-8"))
    if result == 0:
        raise RuntimeError("Ошибка конвертации DIHF")


def export_diht(png_paths: tuple[str, ...], output_path: str):
    encoded_paths = [path.encode("utf-8") for path in png_paths]
    inputs = (c.c_char_p * len(encoded_paths))(*encoded_paths)

    result = dll.convert_many(inputs, len(encoded_paths), output_path.encode("utf-8"))
    if result == 0:
        raise RuntimeError("Ошибка конвертации DIHT")


def main():
    if dll.initialize() == 0:
        raise RuntimeError("Failed to initialize SDL")

    window = tk.Tk()
    window.withdraw()

    files = filedialog.askopenfilenames(
        title="Select PNG files",
        filetypes=[("PNG", "*.png"), ("All files", "*.*")]
    )

    if not files:
        print("Конвертация отменена")
        return

    target_type = input("Формат выходного файла (DIHT/DIHF/DIHA): ").strip().upper()

    if target_type == EXTENSION_ANIM:
        print("DIHA пока не реализовано")
        return

    if target_type not in (EXTENSION_TEX, EXTENSION_FONT):
        print("Неизвестный формат")
        return

    output_path = filedialog.asksaveasfilename(
        initialdir=os.path.abspath(os.path.join(base_dir, "..", "..")),
        title="Choose output file",
        defaultextension=f".{target_type}",
        filetypes=[(target_type, f"*.{target_type}"), ("All files", "*.*")]
    )

    if not output_path:
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