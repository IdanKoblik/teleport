#include "select.h"

#include <ncurses.h>

int select_keyboard(const device_entry *list, size_t count) {
    if (!list || count == 0)
        return -1;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    size_t cursor = 0;
    int selected = -1;

    while (1) {
        erase();
        mvprintw(0, 0, "select a keyboard (j/k or arrows, enter to confirm, q to cancel)");

        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        (void)cols;

        size_t visible = (size_t)(rows - 2);
        if (visible == 0) visible = 1;
        size_t top = cursor < visible ? 0 : cursor - visible + 1;
        size_t end = top + visible < count ? top + visible : count;

        for (size_t i = top; i < end; i++) {
            int row = (int)(i - top) + 2;
            if (i == cursor) attron(A_REVERSE);
            mvprintw(row, 0, "[%zu] %s -> %s", i, list[i].name, list[i].devnode);
            if (i == cursor) attroff(A_REVERSE);
        }

        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                if (cursor > 0) cursor--;
                break;
            case KEY_DOWN:
            case 'j':
                if (cursor + 1 < count) cursor++;
                break;
            case '\n':
            case KEY_ENTER:
                selected = (int)cursor;
                goto done;
            case 'q':
            case 27:
                goto done;
            default:
                break;
        }
    }

done:
    curs_set(1);
    keypad(stdscr, FALSE);
    echo();
    nocbreak();
    endwin();
    return selected;
}
