/* This file controls the game flow including placement and movement phases.  
   It also supports saving and loading the game state from a file. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

void clear_screen(void)
{
    printf("\033[2J");   // Clear entire screen
    printf("\033[H");    // Move cursor to top-left
}

// Check if all penguins are placed (left == 0 for all players)
static int all_penguins_placed(Player *players, int num_players)
{
    int i;
    for (i = 0; i < num_players; i++)
        if (players[i].left > 0) return 0;
    return 1;
}

// Save the current game state to a file
int save_game(const char *filename, Tile **board, int rows, int cols, Player *players, int num_players, int mode, int turn_index, int *active_flags)
{
    FILE *fp;
    int i, r, c;

    // open file for writing
    fp = fopen(filename, "w");
    if (!fp) return 0;

    // write header and version
    fprintf(fp, "PENGUINS_SAVE_V1\n");

    // write board size and number of players, mode, turn index
    fprintf(fp, "%d %d\n", rows, cols);
    fprintf(fp, "%d %d %d\n", num_players, mode, turn_index);

    // write active flags for players
    for (i = 0; i < num_players; i++)
    {
        fprintf(fp, "%d", active_flags ? active_flags[i] : 1);
        if (i + 1 < num_players) fprintf(fp, " ");
    }
    fprintf(fp, "\n");

    // write each player's data
    for (i = 0; i < num_players; i++)
    {
        fprintf(fp, "%d %d %d %d %s\n",
                players[i].id,
                players[i].is_ai,
                players[i].left,
                players[i].score,
                players[i].name);
    }

    // write all board tiles with fish and owner info
    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            fprintf(fp, "%d %d\n", board[r][c].fish, board[r][c].owner);
        }
    }

    // close file
    fclose(fp);
    return 1;
}

// Load game state from a file and allocate memory for board, players, active flags
int load_game(const char *filename, Tile ***out_board, int *out_rows,
                int *out_cols, Player **out_players, int *out_num_players,
                int *out_mode, int *out_turn_index, int **out_active_flags)
{
    FILE *fp;
    char header[64];
    int rows, cols, num_players, mode, turn_index;
    int *active;
    Player *players;
    Tile **board;
    int i, r, c;

    // open file for reading
    fp = fopen(filename, "r");
    if (!fp) return 0;

    // read and check header string
    if (fscanf(fp, "%63s", header) != 1)
    {
        fclose(fp);
        return 0;
    }

    if (strcmp(header, "PENGUINS_SAVE_V1") != 0)
    {
        fclose(fp);
        return 0;
    }

    // read board dimensions
    if (fscanf(fp, "%d %d", &rows, &cols) != 2)
    {
        fclose(fp);
        return 0;
    }

    // read number of players, mode, and turn index
    if (fscanf(fp, "%d %d %d", &num_players, &mode, &turn_index) != 3)
    {
        fclose(fp);
        return 0;
    }

    // allocate memory for active flags, players, and board
    active = (int *)malloc(num_players * sizeof(int));
    players = (Player *)malloc(num_players * sizeof(Player));
    board = create_board(rows, cols);

    if (!active || !players || !board)
    {
        if (active) free(active);
        if (players) free(players);
        if (board) free_board(board, rows);
        fclose(fp);
        return 0;
    }

    // read active flags for each player
    for (i = 0; i < num_players; i++)
    {
        if (fscanf(fp, "%d", &active[i]) != 1)
        {
            free(active);
            free(players);
            free_board(board, rows);
            fclose(fp);
            return 0;
        }
    }

    // read player data (id, is_ai, left, score, name)
    for (i = 0; i < num_players; i++)
    {
        int id, is_ai, left, score;
        char name[32];

        if (fscanf(fp, "%d %d %d %d %31s", &id, &is_ai, &left, &score, name) != 5)
        {
            free(active);
            free(players);
            free_board(board, rows);
            fclose(fp);
            return 0;
        }

        players[i].id = id;
        players[i].is_ai = is_ai;
        players[i].left = left;
        players[i].score = score;
        strncpy(players[i].name, name, sizeof(players[i].name));
        players[i].name[sizeof(players[i].name) - 1] = '\0';
    }

    // read board tiles (fish and owner)
    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            int fish, owner;
            if (fscanf(fp, "%d %d", &fish, &owner) != 2)
            {
                free(active);
                free(players);
                free_board(board, rows);
                fclose(fp);
                return 0;
            }
            board[r][c].fish = fish;
            board[r][c].owner = owner;
        }
    }

    // close file after reading all data
    fclose(fp);

    // return all loaded data through output pointers
    *out_rows = rows;
    *out_cols = cols;
    *out_num_players = num_players;
    *out_mode = mode;
    *out_turn_index = turn_index;
    *out_active_flags = active;
    *out_players = players;
    *out_board = board;

    return 1;
}

// Players place exactly one penguin on a tile with 1 fish, AI auto places
static void placement_phase(Tile **board, int rows, int cols, Player *players, int num_players)
{
    int p, r, c, ok;

    printf("\n=== Placement Phase ===\n");
    printf("Place your penguin ONLY on an empty tile with exactly 1 fish.\n");

    // placement continues until all penguins placed or no valid tiles left
    while (1)
    {
        int done = 1;
        for (p = 0; p < num_players; p++)
            if (players[p].left > 0)
                done = 0;

        if (done || !can_place(board, rows, cols))
            break;

        for (p = 0; p < num_players; p++)
        {
            if (players[p].left == 0)
                continue;

            if (!can_place(board, rows, cols))
                break;

            if (players[p].is_ai)
            {
                // AI auto placement: find first valid tile
                if (!find_first_placement(board, rows, cols, &r, &c))
                {
                    printf("AI could not find a valid placement.\n");
                    continue;
                }
                printf("AI places penguin at row %d col %d.\n", r + 1, c + 1);
            }
            else
            {
                // human player input loop for placement
                ok = 0;
                while (!ok)
                {                  
                    print_board(board, rows, cols);
                    printf("Player %d (%s): score=%d\n",
                           players[p].id, players[p].name, players[p].score);
                    printf("Enter row and column to place penguin (1-%d 1-%d): ", rows, cols);

                    if (scanf("%d %d", &r, &c) != 2)
                    {
                        printf("Invalid input. Exiting.\n");
                        exit(1);
                    }

                    r--; c--;

                    if (r < 0 || r >= rows || c < 0 || c >= cols)
                    {
                        printf("Out of bounds.\n");
                        continue;
                    }
                    if (board[r][c].fish != 1)
                    {
                        printf("Invalid: tile must have exactly 1 fish.\n");
                        continue;
                    }
                    if (board[r][c].owner != 0)
                    {
                        printf("Invalid: tile is occupied.\n");
                        continue;
                    }

                    ok = 1;
                }
            }

            // update score, board state, and penguins left to place
            players[p].score += board[r][c].fish;
            board[r][c].fish = 0;
            board[r][c].owner = players[p].id;
            players[p].left--;
        }
    }
}

// Players move 1 step with rules, AI chooses simple move, humans can save & quit
static void movement_phase(Tile **board, int rows, int cols, Player *players, int num_players, int mode, int *turn_index_io, int *active_flags)
{
    int active_count = 0;
    int p;

    printf("\n=== Movement Phase ===\n");
    printf("Move your penguin 1 step: up/down/left/right.\n");
    printf("You cannot move onto empty water (--) or onto occupied tiles.\n");
    printf("If a player cannot move, they will be skipped for the rest of the game.\n");
    printf("Human can enter Q to SAVE and QUIT during movement.\n");

    // count how many players are active (can still move)
    for (p = 0; p < num_players; p++)
        if (active_flags[p]) active_count++;

    // game continues while there is at least one active player
    while (active_count > 0)
    {
        int any_move = 0;
        int start = *turn_index_io;
        int loops = 0;

        // loop through players in turn order
        while (loops < num_players)
        {
            int idx = (*turn_index_io) % num_players;
            int pr, pc;
            int tr, tc;

            // update turn index to next player
            (*turn_index_io) = (idx + 1) % num_players;
            loops++;

            if (!active_flags[idx])
                continue;

            // check if player can move any penguin
            if (!player_can_move(board, rows, cols, players[idx].id))
            {
                printf("\nPlayer %d (%s) cannot move anymore and is now inactive.\n",
                       players[idx].id, players[idx].name);
                active_flags[idx] = 0;
                active_count--;
                if (active_count == 0) break;
                continue;
            }

            // find current penguin position for player
            if (!find_penguin(board, rows, cols, players[idx].id, &pr, &pc))
            {
                printf("\nError: penguin for Player %d not found.\n", players[idx].id);
                active_flags[idx] = 0;
                active_count--;
                if (active_count == 0) break;
                continue;
            }

            if (players[idx].is_ai)
            {
                // AI chooses best adjacent move
                if (!find_best_adjacent_move(board, rows, cols, players[idx].id, &tr, &tc))
                {
                    printf("AI has no valid moves.\n");
                    active_flags[idx] = 0;
                    active_count--;
                    if (active_count == 0) break;
                    continue;
                }
                printf("AI moves to row %d col %d.\n", tr + 1, tc + 1);
            }

            else
            {
                // human player input loop for WASD movement
                while (1)
                {
                    char cmd;
                    int nr = pr, nc = pc;
                    
                    print_board(board, rows, cols);
                    printf("Player %d (%s): score=%d\n", players[idx].id, players[idx].name, players[idx].score);
                    printf("Your penguin is at: row %d col %d\n", pr + 1, pc + 1);

                    printf("Move with W/A/S/D (1 step). (Q = save & quit)\n");
                    printf("Enter command: ");

                    {
                        char line[64];
                        int k;

                        if (!fgets(line, sizeof(line), stdin))
                        {
                            printf("Invalid input. Exiting.\n");
                            exit(1);
                        }

                        /* take first non-space character from the line */
                        cmd = '\0';
                        for (k = 0; line[k] != '\0'; k++)
                        {
                            if (line[k] != ' ' && line[k] != '\t' && line[k] != '\n' && line[k] != '\r')
                            {
                                cmd = line[k];
                                break;
                            }
                        }

                        if (cmd == '\0')
                        {
                            printf("Invalid command. Use W/A/S/D.\n");
                            continue;
                        }

                        if (cmd >= 'A' && cmd <= 'Z')
                            cmd = (char)(cmd - 'A' + 'a');
                    }

                    if (cmd == 'w') nr = pr - 1;
                    else if (cmd == 's') nr = pr + 1;
                    else if (cmd == 'a') nc = pc - 1;
                    else if (cmd == 'd') nc = pc + 1;
                    else if (cmd == 'q')
                    {
                        if (save_game(SAVE_FILE, board, rows, cols, players, num_players, mode, *turn_index_io, active_flags))
                        {
                            printf("Game saved to %s. Exiting now.\n", SAVE_FILE);
                        }
                        else
                        {
                            printf("Failed to save game.\n");
                        }
                        exit(0);
                    }
                    
                    else
                    {
                        printf("Invalid command. Use W/A/S/D.\n");
                        continue;
                    }

                    tr = nr;
                    tc = nc;

                    if (!is_valid_move(board, rows, cols, players[idx].id, pr, pc, tr, tc))
                    {
                        printf("Invalid move.\n");
                        continue;
                    }
                    break;
                }
            }

            // apply move: update score, board tiles (new and old)
            players[idx].score += board[tr][tc].fish;
            board[tr][tc].fish = 0;
            board[tr][tc].owner = players[idx].id;

            board[pr][pc].owner = 0;
            board[pr][pc].fish = 0;

            any_move = 1;
        }

        // if no player moved in this round, end the game
        if (!any_move)
            break;
    }

    printf("\nNo players can move. Game over.\n");
}

// Start a new game: placement phase first, then movement with fresh state
void play_game(Tile **board, int rows, int cols, Player *players, int num_players)
{
    placement_phase(board, rows, cols, players, num_players);
    {
        // allocate memory for active player flags
        int *active_flags = (int *)malloc(num_players * sizeof(int));
        int turn_index = 0;
        int i;

        if (!active_flags)
        {
            printf("Memory allocation failed.\n");
            return;
        }

        // set all players as active at start
        for (i = 0; i < num_players; i++)
            active_flags[i] = 1;

        // call movement phase to play the game
        movement_phase(board, rows, cols, players, num_players, 1, &turn_index, active_flags);

        // free active flags memory after game ends
        free(active_flags);
    }
}

// Continue game from loaded state: skip placement, continue movement phase
void continue_game(Tile **board, int rows, int cols, Player *players, int num_players, int mode, int turn_index, int *active_flags)
{
    movement_phase(board, rows, cols, players, num_players, mode, &turn_index, active_flags);
}