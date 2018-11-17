#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "APP.h"

#define TITLE "CHIP-8"
#define SIZE 8
#define WIDTH 64 * SIZE
#define HEIGHT 32 * SIZE

#define CPF 10 /* Cycles Per Frame */

#define KEY_0 0
#define KEY_1 1
#define KEY_2 2
#define KEY_3 3
#define KEY_4 4
#define KEY_5 5
#define KEY_6 6
#define KEY_7 7
#define KEY_8 8
#define KEY_9 9
#define KEY_A 10
#define KEY_B 11
#define KEY_C 12
#define KEY_D 13
#define KEY_E 14
#define KEY_F 15

#define FONTSET					\
  {						\
    0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */	\
    0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */	\
    0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */	\
    0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */	\
    0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */	\
    0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */	\
    0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */	\
    0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */	\
    0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */	\
    0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */	\
    0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */	\
    0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */	\
    0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */	\
    0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */	\
    0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */	\
    0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */	\
  }						\

typedef struct Chip8_t {
  unsigned char ram[4096];
  unsigned short stack[16];
  unsigned char V[16];
  unsigned short I;
  unsigned char dt;
  unsigned char st;
  unsigned short pc;
  unsigned char sp;
  unsigned char keypad[16];
  unsigned char display[32*64];
} Chip8;

int load(Chip8 *, const char *);
void cycle(Chip8 *);
void update(Chip8 *);
void init(Chip8 *);
void process(App *, Chip8 *);
void draw(App *, Chip8 *);

void init(Chip8 *cpu) {
  int i;
  unsigned char font[] = FONTSET;
  for (i = 0; i < 0x50; i++) cpu->ram[i] = font[i];
  for (i = 0x50; i < 4096 + 80; i++) cpu->ram[i] = 0;
  for (i = 0; i < 0x10; i++) cpu->keypad[i] = 0;
  for (i = 0; i < 32*64; i++) cpu->display[i] = 0;
  for (i = 0; i < 0x10; i++) cpu->V[i] = 0;
  cpu->pc = 0x200;
  cpu->sp = 0;
  cpu->I = 0;
}

void draw(App *app, Chip8 * cpu) {
  int i, j;
  for (i = 0; i < 32; i++) {
    for (j = 0; j < 64; j++) {
      if (cpu->display[i * 64 + j] == 1)
	APP_SetColor(app, 255, 255, 255, SDL_ALPHA_OPAQUE);
      else APP_SetColor(app, 0, 0, 0, SDL_ALPHA_OPAQUE);
      APP_FillRect(app, j * SIZE, i * SIZE, SIZE, SIZE);
    }
  }
}

void process(App *app, Chip8 *cpu) {
  int i;
  for (i = 0; i < 16; i++) cpu->keypad[i] = app->keyboard.key[i];
}

int load(Chip8 *cpu, const char *path) {
  int i, c;
  FILE *src = fopen(path, "r");
  if (src != NULL) {
    i = 0;
    while ((c = fgetc(src)) != EOF) {
      cpu->ram[0x200 + i] = c;
      i++;
    }
    fclose(src);
    return 0;
  }
  return -1;
}

