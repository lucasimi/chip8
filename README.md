# chip8
A CHIP-8 virtual machine implemented in C with SDL2

## Usage
Compile with `cc chip8.c APP.c -o chip8 -lSDL2`.

Run with `./chip8 INVADERS`.

![Space Invaders](https://github.com/lucasimi/chip8/blob/master/invaders1.png)
![Space Invaders](https://github.com/lucasimi/chip8/blob/master/invaders2.png)

## Keypad Layout
chip8 keypad uses the following layout:

```
1 2 3 4       1 2 3 C 
Q W E R  -->  4 5 6 D 
A S D F       7 8 9 E 
Z X C V       A 0 B F 
```

## How it works

1. Initialization: ram and display get cleared, fonts get loaded from 0x00 onwards, registers are set to default values;
2. Loading: a rom file is loaded into ram, starting at 0x200;
3. Mainloop: The display is rendered with 60fps as target framerate. Each step of the mainloop performs the following actions:

  3.1. fetch-decode-execute cycle (x 10): get the opcode from ram, decide what to do, execute the instruction;
  
  3.2. rendering: clear the screen, draw a square for every pixel set to 1;
  
  3.3 update timers: decrease delay and sound timers, beep when sound timer is > 0.

### Notes
- sprites flicker when moving (accurate implementation)

### References
1. [Cowgood's CHIP-8](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
2. [Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
