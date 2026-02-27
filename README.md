# theDODIK3D

Wolfenstein-style raycasting 3D движок на C17, SDL3, Windows/MSVC.

---

## v0.2.1 — Фикс сборки: toolset, SDL3.lib, _CRT_SECURE_NO_WARNINGS

### Проблема

Проект не собирался из командной строки по трём причинам.

**1. Несуществующий PlatformToolset `v145`**

В `theDODIK3D.vcxproj` для всех четырёх конфигураций был указан `<PlatformToolset>v145</PlatformToolset>`. Тулсет `v145` не существует ни в одном официальном релизе Visual Studio. На машине установлены `v142` (14.29) и `v143` (14.34). MSBuild падал с `MSB8020: Cannot find Build Tools for v145`.

**Исправление:** заменить `v145` → `v143` во всех четырёх конфигурациях.

---

**2. Отсутствие `lib/x64/SDL3.lib`**

Директория `lib/` не была зафиксирована в репозитории. Линковщик не мог найти `SDL3.lib` (`LNK1104`). SDL3 headers присутствовали (`include/SDL3/`), DLL присутствовала в `PNGConverter/lib/`, но import library отсутствовала.

**Исправление:** import library сгенерирована из `SDL3.dll` через `dumpbin.exe` + `lib.exe` (MSVC toolchain):
```
dumpbin /EXPORTS SDL3.dll  →  SDL3.def  →  lib /MACHINE:X64 /DEF:SDL3.def  →  SDL3.lib
```
SDL3.dll и сгенерированный SDL3.lib добавлены в `lib/x64/`. SDL версия: **3.4.0** (1271 экспортируемый символ).

---

**3. Ошибка C4996 (`fopen` treated as error)**

В `SDLCheck=true` режиме MSVC превращает предупреждение C4996 о `fopen` в ошибку компилятора. Это затрагивало `assets.c` и `world.c`, где используется стандартный `fopen`. `fopen_s` — MSVC-специфичная функция (Annex K), добавлять её нецелесообразно.

**Исправление:** добавить `_CRT_SECURE_NO_WARNINGS` в `PreprocessorDefinitions` всех четырёх конфигураций в `vcxproj`. Это стандартная практика для C-проектов под MSVC.

---

### Затронутые файлы

| Файл | Изменение |
|------|-----------|
| `theDODIK3D.vcxproj` | `v145` → `v143`; `_CRT_SECURE_NO_WARNINGS` во все конфигурации |
| `lib/x64/SDL3.lib` | добавлен (import library, сгенерирован из SDL3.dll 3.4.0) |
| `lib/x64/SDL3.dll` | добавлен (скопирован из PNGConverter/lib/) |

---

## v0.2 — Система анимированных текстур, рефакторинг, исправление UB

### Новый функционал: анимированные текстуры

#### Зачем

Ранее в `assets.h` существовала заготовка `TextureAnim` с полями `w, h, index, frames, pixels[]`, но ни функции загрузки, ни рендера не было. Формат `.DIHA` был объявлен как `ASSETS_EXT_ANIM`, но нигде не использовался. Задача — довести систему до рабочего состояния по аналогии с `Texture` / `assets_load_texture` / `render_draw_texture`.

---

#### `src/assets/assets.h`

**`AnimatedTexture`** (заменяет нерабочий `TextureAnim`):

```c
typedef struct AnimatedTexture {
    u32   w, h;
    u32   frame_count;
    u32   current_frame;
    float frame_duration; // секунд на кадр
    float frame_timer;    // накопленное время
    u32   pixels[];       // frame_count * w * h пикселей подряд
} AnimatedTexture;
```

`pixels[]` — гибкий массив в конце структуры (так же, как в `Texture`). Кадры хранятся последовательно: кадр N начинается с `pixels[N * w * h]`. Это позволяет выделить всё одним `malloc` и переключать кадры смещением указателя без дополнительных аллокаций.

`frame_timer` накапливает `dt` каждый кадр; когда он достигает `frame_duration` — `current_frame` переключается, таймер сбрасывается вычитанием (не обнулением), чтобы не терять накопленный остаток при неравномерном `dt`.

**`AnimId`** enum — аналог `TextureId`, индексирует массив анимаций:

```c
typedef enum AnimId {
    // добавлять новые ID сюда по мере создания .DIHA ассетов
    ANIM_COUNT
} AnimId;
```

**`ASSETS_ANIMS_COUNT_MAX 64`** — лимит слотов для анимаций (отдельно от `ASSETS_TEXTURES_COUNT_MAX 1024`).

