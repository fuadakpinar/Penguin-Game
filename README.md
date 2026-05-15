# Penguin Game

A terminal-based, turn-based penguin board game written in C. Each player
places a penguin on a 10×10 ice board, then moves it one step at a time
to collect fish from neighbouring tiles. A tile becomes empty water once
its penguin leaves, so the board slowly collapses around the players
until nobody can move.

Originally written during a university programming course; rewritten as
a clean, multi-file C project with a separate board / players / game
layer, an optional AI opponent, ANSI-coloured output, and a save / load
feature that lets a match be resumed from where it was paused.

---

## Features

- **Turn-based board game** on a fixed 10×10 grid with randomly seeded
  fish counts (0–3 per tile).
- **2–4 players** — choose Player vs Player (up to 4) or Player vs AI
  (fixed at 2).
- **Simple AI opponent** — auto-places its penguin on the first
  available 1-fish tile, and on each turn picks the adjacent move with
  the most fish.
- **WASD movement** — `W` up, `A` left, `S` down, `D` right; 1 step,
  orthogonal only.
- **Save & quit at any time** — a human player can press `Q` during the
  movement phase to write the current game state to `savegame.txt` and
  exit. On the next launch the program offers to resume.
- **ANSI-coloured terminal output** — each player id maps to its own
  colour (blue / red / green / yellow) so the board stays readable with
  multiple penguins on it.
- **Modular layout** — board logic, player logic, game flow, and the
  program entry point each live in their own translation unit with a
  matching header.
- **Dynamic memory** — the board (`Tile **`), the player array, and the
  per-player active flags are all heap-allocated and freed on exit.

## Project structure

```
Penguin-Game/
├── README.md
├── main.c            # entry point: menu, save detection, mode select
├── board.c / board.h # 2D board, fish layout, move validation, AI helpers
├── game.c / game.h   # placement & movement phases, save_game / load_game
├── players.c / players.h # Player struct, init, scoreboard
└── savegame.txt      # written on save & quit (created at runtime)
```

The three modules map cleanly onto the three concerns of the game:

- `board.*` knows about tiles, fish, ownership, and what counts as a
  legal move — but not whose turn it is.
- `players.*` knows about player identity, names, score, and AI flag —
  but not about the board.
- `game.*` is the coordinator: it runs the placement phase, the
  movement phase, and the save / load format.

## Requirements

- A C compiler (GCC or Clang)
- A terminal that supports ANSI escape sequences (any modern macOS /
  Linux terminal; on Windows use Windows Terminal or WSL)

## Build & run

Using GCC:

```bash
gcc main.c board.c game.c players.c -o Penguin-Game
./Penguin-Game
```

Or with Clang:

```bash
clang main.c board.c game.c players.c -o Penguin-Game
./Penguin-Game
```

No external libraries — only the C standard library (`stdio.h`,
`stdlib.h`, `string.h`, `time.h`).

## Game flow

On launch the program does, in order:

1. **Save check** — if `savegame.txt` is present, ask whether to
   continue the saved game or start a new one.
2. **Mode select** (new game only) — Player vs Player or Player vs AI.
3. **Player count** (PvP only) — 2 to 4 players; PvAI is always 2.
4. **Name prompt** — each human player enters a name (max 31 chars).
   AI players are automatically named `AI`.
5. **Placement phase** — each player places **one** penguin on a tile
   that currently has exactly 1 fish. The fish is collected as score
   and the tile becomes owned by that penguin.
6. **Movement phase** — players take turns moving 1 step (W/A/S/D).
   Each move collects the fish on the destination tile; the tile left
   behind turns into empty water (`--`).
7. **End** — a player who cannot move is marked inactive. When nobody
   can move, the final scoreboard is printed.

### Movement rules

- 1 step only, in one of the 4 orthogonal directions.
- The destination must be **inside the board**, **not empty water**,
  and **not occupied by another penguin**.
- A player with no legal moves is skipped permanently for the rest of
  the game.

### Board legend

```
3(0)   tile with 3 fish, no owner
1(2)   tile with 1 fish, owned by player 2 (id printed in player colour)
 --    empty water (no fish, no owner) — penguins cannot enter
```

Row and column numbers are 1-indexed in the UI.

## Save / load

Pressing `Q` during the movement phase writes the full game state to
`savegame.txt` and exits. The file uses a small text format with a
version header so a future save format can reject mismatched files:

```
PENGUINS_SAVE_V1
<rows> <cols>
<num_players> <mode> <turn_index>
<active_flag_player_1> <active_flag_player_2> ...
<id> <is_ai> <left> <score> <name>      # one line per player
...
<fish> <owner>                          # one line per tile, row-major
...
```

Only the movement phase can be saved — placement is short and always
restarted fresh on a new game. On the next launch, choosing **Continue**
re-allocates the board, the player array, and the active-flags array,
and resumes the movement phase from the saved `turn_index`.

If the header doesn't match `PENGUINS_SAVE_V1` the load is rejected
and the program reports a load failure rather than reading garbage.

## AI behaviour

The AI is intentionally simple:

- **Placement**: picks the first tile it finds (row-major scan) that
  has exactly 1 fish and no owner.
- **Movement**: looks at its 4 neighbouring tiles, picks the legal one
  with the most fish; ties are broken by direction order
  (up, down, left, right). If no legal move exists, the AI becomes
  inactive like any human player.

There is no lookahead and no scoring of long-term board control — it
is a baseline opponent for a 2-player game, not a strong solver.

## Learning goals

This project was written to practise:

- Multi-file C project structure (`.c` / `.h` separation, internal
  helpers marked `static`).
- Dynamic memory: 2D arrays via `Tile **`, allocation failure
  handling, paired `free` on every exit path.
- Splitting game logic from I/O — board rules don't call `printf`,
  game flow does.
- A simple text-based save format with a version header.

## Status

This is a coursework / portfolio project. It runs end-to-end (new game,
AI opponent, save & resume) and is the cleaned-up successor to several
earlier iterations kept alongside this folder as backup snapshots
(`Penguin-Game(BackupBefore...)`). Those backups are intentionally
ignored — only the current `Penguin-Game/` directory is maintained.

## Author

**Fuad Akpinar** — Computer Science student.
