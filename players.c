// This file handles player-related data and helper functions.

#include <stdio.h>
#include <stdlib.h>
#include "players.h"

// These are ANSI color codes used to print colored output in the terminal.
#define CLR_RESET  "\x1b[0m"
#define CLR_BLUE   "\x1b[34m"
#define CLR_RED    "\x1b[31m"
#define CLR_GREEN  "\x1b[32m"
#define CLR_YELLOW "\x1b[33m"

// This function returns the correct color string based on player id.
static const char *player_color(int id)
{
    switch (id)
    {
        case 1: return CLR_BLUE;
        case 2: return CLR_RED;
        case 3: return CLR_GREEN;
        case 4: return CLR_YELLOW;
        default: return CLR_RESET;
    }
}

// This function dynamically allocates memory for all players.
Player *create_players(int num_players)
{
    return malloc(num_players * sizeof(Player));
}

// This function initializes player data (id, score, penguins) and handles human vs AI name setup.
void init_players(Player *players, int num_players)
{
    // Initialize each player one by one.
    int i;

    for (i = 0; i < num_players; i++)
    {
        players[i].id = i + 1;
        players[i].left = 1;
        players[i].score = 0;

        if (players[i].is_ai)
        {
            snprintf(players[i].name, sizeof(players[i].name), "AI");
            printf("Player %d is an AI player.\n", players[i].id);
        }
        else
        {
            printf("Enter name for Player %d: ", players[i].id);
            scanf("%31s", players[i].name);
        }
    }
}

// This function prints the final scoreboard with player colors.
void print_scores(Player *players, int num_players)
{
    // Print all players' scores.
    int i;
    for (i = 0; i < num_players; i++)
    {
        const char *clr = player_color(players[i].id);
        printf("%sPlayer %d (%s)%s: %d fish\n",
               clr,
               players[i].id,
               players[i].name,
               CLR_RESET,
               players[i].score);
    }
}