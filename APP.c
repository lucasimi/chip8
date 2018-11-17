#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "APP.h"

int APP_Init(App *app, int width, int height, const char *title) {
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) return -1;
  app->graphics.window =
    SDL_CreateWindow(title,
		     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		     width, height, SDL_WINDOW_SHOWN);
  if (app->graphics.window == NULL) return -1;
  app->graphics.renderer =
    SDL_CreateRenderer(app->graphics.window, -1,
		       SDL_RENDERER_ACCELERATED |
		       SDL_RENDERER_PRESENTVSYNC);
  if (app->graphics.renderer == NULL) return -1;
  app->time.ticks = SDL_GetTicks();
  app->time.delta = 0;
  app->mouse.left = 0;
  app->mouse.middle = 0;
  app->mouse.right = 0;
  app->mouse.x = 0;
  app->mouse.y = 0;
  app->run.loop = 0;
  int i;
  for (i = 0; i < 32; i++) {
    app->keyboard.keycode[i] = 0;
    app->keyboard.key[i] = 0;
    app->run.mode[i] = 0;
    app->time.timer[i] = 0;
  }
  return 0;
}

void APP_GetInput(App *app) {
  SDL_Event e;
  int i;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT:
      app->run.loop = 0;
      break;
    case SDL_MOUSEMOTION:
      app->mouse.x = e.motion.x;
      app->mouse.y = e.motion.y;
      break;
    case SDL_MOUSEBUTTONDOWN:
      switch (e.button.button) {
      case SDL_BUTTON_LEFT: app->mouse.left = 1; break;
      case SDL_BUTTON_MIDDLE: app->mouse.middle = 1; break;
      case SDL_BUTTON_RIGHT: app->mouse.right = 1; break;
      default: break;
      }
      break;
    case SDL_MOUSEWHEEL:
      app->mouse.wheel = e.wheel.y;
      break;
    case SDL_MOUSEBUTTONUP:
      switch (e.button.button) {
      case SDL_BUTTON_LEFT: app->mouse.left = 0; break;
      case SDL_BUTTON_MIDDLE: app->mouse.middle = 0; break;
      case SDL_BUTTON_RIGHT: app->mouse.right = 0; break;
      default: break;
      }
      break;
    case SDL_KEYDOWN:
      for (i = 0; i < 32; i++)
	if (e.key.keysym.sym == app->keyboard.keycode[i]) {
	  app->keyboard.key[i] = 1;
	  break;
	}
      break;
    case SDL_KEYUP:
      for (i = 0; i < 32; i++)
	if (e.key.keysym.sym == app->keyboard.keycode[i]) { 
	  app->keyboard.key[i] = 0;
	  break;
	}
      break;
    default: break;
    }
  }
}

void APP_SetColor(App *app, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  SDL_SetRenderDrawColor(app->graphics.renderer, r, g, b, a);
}

void APP_Clear(App *app) {
  SDL_RenderClear(app->graphics.renderer);
}


void APP_DrawRect(App *app, int x, int y, int w, int h) {
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_RenderDrawRect(app->graphics.renderer, &rect);
}

void APP_FillRect(App *app, int x, int y, int w, int h) {
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  SDL_RenderFillRect(app->graphics.renderer, &rect);
}

void APP_DrawLine(App *app, int x0, int y0, int x1, int y1) {
  SDL_RenderDrawLine(app->graphics.renderer, x0, y0, x1, y1);
}

void APP_DrawPoint(App *app, int x, int y) {
  SDL_RenderDrawPoint(app->graphics.renderer, x, y);
}

void APP_Render(App *app) {
  SDL_RenderPresent(app->graphics.renderer);
}

void APP_MatchFPS(App *app, Uint32 fps) {
  Uint32 t = SDL_GetTicks();
  app->time.delta = t - app->time.ticks;
  if (1000 / fps > t - app->time.ticks)
    SDL_Delay(1000 / fps - (t - app->time.ticks));
  app->time.ticks = t;
}

void APP_Quit(App *app) {
  SDL_DestroyRenderer(app->graphics.renderer);
  SDL_DestroyWindow(app->graphics.window);
  SDL_Quit();
}
