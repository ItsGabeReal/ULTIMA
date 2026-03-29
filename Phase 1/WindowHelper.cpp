#include "WindowHelper.h"

WINDOW *WindowHelper::create_window(int height, int width, int starty, int startx)
{
    pthread_mutex_lock(&myMutex);
    WINDOW *Win;

    Win = newwin(height, width, starty, startx);
    scrollok(Win, TRUE); // Allow scrolling
    scroll(Win);         // Scroll the window
    box(Win, 0, 0);      // 0, 0 gives default characters for lines

    wrefresh(Win); // Draw the window
    pthread_mutex_unlock(&myMutex);
    return Win;
}

void WindowHelper::write_window(WINDOW *Win, std::string text)
{
    pthread_mutex_lock(&myMutex);
    wprintw(Win, "%s", (char*)text.c_str()); // format string to avoid nonliteral string (-Wformat-security warning on build)
    box(Win, 0, 0);

    wrefresh(Win);
    pthread_mutex_unlock(&myMutex);
}

void WindowHelper::write_window(WINDOW *Win, int y, int x, std::string text)
{
    pthread_mutex_lock(&myMutex);
    mvwprintw(Win, y, x, "%s", (char*)text.c_str()); // format string to avoid nonliteral string (-Wformat-security warning on build)
    box(Win, 0, 0);

    wrefresh(Win);
    pthread_mutex_unlock(&myMutex);
}

void WindowHelper::clear_window(WINDOW *Win)
{
    pthread_mutex_lock(&myMutex);
    wclear(Win);
    pthread_mutex_unlock(&myMutex);
}