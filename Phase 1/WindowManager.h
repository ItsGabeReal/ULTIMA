/**
 * WindowManager.h
 *
 * Helper class to make using nCurses windows easier.
 *
 * @author Colin Christy
 * @date 3/29/2026
 */

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <ncurses.h>
#include <pthread.h>
#include <string>
#include "Sema.h"
#include "Sched.h"

class WindowManager
{
private:
    Semaphore window_lock;

    WINDOW* Log_Win;

    int thread_id; // Thread ID where the window manager is running

    void display_screen_data();

    void display_help(WINDOW *Win);
public:

    WindowManager(int main_thread_id, Scheduler* scheduler);
    ~WindowManager();

    /**
     * Creates a new nCurses window with the given parameters.
     *
     * @param height Height of the window
     * @param width Width of the window
     * @param starty Y coordinate for top left corner
     * @param startx X coordinate for top left corner
     */
    WINDOW *create_window(int thread_id, int height, int width, int starty, int startx);

    /**
     * Writes the provided text to the specified window.
     *
     * @param Win Window to output text
     * @param text Text to output
     */
    void write_window(WINDOW *Win, int thread_id, std::string text);

    /**
     * Writes the provide text to the specified window and coordinates.
     *
     * @param Win Window to output text
     * @param y Y coordinate to output text
     * @param x X coordinate to output text
     * @param text Text to output
     */
    void write_window(WINDOW *Win, int thread_id, int y, int x, std::string text);

    /**
     * Clears the specified window.
     * 
     * @param Win Window to clear
     */
    void clear_window(WINDOW *Win, int thread_id);

    void log(std::string message);
};
#endif