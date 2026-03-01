"""
pack_texmap.py — упаковывает набор одиночных .DIHT файлов в TextureMap .DIHT.

Формат одиночного .DIHT (старый, создаётся PNGConverter):
    [w : u32][h : u32][pixels : w*h*4 байта ARGB8888]

Формат TextureMap .DIHT (новый):
    [textures_count : u32]
    для каждой текстуры:
        [w : u32][h : u32][pixels : w*h*4 байта ARGB8888]

Использование:
    python tools/pack_texmap.py
"""

import struct
import os

# Корень проекта — директория, содержащая assets/
PROJ_ROOT = os.path.join(os.path.dirname(__file__), "..")
ASSETS    = os.path.join(PROJ_ROOT, "assets", "textures")


def read_single_diht(path: str) -> bytes:
    """Читает одиночный .DIHT и возвращает [w][h][pixels] как байты."""
    with open(path, "rb") as f:
        data = f.read()
    w, h = struct.unpack_from("<II", data, 0)
    expected = 8 + w * h * 4
    if len(data) != expected:
        raise ValueError(f"{path}: ожидалось {expected} байт, получено {len(data)}")
    return data  # [w][h][pixels]


def pack_texmap(output_path: str, *source_paths: str) -> None:
    """
    Упаковывает одиночные .DIHT в TextureMap .DIHT.
    source_paths — пути к файлам в нужном порядке (индекс = TextureId).
    """
    chunks = []
    for p in source_paths:
        if not os.path.isfile(p):
            print(f"  [WARN] файл не найден: {p}")
            # вставляем пустую заглушку 1×1
            chunks.append(struct.pack("<II", 1, 1) + b"\x00" * 4)
        else:
            chunks.append(read_single_diht(p))

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "wb") as f:
        f.write(struct.pack("<I", len(chunks)))  # textures_count
        for chunk in chunks:
            f.write(chunk)

    total = sum(len(c) for c in chunks)
    print(f"  -> {output_path}  ({len(chunks)} текстур, {total} байт пикселей)")


# ─────────────────────────────────────────────────────────────────────────────
# TEXTURE_MAP_TILES
# Порядок строго соответствует TextureId enum в assets.h:
#   0=DEFAULT, 1=BRICKS, 2=BRICKS_SECRET, 3=MARKARYAN, 4=SAND, 5=DOOR
# ─────────────────────────────────────────────────────────────────────────────
T = os.path.join(ASSETS, "tiles")
print("Упаковка tiles.DIHT ...")
pack_texmap(
    os.path.join(ASSETS, "tiles.DIHT"),
    os.path.join(T, "default_64.DIHT"),         # 0 TEXTURE_TILE_DEFAULT
    os.path.join(T, "bricks_64.DIHT"),           # 1 TEXTURE_TILE_BRICKS
    os.path.join(T, "bricks_secret_64.DIHT"),    # 2 TEXTURE_TILE_BRICKS_SECRET
    os.path.join(T, "markaryan_64.DIHT"),         # 3 TEXTURE_TILE_MARKARYAN
    os.path.join(T, "soul_sand_32.DIHT"),         # 4 TEXTURE_TILE_SAND
    os.path.join(T, "door_128.DIHT"),             # 5 TEXTURE_TILE_DOOR
)

