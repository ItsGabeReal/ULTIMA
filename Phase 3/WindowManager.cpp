#include "WindowManager.h"

WindowManager::WindowManager(int thread_id, Scheduler* scheduler)
    : window_lock("Window")
{
    main_thread_id = thread_id;

    initscr();             // Start nCurses
    display_screen_data(); // Display the stdscr display geometry

    //----------------------- Create Log Window ------------------------
    Log_Win = create_window(main_thread_id, 14, 60, 30, 2);
    write_window(Log_Win, main_thread_id, 1, 5, " .....Log Window.....\n");
    write_window(Log_Win, main_thread_id, " ..........Main program started..........\n");
}

WindowManager::~WindowManager()
{
    write_window(Log_Win, 0, "Exiting WindowManager...");
    endwin();
}

WINDOW *WindowManager::create_window(int thread_id, int height, int width, int starty, int startx)
{
    window_lock.down(thread_id);
    WINDOW *Win;

    Win = newwin(height, width, starty, startx);
    scrollok(Win, TRUE); // Allow scrolling
    scroll(Win);         // Scroll the window
    box(Win, 0, 0);      // 0, 0 gives default characters for lines

    wrefresh(Win); // Draw the window
    window_lock.up(thread_id);
    return Win;
}

void WindowManager::write_window(WINDOW *Win, int thread_id, std::string text)
{
    window_lock.down(thread_id);
    wprintw(Win, "%s", (char*)text.c_str()); // format string to avoid nonliteral string (-Wformat-security warning on build)
    box(Win, 0, 0);

    wrefresh(Win);
    window_lock.up(thread_id);
}

void WindowManager::write_window(WINDOW *Win, int thread_id, int y, int x, std::string text)
{
    window_lock.down(thread_id);
    mvwprintw(Win, y, x, "%s", (char*)text.c_str()); // format string to avoid nonliteral string (-Wformat-security warning on build)
    box(Win, 0, 0);

    wrefresh(Win);
    window_lock.up(thread_id);
}

void WindowManager::clear_window(WINDOW *Win, int thread_id)
{
    window_lock.down(thread_id);
    wclear(Win);
    window_lock.up(thread_id);
}

void WindowManager::log(std::string message)
{
    write_window(Log_Win, main_thread_id, message);
}

void WindowManager::display_screen_data()
{
    int Y, X;
    int Max_Y, Max_X;

    start_color();
    // Define color pairs: (pair_number, foreground, background)
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    window_lock.down(main_thread_id);
    attron(COLOR_PAIR(1));          // Use color pair 1
    getmaxyx(stdscr, Max_Y, Max_X); // Get screen size
    wprintw(stdscr, "Initial Screen Height = %d, Initial Screen Width = %d\n", Max_Y, Max_X);
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(2)); // Use color pair 2
    getyx(stdscr, Y, X);   // Get current Y, X coordinate of the cursor
    wprintw(stdscr, "Current Y = %d, Current X = %d\n", Y, X);
    attroff(COLOR_PAIR(2));

    refresh();
    window_lock.up(main_thread_id);
}