**Новые объявления:**

```c
bool             assets_load_anim(const char* path, AnimId id, float frame_duration);
AnimatedTexture* assets_anim_get(AnimId id);
void             assets_update(float dt);
```

**`assets_font_get`** перенесён из `.c` в `inline` в заголовок — это был тривиальный враппер вокруг `assets_get` с лишним тернарным оператором `return font ? font : NULL`, который ничего не делал. Теперь:

```c
static inline Texture* assets_font_get(TextureId id)
{
    return assets_get(id);
}
```

---

#### `src/assets/assets.c`

**`assets_load_anim`** — загрузка `.DIHA` файла:

Формат файла: `u32 w`, `u32 h`, `u32 frame_count`, затем `frame_count * w * h * sizeof(u32)` байт пиксельных данных в формате `ARGB8888`. Структурно идентичен `.DIHT`, но с дополнительным полем `frame_count` в заголовке.

Выделяется один блок памяти `sizeof(AnimatedTexture) + frame_count * w * h * sizeof(u32)`. Вся читается за один `fread`. При любой ошибке файл закрывается, память освобождается.

**`assets_anim_get`** — возвращает `AnimatedTexture*` по `AnimId` с проверкой диапазона.

**`assets_update(float dt)`** — вызывается раз в кадр, обходит весь массив `anims[]` и продвигает таймер каждой загруженной анимации:

```c
anim->frame_timer += dt;
if (anim->frame_timer >= anim->frame_duration) {
    anim->frame_timer  -= anim->frame_duration;
    anim->current_frame = (anim->current_frame + 1) % anim->frame_count;
}
```

---

#### `src/render/render.h` / `render.c`

**`render_draw_anim`** — рендерит текущий кадр `AnimatedTexture` в `FrameBuffer`:

```c
void render_draw_anim(int x_start, int y_start, int w, int h, AnimId anim_id, bool mirrored);
```

Логика идентична `render_draw_texture`: билинейное масштабирование через шаг по текстуре, поддержка `mirrored`, пропуск прозрачных пикселей (alpha == 0). Указатель на начало текущего кадра вычисляется один раз до цикла:

```c
const u32* frame_pixels = &anim->pixels[anim->current_frame * anim->w * anim->h];
```

Forward-declaration `typedef enum AnimId AnimId` добавлена в `render.h` — по аналогии с существующим `typedef enum TextureId TextureId`.

---

#### `src/game/game.c`

`assets_update(timer->delta_scaled)` добавлен в `SDL_AppIterate` сразу после `gtimer_update` — до `world_update` и `render_update`, чтобы `current_frame` был уже актуальным к моменту рендера:

```c
gtimer_update(timer);
assets_update(timer->delta_scaled); // <- новый вызов
world_update(current_world, timer->delta_scaled);
...
render_update(&game_state, 65);
```

Используется `delta_scaled` (с учётом `time_scale`), чтобы анимации реагировали на замедление/ускорение времени.

---

#### Как добавить новую анимацию

1. Сконвертировать спрайт-лист через `PNGConverter/PNGConverter.py` в `.DIHA` (формат: `w`, `h`, `frame_count`, все кадры подряд).
2. Добавить ID в `AnimId` в `assets.h`.
3. Загрузить в `game_load_assets()` в `game_bootstrap.c`:
   ```c
   assets_load_anim("assets/textures/tiles/lava"ASSETS_EXT_ANIM, ANIM_TILE_LAVA, 0.1f);
   ```
4. Рендерить через `render_draw_anim(x, y, w, h, ANIM_TILE_LAVA, false)`.

---

### Исправления и рефакторинг

#### `assets.c` — утечка дескриптора в `assets_load_texture`

**Было:** при `w == 0 || h == 0` функция делала `return false` без `fclose(f)`.

**Стало:** `fclose(f)` вызывается до возврата. Это был единственный путь выхода без закрытия файла — все остальные ошибочные пути уже закрывали файл.

---

#### `assets.c` — неверная логика в `assets_free`

**Было:**

```c
static u32 textures_counter = 0;
// ...
for (int i = 0; i < textures_counter; ++i) {
    if (textures[i]) { free(textures[i]); ... }
}
```

`textures_counter` считает количество успешно загруженных текстур, но текстуры хранятся в массиве `textures[]` по индексу `TextureId`, а не по порядку загрузки. Если `TextureId` не образуют непрерывную последовательность `[0, N)` — цикл освобождает не те слоты. Например: загружены `TEXTURE_TILE_DEFAULT (0)` и `TEXTURE_FONT_MINECRAFT (N)`, `textures_counter = 2`, цикл проверяет только `textures[0]` и `textures[1]`, `textures[N]` утекает.

