/*
   Copyright 2023 Eduardo Antunes S. Vieira <eduardoantunes986@gmail.com>

   This file is part of libre-nes.

   libre-nes is free software: you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation, either version 3 of the License, or (at your option) any later
   version.

   libre-nes is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with
   libre-nes. If not, see <https://www.gnu.org/licenses/>.
*/

#include <SDL.h>
#include <string.h>
#include "SDL_render.h"
#include "SDL_video.h"
#include "picture/screen.h"

// Initialize the screen
int screen_init(Screen *screen, const char *title, int scale) {
    screen->refresh_flag = false;
    // Leave the pixel buffer as is I guess, and initialize the SDL stuff

    // SDL Window initialization
    screen->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, WIDTH * scale, HEIGHT * scale,
        SDL_WINDOW_SHOWN);
    if(screen->window == NULL) return 1;

    // SDL Renderer initialization
    screen->renderer = SDL_CreateRenderer(screen->window, -1, SDL_RENDERER_ACCELERATED);
    if(screen->renderer == NULL) {
        SDL_DestroyWindow(screen->window);
        return 2;
    }
    SDL_RenderSetLogicalSize(screen->renderer, WIDTH, HEIGHT);

    // SDL Texture initialization
    screen->texture = SDL_CreateTexture(screen->renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if(screen->texture == NULL) {
        SDL_DestroyWindow(screen->window);
        return 3;
    }
    return 0; // no errors
}

// Fill a pixel with an RGBA color
void screen_set_pixel(Screen *screen, uint8_t x, uint8_t y, uint32_t rgba_color) {
    screen->pixels[WIDTH * y + x] = rgba_color;
    screen->refresh_flag = true;
}

// Refresh the graphics if the refresh flag is set
void screen_refresh(Screen *screen) {
    // Only do something if requested
    if(!screen->refresh_flag) return;
    screen->refresh_flag = false;

    // Lock the SDL texture and pass in the pixel buffer
    int pitch;
    uint32_t *texture_buffer;
    SDL_LockTexture(screen->texture, NULL, (void**) &texture_buffer, &pitch);
    memcpy(texture_buffer, screen->pixels, pitch * HEIGHT);

    // Unlock the SDL texture and render the new pixel buffer
    SDL_UnlockTexture(screen->texture);
    SDL_RenderClear(screen->renderer);
    SDL_RenderCopy(screen->renderer, screen->texture, NULL, NULL);
    SDL_RenderPresent(screen->renderer);
}