# ─────────────────────────────────────────────────────────────────────────────
# TEXTURE_MAP_DODIK
# Порядок строго соответствует TextureId enum в assets.h:
#   0=IDLE 1=SIDE_LOOK 2=ANGRY 3=SIDE_LOOK_ANGRY 4=EXCITED 5=SURPRISED
#   6=IDLE_HIT 7=SIDE_LOOK_HIT 8=ANGRY_HIT 9=SIDE_LOOK_ANGRY_HIT
#   10=EXCITED_HIT 11=SURPRISED_HIT
#   12=DEAD 13=GRAY 14=HOLY 15=FUCKU 16=FRAME
# ─────────────────────────────────────────────────────────────────────────────
D = os.path.join(ASSETS, "ui", "dodik")
print("Упаковка dodik.DIHT ...")
pack_texmap(
    os.path.join(ASSETS, "dodik.DIHT"),
    os.path.join(D, "idle.DIHT"),                   #  0 IDLE
    os.path.join(D, "side_look.DIHT"),               #  1 SIDE_LOOK
    os.path.join(D, "angry.DIHT"),                   #  2 ANGRY
    os.path.join(D, "side_look_angry.DIHT"),          #  3 SIDE_LOOK_ANGRY
    os.path.join(D, "wow.DIHT"),                     #  4 EXCITED
    os.path.join(D, "wow_hit.DIHT"),                 #  5 SURPRISED  (заглушка)
    os.path.join(D, "idle_hit.DIHT"),                #  6 IDLE_HIT
    os.path.join(D, "side_look_hit.DIHT"),           #  7 SIDE_LOOK_HIT
    os.path.join(D, "angry_hit.DIHT"),               #  8 ANGRY_HIT
    os.path.join(D, "side_look_angry_hit.DIHT"),     #  9 SIDE_LOOK_ANGRY_HIT
    os.path.join(D, "sad.DIHT"),                     # 10 EXCITED_HIT (заглушка)
    os.path.join(D, "sad_hit.DIHT"),                 # 11 SURPRISED_HIT (заглушка)
    os.path.join(D, "dead.DIHT"),                    # 12 DEAD
    os.path.join(D, "gray.DIHT"),                    # 13 GRAY
    os.path.join(D, "holy.DIHT"),                    # 14 HOLY
    os.path.join(D, "fucku.DIHT"),                   # 15 FUCKU
    os.path.join(D, "frame.DIHT"),                   # 16 FRAME
)

# ─────────────────────────────────────────────────────────────────────────────
# TEXTURE_MAP_WEAPON
#   0=HAND_TEST, 1=SHOTGUN_TEST
# ─────────────────────────────────────────────────────────────────────────────
W = os.path.join(ASSETS, "ui", "dodik")
print("Упаковка weapon.DIHT ...")
pack_texmap(
    os.path.join(ASSETS, "weapon.DIHT"),
    os.path.join(W, "hand_test.DIHT"),   # 0 TEXTURE_WEAPON_HAND_TEST
    os.path.join(W, "gun_test.DIHT"),    # 1 TEXTURE_WEAPON_SHOTGUN_TEST
)

# ─────────────────────────────────────────────────────────────────────────────
# TEXTURE_MAP_UI
#   0=BACKGROUND
# ─────────────────────────────────────────────────────────────────────────────
U = os.path.join(ASSETS, "ui", "bg")
print("Упаковка ui.DIHT ...")
pack_texmap(
    os.path.join(ASSETS, "ui.DIHT"),
    os.path.join(U, "bg.DIHT"),    # 0 TEXTURE_UI_BACKGROUND
)

# ─────────────────────────────────────────────────────────────────────────────
# TEXTURE_MAP_ENEMY
#   0=ENEMY
# ─────────────────────────────────────────────────────────────────────────────
E = os.path.join(ASSETS, "entity")
print("Упаковка entity.DIHT ...")
pack_texmap(
    os.path.join(ASSETS, "entity.DIHT"),
    os.path.join(E, "enemy.DIHT"),   # 0 TEXTURE_ENTITY_ENEMY
)

# ─────────────────────────────────────────────────────────────────────────────
# Шрифты — одиночный DIHT без textures_count → оборачиваем в TextureMap
# ─────────────────────────────────────────────────────────────────────────────
def wrap_font(output_path: str, source_path: str) -> None:
    """
    Оборачивает старый одиночный DIHT (без textures_count) в TextureMap DIHT.
    Результат: [1][w][h][pixels]
    """
    if not os.path.isfile(source_path):
        print(f"  [WARN] шрифт не найден: {source_path}")
        return
    data = read_single_diht(source_path)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "wb") as f:
        f.write(struct.pack("<I", 1))  # textures_count = 1
        f.write(data)
    w, h = struct.unpack_from("<II", data)
    print(f"  -> {output_path}  (1 texture {w}x{h})")


F = os.path.join(ASSETS, "font")
print("Упаковка шрифтов ...")
wrap_font(os.path.join(F, "default_texmap.DIHT"),   os.path.join(F, "default.DIHT"))
wrap_font(os.path.join(F, "minecraft_texmap.DIHT"), os.path.join(F, "minecraft.DIHT"))

print("\nГотово! Запусти build.bat для пересборки.")
