#ifndef PLAYER_CONTROLS_H
#define PLAYER_CONTROLS_H

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRACKS 256

extern FILE *mpg_in;
extern FILE *mpg_out;

extern char *playlist[MAX_TRACKS];
extern int playlist_count;
extern int current_index;

extern int pause_flag;

void start_player();
void load_playlist();
void play_current_track();
void pause_player();
void next_track();

#endif