void cycle(Chip8 *cpu) {
  unsigned short opcode = (cpu->ram[cpu->pc] << 8) | cpu->ram[cpu->pc + 1];
  unsigned short nnn = opcode & 0x0FFF;
  unsigned short n = opcode & 0x000F;
  unsigned short x = (opcode & 0x0F00) >> 8;
  unsigned short y = (opcode & 0x00F0) >> 4;
  unsigned short kk = opcode & 0x00FF;
  int i, j;
  unsigned short tmp;
  switch (opcode) {
  case 0x00E0: /* CLS */
    for (i = 0; i < 32 * 64; i++) cpu->display[i] = 0;
    cpu->pc += 2;
    break;
  case 0x00EE: /* RET */
    cpu->sp -= 1;
    cpu->pc = cpu->stack[cpu->sp] + 2;
    break;
  default:
    switch (opcode & 0xF000) {
    case 0x0000: /* SYS addr */
      fprintf(stderr, "Unknown %x\n", opcode);
      break;
    case 0x1000: /* JP addr */
      cpu->pc = nnn;
      break;
    case 0x2000: /* CALL addr */
      cpu->stack[cpu->sp] = cpu->pc;
      cpu->sp += 1;
      cpu->pc = nnn;
      break;
    case 0x3000: /* SE Vx, byte */
      if (cpu->V[x] == kk) cpu->pc += 4;
      else cpu->pc += 2;
      break;
    case 0x4000: /* SNE Vx, byte */
      if (cpu->V[x] != kk) cpu->pc += 4;
      else cpu->pc += 2;
      break;
    case 0x5000:
      switch (opcode & 0x000F) {
      case 0x0000: /* SE Vx, Vy */
	if (cpu->V[x] == cpu->V[y]) cpu->pc += 4;
	else cpu->pc += 2;
	break;
      default: /* error */
	fprintf(stderr, "Unknown %x\n", opcode);
	break;
      }
      break;
    case 0x6000: /* LD Vx, byte */
      cpu->V[x] = kk;
      cpu->pc += 2;
      break;
    case 0x7000: /* ADD Vx, byte */
      cpu->V[x] += kk;
      cpu->pc += 2;
      break;
    case 0x8000:
      switch (opcode & 0x000F) {
      case 0x0000: /* LD Vx, Vy */
	cpu->V[x] = cpu->V[y];
	cpu->pc += 2;
	break;
      case 0x0001: /* OR Vx, Vy */
	cpu->V[x] |= cpu->V[y];
	cpu->pc += 2;
	break;
      case 0x0002: /*AND Vx, Vy */
	cpu->V[x] &= cpu->V[y];
	cpu->pc += 2;
	break;
      case 0x0003: /* XOR Vx, Vy */
	cpu->V[x] ^= cpu->V[y];
	cpu->pc += 2;
	break;
      case 0x0004: /* ADD Vx, Vy */
	tmp = cpu->V[x] + cpu->V[y];
	cpu->V[x] = tmp & 0x00FF;
	cpu->V[0x0F] = tmp >> 8;
	cpu->pc += 2;
	break;
      case 0x0005: /* SUB Vx, Vy */
	if (cpu->V[x] > cpu->V[y]) cpu->V[0x0F] = 1;
	else cpu->V[0x0F] = 0;
	cpu->V[x] -= cpu->V[y];
	cpu->pc += 2;
	break;
      case 0x0006: /* SHR Vx {, Vy} */
	cpu->V[0x0F] = cpu->V[x] & 0x01;
	cpu->V[x] >>= 1;
	cpu->pc += 2;
	break;
      case 0x0007: /* SUBN Vx, Vy */
	if (cpu->V[y] > cpu->V[x]) cpu->V[0x0F] = 1;
	else cpu->V[0x0F] = 0;
	cpu->V[x] = cpu->V[y] - cpu->V[x];
	cpu->pc += 2;
	break;
      case 0x000E: /* SHL Vx {, Vy} */
	cpu->V[0x0F] = (cpu->V[x] >> 8) >> 7;
	cpu->V[x] <<= 1;
	cpu->pc += 2;
	break;
      default: /* error */
	fprintf(stderr, "Unknown %x\n", opcode);
	break;
      }
      break;
    case 0x9000:
      switch (opcode & 0x000F) {
      case 0x0000: /* OR Vx, Vy */
	if (cpu->V[x] != cpu->V[y]) cpu->pc += 4;
	else cpu->pc += 2;
	break;
      default: /* error */
	fprintf(stderr, "Unknown %x\n", opcode);
	break;
      }
      break;
    case 0xA000: /* LD I, addr */
      cpu->I = nnn;
      cpu->pc += 2;
      break;
    case 0xB000: /* JP V0, addr */
      cpu->pc = cpu->V[0] + nnn;
      break;
    case 0xC000: /* RND Vx, byte */
      cpu->V[x] = (rand() % 0xFF) & kk;
      cpu->pc += 2;
      break;
    case 0xD000: /* DRW Vx, Vy, nibble */
      cpu->V[0x0F] = 0;
      for (i = 0; i < n; i++)
	for (j = 0; j < 8 ; j++)
	  if ((cpu->ram[cpu->I + i] & (0x80 >> j)) != 0) {
	    cpu->display[64 * (cpu->V[y] + i) + cpu->V[x] + j] ^= 1;
	    if (cpu->display[64 * (cpu->V[y] + i) + cpu->V[x] + j] == 0)
	      cpu->V[0x0F] = 1;
	  }
      cpu->pc += 2;
      break;
    case 0xE000:
      switch (opcode & 0x00FF) {
      case 0x009E: /* SKP Vx */
	if (cpu->keypad[cpu->V[x]] == 1) cpu->pc += 4;
	else cpu->pc += 2;
	break;
      case 0x00A1: /* SKNP Vx */
	if (cpu->keypad[cpu->V[x]] == 0) cpu->pc += 4;
	else cpu->pc += 2;
	break;
      default: /* error */
	fprintf(stderr, "Unknown %x\n", opcode);
	break;
      }
      break;
    case 0xF000:
      switch (opcode & 0x00FF) {
      case 0x0007: /* LD Vx, DT */
	cpu->V[x] = cpu->dt;
	cpu->pc += 2;
	break;
      case 0x000A: /* LD Vx, K */
	i = 0;
	tmp = 0;
	while ((i < 16) && (tmp == 0)) {
	  if (cpu->keypad[i] == 1) tmp = 1;
	  else i++;
	}
	if (tmp == 1) {
	  cpu->V[x] = i;
	  cpu->pc += 2;
	}
	break;
      case 0x0015: /* LD DT, Vx */
	cpu->dt = cpu->V[x];
	cpu->pc += 2;
	break;
      case 0x0018: /* LD ST, Vx */
	cpu->st = cpu->V[x];
	cpu->pc += 2;
	break;
      case 0x001E: /* ADD I, Vx */
	cpu->I += cpu->V[x];
	cpu->V[0x0F] = cpu->I >> 12;
	cpu->pc += 2;
	break;
      case 0x0029: /* LD F, Vx */
	cpu->I = cpu->V[x] * 0x05; 
	cpu->pc += 2;
	break;
      case 0x0033: /* LD B, Vx */
	cpu->ram[cpu->I] = cpu->V[x] / 100;
	cpu->ram[cpu->I + 1] = (cpu->V[x] / 10) % 10;
	cpu->ram[cpu->I + 2] = cpu->V[x] % 10;
	cpu->pc += 2;
	break;
      case 0x0055: /* LD [I], Vx */
	for (i = 0; i < x + 1; i++) cpu->ram[cpu->I + i] = cpu->V[i];
	cpu->pc += 2;
	break;
      case 0x0065: /* LD Vx, [I] */
	for (i = 0; i < x + 1; i++) cpu->V[i] = cpu->ram[cpu->I + i];
	cpu->pc += 2;
	break;
      default: /* error */
	fprintf(stderr, "Unknown %x\n", opcode);
	break;
      }
      break;
    default: /* error */
      fprintf(stderr, "Unknown %x\n", opcode);
      break;
    }
    break;
  }
}

