#ifndef PLAYERS_H
#define PLAYERS_H

// This header defines the Player structure and player-related function declarations.

// This struct represents a player in the game.
typedef struct {
    char name[32];   // player name
    int id;          // unique player identifier
    int is_ai;       // whether the player is controlled by AI
    int left;        // number of penguins left to place
    int score;       // total collected fish
} Player;

// This function allocates memory for all players.
Player *create_players(int num_players);

// This function initializes player data and names.
void init_players(Player *players, int num_players);

// This function prints the scoreboard.
void print_scores(Player *players, int num_players);

#endif