/* 
 * Program entry point for Penguins Game.
 * Handles menu display, save/load functionality,
 * and starting or continuing the game.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "players.h"
#include "game.h"

// Function to check if a save file exists
static int save_exists(const char *filename)
{
    // Try to open the file to check if it exists
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

int main(void)
{
    // Board size fixed to 10x10, num_players will be chosen by user
    int rows = 10, cols = 10, num_players;
    int mode;

    // Seed random number generator with current time
    srand((unsigned int)time(NULL));

    printf("=== Penguins Game ===\n");

    // If save file exists, ask user if they want to continue or start new game
    if (save_exists(SAVE_FILE))
    {
        int choice;
        printf("A saved game was found.\n");
        printf("1) Continue saved game\n");
        printf("2) Start new game\n");
        printf("Enter choice (1-2): ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");
            return 1;
        }

        if (choice == 1)
        {
            Tile **loaded_board = NULL;
            Player *loaded_players = NULL;
            int *active_flags = NULL;
            int loaded_rows = 0, loaded_cols = 0;
            int loaded_num_players = 0;
            int loaded_mode = 0;
            int loaded_turn_index = 0;

            // Load board, players, and movement state from saved file
            if (!load_game(SAVE_FILE,
                           &loaded_board, &loaded_rows, &loaded_cols,
                           &loaded_players, &loaded_num_players,
                           &loaded_mode, &loaded_turn_index, &active_flags))
            {
                printf("Failed to load saved game.\n");
                return 1;
            }

            // Resume game from movement phase with saved turn and active player state
            continue_game(loaded_board, loaded_rows, loaded_cols,
                          loaded_players, loaded_num_players,
                          loaded_mode, loaded_turn_index, active_flags);

            // Show final scores after game ends
            printf("\n=== Final Scores ===\n");
            print_scores(loaded_players, loaded_num_players);

            // Free all allocated memory for loaded game
            free_board(loaded_board, loaded_rows);
            free(loaded_players);
            free(active_flags);

            return 0;
        }
    }

    // Ask user to select game mode: Player vs Player or Player vs AI
    printf("Select mode:\n");
    printf("1) Player vs Player\n");
    printf("2) Player vs AI\n");
    printf("Enter choice (1-2): ");

    if (scanf("%d", &mode) != 1)
    {
        printf("Invalid input.\n");
        return 1;
    }

    if (mode == 2)
    {
        num_players = 2;
        printf("Player vs AI selected. Number of players set to 2.\n");
    }
    else
    {
        printf("Enter number of players (2-4): ");
        if (scanf("%d", &num_players) != 1)
        {
            printf("Invalid input.\n");
            return 1;
        }
    }

    // Create the game board and players dynamically
    Tile **board = create_board(rows, cols);
    Player *players = create_players(num_players);

    if (!board || !players)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Mark AI-controlled players before initializing player names
    if (mode == 2)
    {
        players[0].is_ai = 0;
        players[1].is_ai = 1;
    }
    else
    {
        int i;
        for (i = 0; i < num_players; i++)
            players[i].is_ai = 0;
    }

    // Initialize players and board for new game
    init_players(players, num_players);
    init_board(board, rows, cols);

    // Start the new game play loop
    play_game(board, rows, cols, players, num_players);

    // Print final scores after game ends
    printf("\n=== Final Scores ===\n");
    print_scores(players, num_players);

    // Clean up all allocated memory before exit
    free_board(board, rows);
    free(players);

    return 0;
}