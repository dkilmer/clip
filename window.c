#include "window.h"
#include <stdio.h>
#include <glad/glad.h>

SDL_Window *main_window;
SDL_GLContext main_context;
SDL_GameController *controller;

int is_mouse_event(void *userdata, SDL_Event* event)
{
	if (event->type == SDL_MOUSEMOTION)	return 0;
	return 1;
}

bool init_window(const char *program_name, int w, int h)
{
	// Initialize SDL's Video subsystem
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0) {
		printf("SDL_Init failed\n");
		return false;
	}

	// Set our OpenGL version.
	// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run_trifle it
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Create our window centered at 512x512 resolution
	main_window = SDL_CreateWindow(program_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                              w, h, SDL_WINDOW_OPENGL);

	// Check that everything worked out okay
	if (!main_window) {
		printf("Unable to create window\n");
		check_sdl_error(__LINE__);
		return false;
	}
	SDL_SetEventFilter(&is_mouse_event, NULL);

	// Create our opengl context and attach it to our window
	main_context = SDL_GL_CreateContext(main_window);

	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// This makes our buffer swap syncronized with the monitor's vertical refresh
	SDL_GL_SetSwapInterval(1);

	// Init GLAD
	if (!gladLoadGL()) {
		printf("glad failed to load\n");
		return -1;
	}

	// Init a game controller if there is one
	controller = NULL;
	int num_js = SDL_NumJoysticks();
	if (num_js > 0) {
		controller = SDL_GameControllerOpen(0);
		if (controller != NULL) {
			int controller_state = SDL_GameControllerEventState(SDL_ENABLE);
			printf("controller state: %d\n", controller_state);
			if (SDL_GameControllerMapping(controller) == NULL) {
				printf("no mapping for controller\n");
			}
			const char *cn = SDL_GameControllerName(controller);
			printf("controller name: %s\n", cn);
		} else {
			printf("unable to initialize controller\n");
		}
	} else {
		printf("no controllers attached\n");
	}

	return true;
}

void cleanup_window()
{
	SDL_GL_DeleteContext(main_context);
	SDL_DestroyWindow(main_window);
	if (controller != NULL) {
		SDL_GameControllerClose(controller);
	}
	SDL_Quit();
}

void swap_window() {
	SDL_GL_SwapWindow(main_window);
}

int key_map_index_for_key(int keycode, const int *key_map) {
	for (int i=0; i<NUM_KEYS; i++) {
		if (key_map[i] == keycode) {
			return i;
		}
	}
	return -1;
}

