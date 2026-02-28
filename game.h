#ifndef GAME_H
#define GAME_H

// This header declares functions to control the game play.
// It also has functions to save and load the game state.

#include "board.h"
#include "players.h"

// This is the filename used to store the game save.
#define SAVE_FILE "savegame.txt"

// Start a new game, including placement and movement phases.
void play_game(Tile **board, int rows, int cols,
               Player *players, int num_players);

// Resume a saved game directly from the movement phase.
void continue_game(Tile **board, int rows, int cols,
                   Player *players, int num_players,
                   int mode, int turn_index, int *active_flags);

// Save the current game state to a file.
int save_game(const char *filename,
              Tile **board, int rows, int cols,
              Player *players, int num_players,
              int mode, int turn_index, int *active_flags);

// Load a saved game and create memory for board and players.
int load_game(const char *filename,
              Tile ***out_board, int *out_rows, int *out_cols,
              Player **out_players, int *out_num_players,
              int *out_mode, int *out_turn_index, int **out_active_flags);

#endif