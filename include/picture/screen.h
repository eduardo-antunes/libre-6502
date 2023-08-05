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

// Graphics rendering for the NES

#ifndef LIBRE_NES_SCREEN_H
#define LIBRE_NES_SCREEN_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

#define WIDTH 256
#define HEIGHT 240

// Structure representing the NES screen. It holds the SDL objects necessary
// to also render the graphics to the actual computer screen
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t pixels[HEIGHT * WIDTH];
    bool refresh_flag;
} Screen;

// Initialize the screen
int screen_init(Screen *screen, const char *title, int scale);

// Fill a pixel with an RGBA color
void screen_set_pixel(Screen *screen, uint8_t x, uint8_t y, uint32_t rgba_color);

// Refresh the graphics if the refresh flag is set
void screen_refresh(Screen *screen);

#endif // LIBRE_NES_SCREEN_H
