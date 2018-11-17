#ifndef APP_H
#define APP_H

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

typedef struct App_t {
  struct AppRun_t {
    int loop;
    int mode[32];
  } run;
  struct AppGraphics_t {
    SDL_Renderer *renderer;
    SDL_Window *window;
  } graphics;
  struct AppTime_t {
    Uint32 ticks;
    Uint32 delta;
    Uint32 timer[32];
  } time;
  struct AppMouse_t {
    int left, middle, right, wheel;
    int x, y;
  } mouse;
  struct AppKeyboard_t {
    int key[32];
    SDL_Keycode keycode[32];
  } keyboard;
} App;

int APP_Init(App *, int, int, const char *);
void APP_GetInput(App *);
void APP_SetColor(App*, Uint8, Uint8, Uint8, Uint8);
void APP_FillRect(App *, int, int, int, int);
void APP_DrawRect(App *, int, int, int, int);
void APP_DrawLine(App *, int, int, int, int);
void APP_DrawPoint(App *, int, int);
void APP_Clear(App *);
void APP_Render(App *);
void APP_MatchFPS(App *, Uint32);
void APP_Quit(App *);

#endif
