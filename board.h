#ifndef BOARD_H
#define BOARD_H

// This header file defines the board structure and functions related to the board.

typedef struct {
    // This struct represents one cell on the board.
    int fish;   // Number of fish on this tile.
    int owner;  // Which player owns this tile, -1 if none.
} Tile;

// Create a new board with given rows and columns, allocating memory dynamically.
Tile **create_board(int rows, int cols);

// Initialize the board by setting fish counts on each tile.
void init_board(Tile **board, int rows, int cols);

// Print the current state of the board to the terminal.
void print_board(Tile **board, int rows, int cols);

// Check if the placement phase can continue (if there are places to put penguins).
int can_place(Tile **board, int rows, int cols);

// Free the memory allocated for the board.
void free_board(Tile **board, int rows);

// Find the position of a player's penguin on the board.
// Returns 1 if found, and sets out_r and out_c to the position.
int find_penguin(Tile **board, int rows, int cols, int player_id, int *out_r, int *out_c);

// Check if the player has any valid moves left.
int player_can_move(Tile **board, int rows, int cols, int player_id);

// Check if a move from (from_r, from_c) to (to_r, to_c) is valid according to game rules.
int is_valid_move(Tile **board, int rows, int cols, int player_id, int from_r, int from_c, int to_r, int to_c);

// Used by AI to find the first place to put a penguin.
int find_first_placement(Tile **board, int rows, int cols, int *out_r, int *out_c);

// Used by AI to find the best adjacent move for a player.
int find_best_adjacent_move(Tile **board, int rows, int cols, int player_id, int *to_r, int *to_c);

#endif