#ifndef KEYBOARD_H_
#define KEYBOARD_H_

typedef enum modifiers {
    LEFT_CTRL = 1 << 0,
    LEFT_SHIFT = 1 << 1,
    LEFT_ALT = 1 << 2,
    LEFT_SUPER = 1 << 3,

    RIGHT_CTRL = 1 << 4,
    RIGHT_SHIFT = 1 << 5,
    RIGHT_ALT = 1 << 6,
    RIGHT_SUPER = 1 << 7
} modifiers;

typedef enum key_state {
    PRESSED = 0,
    RELEASED = 1,
    AUTO_REPEAT = 2
} key_state;

#endif // KEYBOARD_H_
