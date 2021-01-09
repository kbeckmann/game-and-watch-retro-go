#include "odroid_system.h"
#include "odroid_input.h"

#include <SDL2/SDL.h>
static odroid_gamepad_state_t out_state;

void odroid_input_read_gamepad(odroid_gamepad_state_t* out_state)
{
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            // printf("Press %d\n", event.key.keysym.sym);
            switch (event.key.keysym.sym) {
            case SDLK_x:
                out_state->values[ODROID_INPUT_A] = 1;
                break;
            case SDLK_z:
                out_state->values[ODROID_INPUT_B] = 1;
                break;
            case SDLK_LSHIFT:
                out_state->values[ODROID_INPUT_START] = 1;
                break;
            case SDLK_LCTRL:
                out_state->values[ODROID_INPUT_SELECT] = 1;
                break;
            case SDLK_UP:
                out_state->values[ODROID_INPUT_UP] = 1;
                break;
            case SDLK_DOWN:
                out_state->values[ODROID_INPUT_DOWN] = 1;
                break;
            case SDLK_LEFT:
                out_state->values[ODROID_INPUT_LEFT] = 1;
                break;
            case SDLK_RIGHT:
                out_state->values[ODROID_INPUT_RIGHT] = 1;
                break;
            case SDLK_ESCAPE:
                exit(1);
                break;
            default:
                break;
            }
        } else if (event.type == SDL_KEYUP) {
            // printf("Release %d\n", event.key.keysym.sym);
            switch (event.key.keysym.sym) {
            case SDLK_x:
                out_state->values[ODROID_INPUT_A] = 0;
                break;
            case SDLK_z:
                out_state->values[ODROID_INPUT_B] = 0;
                break;
            case SDLK_LSHIFT:
                out_state->values[ODROID_INPUT_START] = 0;
                break;
            case SDLK_LCTRL:
                out_state->values[ODROID_INPUT_SELECT] = 0;
                break;
            case SDLK_UP:
                out_state->values[ODROID_INPUT_UP] = 0;
                break;
            case SDLK_DOWN:
                out_state->values[ODROID_INPUT_DOWN] = 0;
                break;
            case SDLK_LEFT:
                out_state->values[ODROID_INPUT_LEFT] = 0;
                break;
            case SDLK_RIGHT:
                out_state->values[ODROID_INPUT_RIGHT] = 0;
                break;
            default:
                break;
            }
        }
    }
}

void odroid_input_wait_for_key(odroid_gamepad_key_t key, bool pressed)
{
}