void update(Chip8 *cpu) {
  if (cpu->dt > 0) cpu->dt-=1;
  if (cpu->st > 0) {
    printf("\a");
    fflush(stdout);
    cpu->st-=1;
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  App app;
  Chip8 cpu;
  if (argc != 2) {
    fprintf(stderr, "Wrong number of parameters\n");
    return 1;
  }
  if (APP_Init(&app, WIDTH, HEIGHT, TITLE) < 0) {
    APP_Quit(&app);
    return 1;
  }
  init(&cpu);
  if (load(&cpu, argv[1]) < 0) {
    fprintf(stderr, "File not found: %s\n", argv[1]);
    return 3;
  }
  app.keyboard.keycode[KEY_0] = SDLK_x;
  app.keyboard.keycode[KEY_1] = SDLK_1;
  app.keyboard.keycode[KEY_2] = SDLK_2;
  app.keyboard.keycode[KEY_3] = SDLK_3;
  app.keyboard.keycode[KEY_4] = SDLK_q;
  app.keyboard.keycode[KEY_5] = SDLK_w;
  app.keyboard.keycode[KEY_6] = SDLK_e;
  app.keyboard.keycode[KEY_7] = SDLK_a;
  app.keyboard.keycode[KEY_8] = SDLK_s;
  app.keyboard.keycode[KEY_9] = SDLK_d;
  app.keyboard.keycode[KEY_A] = SDLK_z;
  app.keyboard.keycode[KEY_B] = SDLK_c;
  app.keyboard.keycode[KEY_C] = SDLK_4;
  app.keyboard.keycode[KEY_D] = SDLK_r;
  app.keyboard.keycode[KEY_E] = SDLK_f;
  app.keyboard.keycode[KEY_F] = SDLK_v;
  app.run.loop = 1;
  while (app.run.loop) {
    APP_GetInput(&app);
    process(&app, &cpu);
    int i;
    for (i = 0; i < CPF; i++) cycle(&cpu);
    APP_SetColor(&app, 0, 0, 0, SDL_ALPHA_OPAQUE);
    APP_Clear(&app);
    draw(&app, &cpu);
    APP_Render(&app);
    update(&cpu);
    APP_MatchFPS(&app, 60);
  }
  APP_Quit(&app);
  return 0;
}
