#include "playerControls.h"

FILE *mpg_in = NULL;
FILE *mpg_out = NULL;


char *playlist[MAX_TRACKS];
int playlist_count = 0;
int current_index = 0;

int pause_flag = 0;

void start_player() {
	// Start mpg123 in remote control mode
	mpg_in = popen("mpg123 -R", "w");
	if (!mpg_in) {
		perror("Failed to start mpg123");
		exit(1);
	}
}

void load_playlist() {
	DIR *dir;
	struct dirent *entry;

	dir = opendir("Playlist");
	if (!dir) {
		perror("Cannot open Playlist directory");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strstr(entry->d_name, ".mp3")) {
			char *path = malloc(512);
			snprintf(path, 512, "Playlist/%s", entry->d_name);
			playlist[playlist_count++] = path;
		}
	}

	closedir(dir);


}

void play_current_track() {
	fprintf(mpg_in, "LOAD %s\n", playlist[current_index]);
	fflush(mpg_in);
}

void stop_player(){
	pause_flag = pause_flag ? 0 : 1;// Toggle pause flag
	fprintf(mpg_in, "s\n");
	fflush(mpg_in);

}

// Skip to next song
void next_track() {
	current_index = (current_index + 1) % playlist_count;
	play_current_track();
}