# theDODIK3D

Wolfenstein-стайл raycasting-игра на чистом C17, Windows, SDL3.

---

## Сборка

### Через Visual Studio 2022
Открыть `theDODIK3D.sln`, выбрать конфигурацию **Debug|x64** или **Release|x64**, нажать **Ctrl+Shift+B**.

### Через скрипт `build.bat`
```
build.bat
```
Скрипт ищет MSBuild в стандартных путях VS 2022 и собирает конфигурацию **Debug|x64**.

### Выходной бинарник
`game\x64\Debug\theDODIK3D.exe` (или `Release`).
Post-build event автоматически копирует `assets/` и `SDL3.dll` в папку с бинарником.

---

## Структура проекта

```
src/
  game/     — точка входа (SDL callbacks), инициализация
  engine/   — DDA raycasting
  render/   — software framebuffer, рисование стен/спрайтов/шрифтов
  core/     — world, player, entity, timer, input
  assets/   — загрузка текстур (assets.c) и шрифтов (font.c)
  ui/       — HUD (додик, статы, отладка)
  math/     — Vec2, углы, коллизии
assets/
  textures/ — .DIHT-файлы текстур и шрифтов
  worlds/   — .DIHT-файлы карт мира
PNGConverter/ — конвертер PNG → .DIHT / .DIHA
```

---

## Форматы файлов

### TextureMap (.DIHT)
```
[textures_count : u32]
Для каждой текстуры:
  [w : u32][h : u32][pixels : w * h * 4 байта, ARGB8888]
```

### FontMap (.DIHT)
То же, что TextureMap, но содержит ровно одну текстуру —
единую битмапу всех символов, расположенных в сетке слева направо:
строка 0: `0`…`9`, строка 1: `A`…`J`, строка 2: `K`…`T`, строка 3: `U`…`Z`.
Каждый символ занимает `FONT_CHAR_SIZE × FONT_CHAR_SIZE` (64 × 64) пикселей.
Пиксели хранятся построчно (raw, ARGB8888).

### WorldMap (.DIHT)
Так же, только пиксели каждого тайла кодируют тип стены по палитре (`world_palette.h`):
- `0xFF000000` (чёрный) → стена
- `0xFFFF0000` (красный) → дверь
- `0x00000000` (прозрачный) → воздух

---

## Конвертация ассетов

```bash
python PNGConverter/PNGConverter.py
```
Выбрать PNG-файл(ы), скрипт создаёт `.DIHT` рядом с ними.
Требует `PNGConverter/lib/conv.dll`.

---

## Changelog

### v0.5 (feature/v0.5)

**Модуль ассетов — переработка (`src/assets/`)**

- **`assets.h` / `assets.c`** — полностью переписан загрузчик TextureMap:
  - Исправлен парсинг `.DIHT`: теперь корректно читает `textures_count`,
    затем для каждой текстуры читает `[w][h][pixels]` (формат задокументирован выше).
  - Исправлена утечка памяти: `assets_free()` теперь освобождает как сам `TextureMap`,
    так и все `Texture*` внутри него.
  - Убраны устаревшие параметры `texture_w / texture_h` из сигнатуры
    `assets_load_texmap` — размеры берутся напрямую из файла.
  - Добавлен надёжный rollback при ошибке чтения (освобождаются частично загруженные текстуры).
  - `assets_free()` теперь вызывает `assets_font_free()` для очистки шрифтов.

- **`font.h` / `font.c`** — новый модуль для работы с битмапами шрифтов:
  - Структура `FontMap`: единая ARGB8888-битмапа с символами 0–9, A–Z в сетке.
  - Структура `FontChar`: «вид» на символ внутри битмапы (указатель + stride).
  - `FontMapId` enum: `FONT_MAP_DEFAULT`, `FONT_MAP_MINECRAFT`.
  - `assets_load_fontmap(path, id)` — загрузка шрифта из `.DIHT`.
  - `assets_get_font_char(id, ch)` — inline-доступ к символу без аллокаций.
  - `assets_font_index(c)` — маппинг символа на индекс в сетке.

**Рендерер (`src/render/`)**

- `render_draw_texture` теперь принимает `TextureMapId mid` + `TextureId tid`
  вместо одиночного `TextureId` — исправлена невозможность различать карты.
- `render_draw_font` теперь принимает `FontMapId mid` вместо `TextureId` —
  использует `assets_get_font_char()` вместо несуществующего `assets_get()`.
- `render_draw_wall` и `render_draw_entities` исправлены:
  вместо несуществующего `assets_get()` используют `assets_get_texture(MAP, ID)`.

**UI (`src/ui/`)**

- `ui.c`: все вызовы `render_draw_texture` дополнены корректным `TextureMapId`
  (TEXTURE_MAP_WEAPON для оружия, TEXTURE_MAP_DODIK для лица, TEXTURE_MAP_UI для фона).
- `ui_stats.c`: `TEXTURE_FONT_MINECRAFT` → `FONT_MAP_MINECRAFT`, добавлен
  `#include "assets/font.h"`.

**Инициализация (`src/game/game_bootstrap.c`)**

- Убран несуществующий `assets_load_texture()` — теперь все ресурсы грузятся
  через `assets_load_texmap` / `assets_load_fontmap`.
- Добавлена загрузка шрифтов (font/default, font/minecraft).

**Проект**

- `theDODIK3D.vcxproj` + `.vcxproj.filters`: добавлены `font.c` и `font.h`.
- `build.bat`: скрипт быстрой сборки через MSBuild без открытия Visual Studio.
- `lib\x64\SDL3.lib` + `lib\x64\SDL3.dll`: добавлены зависимости SDL3 (lib была утеряна —
  восстановлена через `dumpbin /EXPORTS` + `lib.exe /DEF`).

**Исправлены ошибки компиляции**

- `assets.h`: добавлены `TEXTURE_TILE_MARKARYAN` и `TEXTURE_TILE_SAND` в `TextureId` —
  использовались в `world_palette.c` но отсутствовали в enum.
- `ui_dodik.h`: сигнатура `ui_dodik_update` исправлена с `Player*` на `const Player*`
  (несоответствие с реализацией в `ui_dodik.c`).
