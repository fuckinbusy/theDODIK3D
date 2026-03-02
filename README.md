# theDODIK3D

Wolfenstein-стайл рейкастинг 3D-игра на C17 с использованием SDL3.

## Сборка

Открыть `theDODIK3D.sln` в Visual Studio 2022 или запустить из корня проекта:

```bat
build.bat
```

Результат: `game\x64\Debug\theDODIK3D.exe`

---

## Changelog

### v0.7 — Рефакторинг (2026-03-02)

Фокус версии: устранение дублирования кода и улучшение расширяемости. Поведение игры не изменилось.

#### `src/render/render.h`
- Добавлены именованные константы вместо магических чисел:
  - `RENDER_SHADE_BASE` (1.5f) — база формулы затенения стен и плоскостей
  - `RENDER_CEILING_COLOR` — цвет потолка
  - `RENDER_FLOOR_COLOR` — цвет пола
  - `RENDER_SPRITE_CULL_DIST` (0.85f) — минимальная дистанция отсечки спрайтов
  - `RENDER_SPRITE_HEIGHT_SCALE` (0.9f) — масштаб высоты спрайта при проекции
  - `RENDER_SPRITE_OFFSET_SCALE` (0.1f) — вертикальное смещение центра спрайта
  - `RENDER_FOV_DEFAULT` (65) — угол обзора по умолчанию в градусах
- Добавлен inline-хелпер `render_has_alpha(u32 color)` — заменяет дублирующийся паттерн `(color >> 24) & 0xFF` в трёх местах рендерера

#### `src/render/render.c`
- **render_draw_wall**: стандартизировано вычисление texture_x через `gmath_clamp()` вместо ручных if-проверок (унификация с остальными функциями рендера)
- **render_draw_wall**: `1.5f` → `RENDER_SHADE_BASE`
- **render_draw_entities**: `0.85f` → `RENDER_SPRITE_CULL_DIST`, `1.5f` → `RENDER_SHADE_BASE`, `0.9f` → `RENDER_SPRITE_HEIGHT_SCALE`, `0.1f` → `RENDER_SPRITE_OFFSET_SCALE`
- **render_draw_entities**: альфа-проверка через `render_has_alpha()`
- **render_update**: `0xFF000000` → `RENDER_CEILING_COLOR`, `0xFFAAAAAA` → `RENDER_FLOOR_COLOR`; `1.5f` в таблице затенения → `RENDER_SHADE_BASE`
- **render_draw_texture**: альфа-проверка через `render_has_alpha()`
- **render_draw_font**: выделен статический хелпер `render_draw_font_glyph()` — отрисовка одного символа вынесена в отдельную функцию, `render_draw_font()` теперь занимается только парсингом строки и позиционированием; альфа-проверка через `render_has_alpha()`

#### `src/math/gmath.h`
- Добавлена inline-функция `gmath_normalize_angle(float angle)` — нормализует угол в диапазон [0, 2π). Устраняет идентичный двустрочный паттерн из `player.c` и `entity.c`

#### `src/core/player.c`
- Нормализация угла игрока: два условных оператора заменены вызовом `gmath_normalize_angle()`

#### `src/core/entity.c`
- Нормализация угла сущности: два условных оператора заменены вызовом `gmath_normalize_angle()`

#### `src/assets/assets.c`
- Убраны функции `assets_texture_w()` и `assets_texture_h()` (22 строки, два параллельных switch)
- Вместо них введена статическая таблица `s_tex_meta[ASSETS_TEXTUREMAPS_MAX]` типа `TextureMapMeta { u32 w, h }`
- **Расширяемость**: добавление нового TextureMap теперь требует одной строки в таблице вместо двух правок в двух switch-ах

#### `src/core/world_palette.c`
- Убраны три параллельных switch-оператора в `world_palette_coltotype()`, `world_palette_coltotex()`, `world_palette_textocol()`
- Введена единая статическая таблица `s_palette[]` типа `WorldPaletteEntry { color, tile_type, texture_id }`
- Все три функции реализованы линейным поиском по таблице
- **Расширяемость**: добавление нового типа тайла теперь требует одной строки в таблице вместо трёх правок в трёх switch-ах

#### `src/game/game.c`
- `render_update(&game_state, 65)` → `render_update(&game_state, RENDER_FOV_DEFAULT)`

#### `src/game/game_input.c`
- Добавлена константа `GAME_MOUSE_SENSITIVITY` (0.0015f) — заменяет магическое число в `game_handle_mouse_motion()`

#### `src/game/game_bootstrap.c`
- Добавлена константа `GAME_UI_HEIGHT_RATIO` (0.15f) — заменяет магическое число при расчёте высоты UI-полосы

---

### v0.6 и ранее

История изменений не велась.
