#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define KEY_QUIT    0
#define KEY_LEFT    1
#define KEY_RIGHT   2
#define KEY_UP      3
#define KEY_DOWN    4
#define KEY_JUMP    5
#define KEY_FIRE    6
#define KEY_SPECIAL 7
#define KEY_USE     8
#define KEY_GRAB    9
#define KEY_SELECT  10
#define KEY_RESET   11
#define NUM_KEYS    13

#if defined __cplusplus
extern "C" {
#endif

extern SDL_Window *main_window;
extern SDL_GLContext main_context;
extern SDL_GameController *controller;

typedef struct {
	int x;
	int y;
	bool ldown;
	bool rdown;
	bool odown;
} mouse_input;

bool init_window(const char *program_name, int w, int h);
void cleanup_window();
void swap_window();
void get_input(bool *downs, bool *presses, const int *key_map, mouse_input *mouse);
void set_default_key_map(int *key_map);
void set_wasd_key_map(int *key_map);
void print_sdl_gl_attributes();
void check_sdl_error(int line);

#ifdef __cplusplus
}
#endif

#endif //WINDOW_H