**Стало:** итерация по полному диапазону `[0, ASSETS_TEXTURES_COUNT_MAX)`. Освобождаются ровно те слоты, которые ненулевые:

```c
for (int i = 0; i < ASSETS_TEXTURES_COUNT_MAX; ++i) {
    if (textures[i]) { free(textures[i]); textures[i] = NULL; }
}
```

Аналогичная логика добавлена для `anims[]`.

---

#### `render.h` — UB в `render_color`

**Было:**

```c
static inline u32 render_color(u8 r, u8 g, u8 b, u8 a)
{
    return (a << 24) | (r << 16) | (g << 8) | b;
}
```

В C целочисленное продвижение (integer promotion) преобразует `u8` в `int` (знаковый) перед операцией сдвига. `(u8)0xFF << 24` даёт `(int)0xFF000000 = -16777216` — сдвиг значащего бита в знаковый разряд — **неопределённое поведение** по стандарту C17 (§6.5.7).

**Стало:**

```c
return ((u32)a << 24) | ((u32)r << 16) | ((u32)g << 8) | (u32)b;
```

Явный каст до беззнакового 32-битного типа перед сдвигом — операция строго определена.

---

#### `render.c` — NULL-разыменование в `render_draw_texture`

**Было:** функция обращалась к `texture->w` сразу после `assets_get`, не проверяя возврат.

**Стало:** добавлена ранняя проверка:

```c
Texture* texture = assets_get(tex_id);
if (!texture) return;
```

---

#### `render.c` — выход за границы буфера в `render_draw_texture` (mirrored)

**Проблема:** в mirrored-режиме `tex_step_x` отрицательный, `tex_pos_x` начинается с `texture->w - 1` и убывает. При `w > texture->w` шаг по экрану меньше шага по текстуре — `tex_pos_x` может уйти ниже нуля ещё до конца строки. `int tex_x = (int)tex_pos_x` даёт отрицательный индекс → доступ за начало массива `pixels[]` → **UB**.

Та же проблема существовала и для `tex_y` при `h < texture->h`.

**Стало:** явный clamp для обоих индексов в обеих функциях (`render_draw_texture` и `render_draw_anim`):

```c
if (tex_x < 0) tex_x = 0;
if (tex_x >= (int)texture->w) tex_x = (int)texture->w - 1;
```

---

#### `world.h` — дублирование bounds-проверок

**Было:** функции `world_tile_type`, `world_tile_texture`, `world_tile_color` каждая самостоятельно проверяла `world == NULL`, `tile_x < 0`, `tile_x >= world->w`, `tile_y < 0`, `tile_y >= world->h` — ровно те же условия, которые уже содержала `world_tile()`. Итого каждый вызов делал двойной bounds-чек.

**Стало:** дублирующие проверки удалены; функции делегируют `world_tile()` и проверяют только результат:

```c
static inline u32 world_tile_type(World* world, int tile_x, int tile_y)
{
    WorldTile *tile = world_tile(world, tile_x, tile_y);
    return tile ? tile->type : 0;
}
```

Также исправлен `world_tile`: `return 0` для пути-ошибки заменён на `return NULL` — семантически корректно для указателя (хотя в C `return 0` и `return NULL` эквивалентны, явный `NULL` читается однозначно). Попутно исправлено сравнение `tile_x >= world->w` при смешанных типах `int`/`u32` — добавлен каст `(int)world->w` и `(int)world->h`, чтобы избежать срабатывания signed/unsigned предупреждений компилятора.

---

### Затронутые файлы

| Файл | Тип изменений |
|------|--------------|
| `src/assets/assets.h` | новый `AnimatedTexture`, `AnimId`, объявления; рефакторинг `assets_font_get` |
| `src/assets/assets.c` | `assets_load_anim`, `assets_anim_get`, `assets_update`; фикс `assets_free`; фикс утечки дескриптора |
| `src/render/render.h` | `render_draw_anim`; forward-declaration `AnimId`; фикс UB в `render_color` |
| `src/render/render.c` | `render_draw_anim`; NULL-проверка в `render_draw_texture`; clamp индексов |
| `src/core/world.h` | удалены дублирующие bounds-чеки; `return 0` → `return NULL` |
| `src/game/game.c` | вызов `assets_update(timer->delta_scaled)` в главном цикле |
