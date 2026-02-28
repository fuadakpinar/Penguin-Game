/* This file contains functions to create, initialize, print, and manage the game board.
   It also includes functions to check valid moves and find penguins on the board. */

#include <stdio.h>
#include <stdlib.h>
#include "board.h"

// ANSI color codes for terminal output
#define CLR_RESET  "\x1b[0m"
#define CLR_BLUE   "\x1b[34m"
#define CLR_RED    "\x1b[31m"
#define CLR_GREEN  "\x1b[32m"
#define CLR_YELLOW "\x1b[33m"

// Return a color string based on player id
static const char *player_color(int owner)
{
    switch (owner)
    {
        case 1: return CLR_BLUE;
        case 2: return CLR_RED;
        case 3: return CLR_GREEN;
        case 4: return CLR_YELLOW;
        default: return CLR_RESET;
    }
}

// Create a dynamic 2D board with given rows and columns
Tile **create_board(int rows, int cols)
{
    int i;
    // Allocate memory for row pointers
    Tile **board = malloc(rows * sizeof(Tile *));
    if (!board) return NULL;

    // Allocate memory for each row's columns
    for (i = 0; i < rows; i++)
    {
        board[i] = malloc(cols * sizeof(Tile));
        if (!board[i]) return NULL;
    }
    return board;
}

// Initialize the board with random fish numbers and no owners
void init_board(Tile **board, int rows, int cols)
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            int r = rand() % 100;
            if (r < 15)
                board[i][j].fish = 0;
            else
                board[i][j].fish = (rand() % 3) + 1;

            board[i][j].owner = 0;
        }
    }
}

// Print the board with colors for penguins
void print_board(Tile **board, int rows, int cols)
{
    int i, j;

    printf("\n    ");
    for (j = 0; j < cols; j++)
        printf("%3d  ", j + 1);
    printf("\n");

    for (i = 0; i < rows; i++)
    {
        printf("%2d |", i + 1);
        for (j = 0; j < cols; j++)
        {
            int fish = board[i][j].fish;
            int owner = board[i][j].owner;

            // Print empty water tile
            if (fish == 0 && owner == 0)
            {
                printf("  -- ");
            }
            // Print tile with fish but no owner
            else if (owner == 0)
            {
                printf(" %d(%d)", fish, owner);
            }
            // Print tile with fish and a penguin owner in color
            else
            {
                const char *clr = player_color(owner);
                printf(" %d(%s%d%s)", fish, clr, owner, CLR_RESET);
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Check if the placement phase can continue by looking for tiles with 1 fish and no owner
int can_place(Tile **board, int rows, int cols)
{
    int i, j;
    for (i = 0; i < rows; i++)
        for (j = 0; j < cols; j++)
            if (board[i][j].fish == 1 && board[i][j].owner == 0)
                return 1;
    return 0;
}

// Free all dynamically allocated memory of the board properly
void free_board(Tile **board, int rows)
{
    int i;
    if (!board) return;
    for (i = 0; i < rows; i++)
        free(board[i]);
    free(board);
}

// Search the board for a penguin belonging to a player
int find_penguin(Tile **board, int rows, int cols, int player_id, int *out_r, int *out_c)
{
    int r, c;
    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            if (board[r][c].owner == player_id)
            {
                if (out_r) *out_r = r;
                if (out_c) *out_c = c;
                return 1;
            }
        }
    }
    return 0;
}

// Check if a tile is empty water (no fish and no owner)
static int tile_is_empty_water(Tile **board, int r, int c)
{
    return (board[r][c].fish == 0 && board[r][c].owner == 0);
}

// Check if a move is valid: inside bounds, adjacent, not empty water, and unoccupied
int is_valid_move(Tile **board, int rows, int cols, int player_id, int from_r, int from_c, int to_r, int to_c)
{
    int dr = to_r - from_r;
    int dc = to_c - from_c;

    if (to_r < 0 || to_r >= rows || to_c < 0 || to_c >= cols)
        return 0;

    
    if (!((dr == 1 && dc == 0) || (dr == -1 && dc == 0) || (dr == 0 && dc == 1) || (dr == 0 && dc == -1)))
        return 0;

    
    if (tile_is_empty_water(board, to_r, to_c))
        return 0;

    
    if (board[to_r][to_c].owner != 0)
        return 0;

    (void)player_id; 
    return 1;
}

// Check if the player has at least one valid move available
int player_can_move(Tile **board, int rows, int cols, int player_id)
{
    int r, c;
    if (!find_penguin(board, rows, cols, player_id, &r, &c))
        return 0;

    if (is_valid_move(board, rows, cols, player_id, r, c, r - 1, c)) return 1;
    if (is_valid_move(board, rows, cols, player_id, r, c, r + 1, c)) return 1;
    if (is_valid_move(board, rows, cols, player_id, r, c, r, c - 1)) return 1;
    if (is_valid_move(board, rows, cols, player_id, r, c, r, c + 1)) return 1;

    return 0;
}

// Find the first tile suitable for penguin placement (used by AI)
int find_first_placement(Tile **board, int rows, int cols, int *out_r, int *out_c)
{
    int r, c;
    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            if (board[r][c].fish == 1 && board[r][c].owner == 0)
            {
                if (out_r) *out_r = r;
                if (out_c) *out_c = c;
                return 1;
            }
        }
    }
    return 0;
}

// Simple AI heuristic to find the best adjacent move with the most fish
int find_best_adjacent_move(Tile **board, int rows, int cols, int player_id, int *to_r, int *to_c)
{
    int pr, pc;
    int best_r = -1, best_c = -1;
    int best_fish = -1;

    if (!find_penguin(board, rows, cols, player_id, &pr, &pc))
        return 0;

    if (is_valid_move(board, rows, cols, player_id, pr, pc, pr - 1, pc))
    {
        best_r = pr - 1;
        best_c = pc;
        best_fish = board[best_r][best_c].fish;
    }
    if (is_valid_move(board, rows, cols, player_id, pr, pc, pr + 1, pc))
    {
        int f = board[pr + 1][pc].fish;
        if (f > best_fish) { best_r = pr + 1; best_c = pc; best_fish = f; }
    }
    if (is_valid_move(board, rows, cols, player_id, pr, pc, pr, pc - 1))
    {
        int f = board[pr][pc - 1].fish;
        if (f > best_fish) { best_r = pr; best_c = pc - 1; best_fish = f; }
    }
    if (is_valid_move(board, rows, cols, player_id, pr, pc, pr, pc + 1))
    {
        int f = board[pr][pc + 1].fish;
        if (f > best_fish) { best_r = pr; best_c = pc + 1; best_fish = f; }
    }

    if (best_r == -1)
        return 0;

    if (to_r) *to_r = best_r;
    if (to_c) *to_c = best_c;
    return 1;
}