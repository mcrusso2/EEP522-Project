#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

#define MAX_TRACKS 256

FILE *mpg_in = NULL;
FILE *mpg_out = NULL;

char *playlist[MAX_TRACKS];
int playlist_count = 0;
int current_index = 0;

int pause_flag = 0;

void start_player();
void load_playlist();
void play_current_track();
void stop_player();
void next_track();
