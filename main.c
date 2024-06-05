#include "SDL2/include/SDL2/SDL.h"
#include "SDL2/include/SDL2/SDL_surface.h"
#include "la.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./buffer.h"
#include "./la.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH (FONT_WIDTH / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_SCALE 5

#undef main

void scc(int code) {
  if (code < 0) {
    fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    exit(1);
  }
}

void *scp(void *ptr) {
  if (ptr == NULL) {
    fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    exit(1);
  }
  return ptr;
}

SDL_Surface *surface_from_file(const char *file_path) {
  int width, height, n;
  unsigned char *pixels =
      stbi_load(file_path, &width, &height, &n, STBI_rgb_alpha);
  if (pixels == NULL) {
    fprintf(stderr, "ERROR: could not load file %s: %s\n", file_path,
            stbi_failure_reason());
    exit(1);
  }

#if SDL_BYTEORDER == SDL_BIG_EDNIAN
  const Uint32 rmask = 0xff000000;
  const Uint32 gmask = 0x00ff0000;
  const Uint32 bmask = 0x0000ff00;
  const Uint32 amask = 0x000000ff;
#else // little endian, like x86
  const Uint32 rmask = 0x000000ff;
  const Uint32 gmask = 0x0000ff00;
  const Uint32 bmask = 0x00ff0000;
  const Uint32 amask = 0xff000000;
#endif

  const int depth = 32;
  const int pitch = 4 * width;

  return scp(SDL_CreateRGBSurfaceFrom((void *)pixels, width, height, depth,
                                      pitch, rmask, gmask, bmask, amask));
}

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

typedef struct {
  SDL_Texture *spritesheet;
  SDL_Rect glyph_table[ASCII_DISPLAY_HIGH - ASCII_DISPLAY_LOW + 1];
} Font;

Font font_load_from_file(SDL_Renderer *renderer, const char *file_path) {
  Font font = {0};
  SDL_Surface *font_surface = surface_from_file(file_path);
  scc(SDL_SetColorKey(font_surface, SDL_TRUE, 0xFF000000));
  font.spritesheet = scp(SDL_CreateTextureFromSurface(renderer, font_surface));

  SDL_FreeSurface(font_surface);

  for (size_t ascii = ASCII_DISPLAY_LOW; ascii < ASCII_DISPLAY_HIGH; ++ascii) {
    const size_t index = ascii - ASCII_DISPLAY_LOW;
    const size_t col = index % FONT_COLS;
    const size_t row = index / FONT_COLS;

    font.glyph_table[index] = (SDL_Rect){
        .x = col * FONT_CHAR_WIDTH,
        .y = row * FONT_CHAR_HEIGHT,
        .w = FONT_CHAR_WIDTH,
        .h = FONT_CHAR_HEIGHT,
    };
  }

  return font;
}

void render_char(SDL_Renderer *renderer, const Font *font, char c, Vec2f pos,
                 float scale) {

  const SDL_Rect dst = {
      .x = (int)floorf(pos.x),
      .y = (int)floorf(pos.y),
      .w = (int)floorf(FONT_CHAR_WIDTH * scale),
      .h = (int)floorf(FONT_CHAR_HEIGHT * scale),
  };

  size_t index = '?' - ASCII_DISPLAY_LOW;
  if (ASCII_DISPLAY_LOW <= c && c <= ASCII_DISPLAY_HIGH) {
    index = c - ASCII_DISPLAY_LOW;
  }

  // assert(c >= ASCII_DISPLAY_LOW);
  // assert(c <= ASCII_DISPLAY_HIGH);

  scc(SDL_RenderCopy(renderer, font->spritesheet, &font->glyph_table[index],
                     &dst));
}

void set_texture_color(SDL_Texture *texture, Uint32 color) {
  scc(SDL_SetTextureColorMod(texture, (color >> (8 * 0)) & 0xff,
                             (color >> (8 * 1)) & 0xff,
                             (color >> (8 * 2)) & 0xff));

  scc(SDL_SetTextureAlphaMod(texture, (color >> (8 * 3) & 0xff)));
}

void render_text_sized(SDL_Renderer *renderer, Font *font, const char *text,
                       size_t text_size, Vec2f pos, Uint32 color, float scale) {

  set_texture_color(font->spritesheet, color);

  Vec2f pen = pos;
  for (size_t i = 0; i < text_size; ++i) {
    render_char(renderer, font, text[i], pen, scale);
    pen.x += FONT_CHAR_WIDTH * scale;
  }
}

Line line = {0};
size_t cursor = 0;

#define UNHEX(color)                                                           \
  (color) >> (8 * 0) & 0xFF, (color) >> (8 * 1) & 0xFF,                        \
      (color) >> (8 * 2) & 0xFF, (color) >> (8 * 3) & 0xFF

void render_cursor(SDL_Renderer *renderer, const Font *font) {

  const Vec2f pos = vec2f(cursor * (float)FONT_CHAR_WIDTH * FONT_SCALE, 0.0f);

  const SDL_Rect rect = {
      .x = (int)floorf(pos.x),
      .y = (int)floorf(pos.y),
      .w = FONT_CHAR_WIDTH * FONT_SCALE,
      .h = FONT_CHAR_HEIGHT * FONT_SCALE,
  };

  scc(SDL_SetRenderDrawColor(renderer, UNHEX(0xFFFFFFFF)));
  scc(SDL_RenderFillRect(renderer, &rect));

  set_texture_color(font->spritesheet, 0xFF000000);

  if (cursor < line.size) {
    render_char(renderer, font, line.chars[cursor], pos, FONT_SCALE);
  }
}

// TODO: Multiple lines
// TODO: Save/Load file
// TODO: Jump forward/backward by word
// TODO: Delete a word
// TODO: Blinking cursor

int main(int argc, char **argv) {

  (void)argc;
  (void)argv;

  scc(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window *window =
      scp(SDL_CreateWindow("7ext1", 100, 100, 800, 600, SDL_WINDOW_RESIZABLE));

  SDL_Renderer *renderer =
      scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

  SDL_Surface *iconSurface = SDL_LoadBMP("./cutecat.bmp");
  if (!iconSurface) {
    fprintf(stderr, "Could not load icon: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_SetWindowIcon(window, iconSurface);

  Font font = font_load_from_file(renderer, "./charmap-oldschool_white.png");

  bool quit = false;
  while (!quit) {
    SDL_Event event = {0};
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        quit = true;
      } break;

      case SDL_KEYDOWN: {
        switch (event.key.keysym.sym) {
        case SDLK_BACKSPACE: {
          if (line.size > 0) {
            line_backspace(&line, cursor);
            if (cursor > 0) {
              cursor--;
            }
          }
        } break;

        case SDLK_DELETE: {
          line_delete(&line, cursor);
        } break;

        case SDLK_LEFT: {
          if (cursor > 0) {
            cursor--;
          }
        } break;

        case SDLK_RIGHT: {
          if (cursor < line.size) {
            cursor++;
          }
        } break;
        }

      } break;

      case SDL_TEXTINPUT: {
        line_insert_text_before(&line, event.text.text, cursor);
        cursor += strlen(event.text.text);
      } break;
      }
    }

    scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
    scc(SDL_RenderClear(renderer));

    render_text_sized(renderer, &font, line.chars, line.size, vec2f(0.0, 0.0),
                      0xFFFFFFFF, FONT_SCALE);
    render_cursor(renderer, &font);

    SDL_RenderPresent(renderer);
  }

  SDL_Quit();

  return 0;
}