void get_input(bool *downs, bool *presses, const int *key_map, mouse_input *mouse) {
	SDL_Event e;
	for (int i=0; i<NUM_KEYS; i++) {
		presses[i] = false;
	}
	while (SDL_PollEvent(&e)) {
		//PrintEvent(&e);
		if (e.type == SDL_QUIT) {
			downs[KEY_QUIT] = true;
			presses[KEY_QUIT] = true;
		}
		if (e.type == SDL_CONTROLLERBUTTONDOWN) {
			switch (e.cbutton.button) {
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					downs[KEY_UP] = true;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					downs[KEY_DOWN] = true;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					downs[KEY_LEFT] = true;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					downs[KEY_RIGHT] = true;
					break;
				case SDL_CONTROLLER_BUTTON_BACK:
					downs[KEY_QUIT] = true;
					break;
				case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
					downs[KEY_RESET] = true;
					break;
				case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
					downs[KEY_SPECIAL] = true;
					break;
				case SDL_CONTROLLER_BUTTON_START:
					downs[KEY_SELECT] = true;
					break;
				case SDL_CONTROLLER_BUTTON_A:
					downs[KEY_FIRE] = true;
					break;
				case SDL_CONTROLLER_BUTTON_X:
					downs[KEY_GRAB] = true;
					break;
				case SDL_CONTROLLER_BUTTON_B:
					downs[KEY_USE] = true;
					break;
				case SDL_CONTROLLER_BUTTON_Y:
					downs[KEY_SPECIAL] = true;
					break;
				default:
					break;
			}
		} else if (e.type == SDL_CONTROLLERBUTTONUP) {
			switch (e.cbutton.button) {
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					presses[KEY_UP] = true;
					downs[KEY_UP] = false;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					presses[KEY_DOWN] = true;
					downs[KEY_DOWN] = false;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					presses[KEY_LEFT] = true;
					downs[KEY_LEFT] = false;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					presses[KEY_RIGHT] = true;
					downs[KEY_RIGHT] = false;
					break;
				case SDL_CONTROLLER_BUTTON_BACK:
					presses[KEY_QUIT] = true;
					downs[KEY_QUIT] = false;
					break;
				case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
					presses[KEY_RESET] = true;
					downs[KEY_RESET] = false;
					break;
				case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
					presses[KEY_SPECIAL] = true;
					downs[KEY_SPECIAL] = false;
					break;
				case SDL_CONTROLLER_BUTTON_START:
					presses[KEY_SELECT] = true;
					downs[KEY_SELECT] = false;
					break;
				case SDL_CONTROLLER_BUTTON_A:
					presses[KEY_FIRE] = true;
					downs[KEY_FIRE] = false;
					break;
				case SDL_CONTROLLER_BUTTON_X:
					presses[KEY_GRAB] = true;
					downs[KEY_GRAB] = false;
					break;
				case SDL_CONTROLLER_BUTTON_B:
					presses[KEY_USE] = true;
					downs[KEY_USE] = false;
					break;
				case SDL_CONTROLLER_BUTTON_Y:
					presses[KEY_SPECIAL] = true;
					downs[KEY_SPECIAL] = false;
					break;
				default:
					break;
			}
		} else if (e.type == SDL_KEYDOWN) {
			int idx = key_map_index_for_key(e.key.keysym.sym, key_map);
			if (idx >= 0 && idx < NUM_KEYS) {
				downs[idx] = true;
			}
		} else if (e.type == SDL_KEYUP) {
			int idx = key_map_index_for_key(e.key.keysym.sym, key_map);
			if (idx >= 0 && idx < NUM_KEYS) {
				presses[idx] = true;
				downs[idx] = false;
			}
		} else if (e.type == SDL_MOUSEBUTTONDOWN) {
			//printf("mouse down: (%d, %d)\n", e.button.x, e.button.y);
			mouse->x = e.button.x;
			mouse->y = e.button.y;
			switch (e.button.button)
			{
				case SDL_BUTTON_LEFT:
					mouse->ldown = true;
					break;
				case SDL_BUTTON_RIGHT:
					mouse->rdown = true;
					break;
				default:
					mouse->odown = true;
					break;
			}
		} else if (e.type == SDL_MOUSEBUTTONUP) {
			//printf("mouse up: (%d, %d)\n", e.button.x, e.button.y);
			switch (e.button.button)
			{
				case SDL_BUTTON_LEFT:
					mouse->ldown = false;
					break;
				case SDL_BUTTON_RIGHT:
					mouse->rdown = false;
					break;
				default:
					mouse->odown = false;
					break;
			}
		//} else if (e.type == SDL_MOUSEMOTION) {
		//	//printf("mouse motion: (%d, %d)\n", e.motion.x, e.motion.y);
		//	mouse->x = e.motion.x;
		//	mouse->y = e.motion.y;
		}
	}
	SDL_GetMouseState(&mouse->x, &mouse->y);
}

void set_default_key_map(int *key_map) {
	key_map[KEY_QUIT] =  SDLK_ESCAPE;
	key_map[KEY_LEFT] =  SDLK_LEFT;
	key_map[KEY_RIGHT] = SDLK_RIGHT;
	key_map[KEY_UP] =    SDLK_UP;
	key_map[KEY_DOWN] =  SDLK_DOWN;
	key_map[KEY_JUMP] =  SDLK_w;
	key_map[KEY_FIRE] = SDLK_SPACE;
	key_map[KEY_SPECIAL] = SDLK_s;
	key_map[KEY_USE] = SDLK_LGUI;
	key_map[KEY_GRAB] = SDLK_LALT;
	key_map[KEY_SELECT] = SDLK_RETURN;
	key_map[KEY_RESET] = SDLK_r;
}

void set_wasd_key_map(int *key_map) {
	key_map[KEY_QUIT] =  SDLK_ESCAPE;
	key_map[KEY_LEFT] =  SDLK_a;
	key_map[KEY_RIGHT] = SDLK_d;
	key_map[KEY_UP] =    SDLK_w;
	key_map[KEY_DOWN] =  SDLK_s;
	key_map[KEY_JUMP] =  SDLK_UP;
	key_map[KEY_FIRE] = SDLK_RIGHT;
	key_map[KEY_SPECIAL] = SDLK_RALT;
	key_map[KEY_USE] = SDLK_DOWN;
	key_map[KEY_GRAB] = SDLK_LEFT;
	key_map[KEY_SELECT] = SDLK_RETURN;
	key_map[KEY_RESET] = SDLK_SPACE;
}

void check_sdl_error(int line)
{
	const char *error = SDL_GetError();
	if (error != NULL) {
		printf("SDL error: %s\n", error);
		if (line != -1) {
			printf("Line: %d\n", line);
		}
		SDL_ClearError();
	}
}

void print_sdl_gl_attributes()
{
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	printf("SDL_GL_CONTEXT_MAJOR_VERSION: %d\n", value);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	printf("SDL_GL_CONTEXT_MINOR_VERSION: %d\n", value);
}
