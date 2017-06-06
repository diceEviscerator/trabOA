#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct block {
	char bytes_s[512];
} block;

typedef struct  sectror_array {
	block sector[60];
} sectror_array;

typedef struct track_array {
	sectror_array track[5];
} track_array;

typedef struct  fatlist_s {
	unsigned int used;
	unsigned int eof;
	unsigned int next;
} fatlist;

typedef struct fatent_s {
	char file_name[100];
	unsigned int first_sector;
	struct fatent_s *next_file;
} fatent;

void initialize_disk();

void show_menu ();

void menu_selection (int menu_option);

void write_file();

void read_file ();

void find_drive_sector(int fat_number, int j, int t, int s);

int find_fat_sector(int j, int t, int s);