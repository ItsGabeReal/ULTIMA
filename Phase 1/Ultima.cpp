// Code mostly from Lab 7 to get UI started
// Definitely can use organizing into separate files (We need Ultima.h)

#include <iostream>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdarg.h>
#include <termios.h>
#include <fcntl.h>
#include "Sema.h"
#include "Sched.h"
#include "WindowManager.h"

using namespace std;

#define MAIN_THREAD_ID 0

//--------------------------------------------------------
// State information for each thread.
// Mostly used in the future, but right now, every thread
// will have a RUNNING state until, we the user kills them
// either by specifically killing a thread number,
// or by quitting the program and killing all the threads.

// const int STARTED	= 0;
// const int READY		= 1;
// const int RUNNING 	= 2;
// const int BLOCKED	= 3;
// const int TERMINATED	= 4;

//--------------------------------------------------------
// Forward declaration
//--------------------------------------------------------
void display_help(WINDOW *Win);
//--------------------------------------------------------
void *perform_simple_output(void *arguments);
void *perform_cpu_work(void *arguments);
void *perform_io_work(void *arguments);
//--------------------------------------------------------

WindowManager wManager(MAIN_THREAD_ID);

//--------------------------------------------------------
// Data Structure for each thread.
// Note that each thread has access to its own WINDOW and should
// be able to display its output to its private window.
//
struct thread_data
{
    int thread_no;
    std::string thread_state;
    WINDOW *thread_win;
    bool kill_signal;
    int sleep_time;
    int thread_results;
    // other stuff
};

//--------------------------------------------------------
// This utility function simply extracts the Width and Height
// of the stdscr, and displays it.
//
// In addition to the screen geometry, we also extract the
// current cursor's Y,X coordinates and display them.
//
// Finally, this function shows how we can use colors in
// nCurses using functions such as start_color(), init_pair(),
// attron(), attroff()

// void display_screen_data()
// {
//     int Y, X;
//     int Max_Y, Max_X;

//     start_color();
//     // Define color pairs: (pair_number, foreground, background)
//     init_pair(1, COLOR_RED, COLOR_BLACK);
//     init_pair(2, COLOR_GREEN, COLOR_BLACK);

//     pthread_mutex_lock(&myMutex);
//     attron(COLOR_PAIR(1));          // Use color pair 1
//     getmaxyx(stdscr, Max_Y, Max_X); // Get screen size
//     wprintw(stdscr, "Initial Screen Height = %d, Initial Screen Width = %d\n", Max_Y, Max_X);
//     attroff(COLOR_PAIR(1));
//     pthread_mutex_unlock(&myMutex);

//     pthread_mutex_lock(&myMutex);
//     attron(COLOR_PAIR(2)); // Use color pair 2
//     getyx(stdscr, Y, X);   // Get current Y, X coordinate of the cursor
//     wprintw(stdscr, "Current Y = %d, Current X = %d\n", Y, X);
//     attroff(COLOR_PAIR(2));

//     refresh();
//     pthread_mutex_unlock(&myMutex);
// }

void display_help(WINDOW *Win)
{
    wManager.clear_window(Win, MAIN_THREAD_ID);
    wManager.write_window(Win, MAIN_THREAD_ID, 1, 1, "...Help...");
    wManager.write_window(Win, MAIN_THREAD_ID, 2, 1, "1 = Kill T1");
    wManager.write_window(Win, MAIN_THREAD_ID, 3, 1, "2 = Kill T2");
    wManager.write_window(Win, MAIN_THREAD_ID, 4, 1, "3 = Kill T3");
    wManager.write_window(Win, MAIN_THREAD_ID, 5, 1, "c = clear screen");
    wManager.write_window(Win, MAIN_THREAD_ID, 6, 1, "h = help screen");
    wManager.write_window(Win, MAIN_THREAD_ID, 7, 1, "s = dump scheduler");
    wManager.write_window(Win, MAIN_THREAD_ID, 8, 1, "q = Quit");
}

//--------------------------------------------------------

//--------------------------------------------------------
void *perform_simple_output(void *arguments)
{
    // extract the thread arguments:   (method 1)
    // cast arguments in to thread_data
    thread_data *td = (thread_data *)arguments;

    int thread_no = td->thread_no;
    int sleep_time = td->sleep_time;
    WINDOW *Win = td->thread_win;
    bool kill_signal = td->kill_signal;

    int CPU_Quantum = 0;
    char buff[256];

    while (!td->kill_signal)
    {
        wManager.write_window(Win, MAIN_THREAD_ID, " Task-" + std::to_string(thread_no) + " running #" + std::to_string(CPU_Quantum++) + "\n");
        sleep(thread_no * 2);
    }
    td->thread_state = DEAD;
    wManager.write_window(Win, MAIN_THREAD_ID, " TERMINATED");
    return (NULL);
}

//--------------------------------------------------------
void *perform_io_work(void *arguments)
{
    // cast arguments in to thread_data
    thread_data *td = (thread_data *)arguments;

    int thread_no = td->thread_no;
    int sleep_time = td->sleep_time;
    WINDOW *Win = td->thread_win;
    bool kill_signal = td->kill_signal;

    // do some I/O, notice that we may get an interrupt during
    // one or more of these I/O operations!

    wManager.write_window(Win, MAIN_THREAD_ID, " T-" + std::to_string(thread_no) + " Started\n");

    wManager.write_window(Win, MAIN_THREAD_ID, " Sleeping for " + std::to_string(sleep_time) + " seconds\n");
    sleep(sleep_time);

    wManager.write_window(Win, MAIN_THREAD_ID, " T-" + std::to_string(thread_no) + " Results = " + std::to_string(td->thread_results) + "\n");

    wManager.write_window(Win, MAIN_THREAD_ID, " T-" + std::to_string(thread_no) + " Finished its work\n");

    td->thread_state = DEAD;
    wManager.write_window(Win, MAIN_THREAD_ID, " TERMINATED");

    return (NULL);
}

//--------------------------------------------------------
void *perform_cpu_work(void *arguments)
{
    // cast arguments in to thread_data
    thread_data *td = (thread_data *)arguments;

    int thread_no = td->thread_no;
    int sleep_time = td->sleep_time;
    WINDOW *Win = td->thread_win;

    char buff[256];

    for (int i = 0; i < 10; i++)
    {
        if (td->kill_signal == true)
            break;
        srand(time(NULL)); // init random seed
        td->thread_results += i * (rand() % 10);
        sprintf(buff, " T-%d Result=%d\n", thread_no, td->thread_results);
        wManager.write_window(Win, MAIN_THREAD_ID, buff);
        sleep(thread_no * 2);
    }

    td->thread_state = DEAD;
    wManager.write_window(Win, MAIN_THREAD_ID, " TERMINATED");
    return (NULL);
}

int main()
{
    Scheduler scheduler;

    //------------------------------------------------------------------------------
    // Create the Thread variables    
    pthread_t thread_1, thread_2, thread_3; // Not sure if we will use these with scheduler

    // Create the Thread_data
    thread_data thread_args_1, thread_args_2, thread_args_3;

    int result_code;

    MEVENT event;                      // Object to handle mouse events
    mousemask(ALL_MOUSE_EVENTS, NULL); // Listen for all mouse events

    //----------------------Heading Window------------------------------------------
    //
    // Create a window to display thread data in
    // Create a new window: WINDOW * win = newwin(nlines, ncols, y0, x0);

    WINDOW *Heading_Win = wManager.create_window(MAIN_THREAD_ID, 12, 80, 3, 2);
    // box(Heading_Win, 0, 0);
    wManager.write_window(Heading_Win, MAIN_THREAD_ID, 2, 28, "ULTIMA 2.0 (Spring 2026)");

    wManager.write_window(Heading_Win, MAIN_THREAD_ID, 4, 2, "Starting ULTIMA 2.0.....");
    wManager.write_window(Heading_Win, MAIN_THREAD_ID, 5, 2, "Starting Thread 1....");
    wManager.write_window(Heading_Win, MAIN_THREAD_ID, 6, 2, "Starting Thread 2....");
    wManager.write_window(Heading_Win, MAIN_THREAD_ID, 7, 2, "Starting Thread 3....");
    wManager.write_window(Heading_Win, MAIN_THREAD_ID, 9, 2, "Press 'q' or Ctrl-C to exit the program...");

    //-----------------------Console Window-----------------------------------------
    //
    WINDOW *Console_Win = wManager.create_window(MAIN_THREAD_ID, 10, 20, 30, 62);
    wManager.write_window(Console_Win, MAIN_THREAD_ID, 1, 1, "....Console....\n");
    wManager.write_window(Console_Win, MAIN_THREAD_ID, 2, 1, "Ultima # ");

    //-----------------------Thread_1 Window----------------------------------------
    // set the thread_args
    thread_args_1.thread_no = 1;
    thread_args_1.thread_state = RUNNING;
    thread_args_1.thread_win = wManager.create_window(MAIN_THREAD_ID, 15, 25, 15, 2);
    wManager.write_window(thread_args_1.thread_win, MAIN_THREAD_ID, 6, 1, "Starting Thread 1.....\n");
    thread_args_1.sleep_time = 1 + rand() % 3;
    thread_args_1.kill_signal = false;
    thread_args_1.thread_results = 0;

    // Create the new thread to do CPU bound or IO bound stuff.
    scheduler.create_task("Task1", perform_simple_output, &thread_args_1);
    wManager.log(" Thread 1 Created.\n");

    //-----------------------Thread_2 Window----------------------------------------
    // set the thread_args
    thread_args_2.thread_no = 2;
    thread_args_2.thread_state = RUNNING;
    thread_args_2.thread_win = wManager.create_window(MAIN_THREAD_ID, 15, 25, 15, 30);
    wManager.write_window(thread_args_2.thread_win, MAIN_THREAD_ID, 6, 1, "Starting Thread 2.....\n");
    thread_args_2.sleep_time = 1 + rand() % 3;
    thread_args_2.kill_signal = false;
    thread_args_2.thread_results = 0;

    // Create the new thread to do CPU bound or IO bound stuff.
    scheduler.create_task("Task2", perform_cpu_work, &thread_args_2);
    wManager.log(" Thread 2 Created.\n");

    //-----------------------Thread_3 Window----------------------------------------
    // set the thread_args
    thread_args_3.thread_no = 3;
    thread_args_3.thread_state = RUNNING;
    thread_args_3.thread_win = wManager.create_window(MAIN_THREAD_ID, 15, 25, 15, 57);
    wManager.write_window(thread_args_3.thread_win, MAIN_THREAD_ID, 6, 1, "Starting Thread 3.....\n");
    thread_args_3.sleep_time = 1 + rand() % 3;
    thread_args_3.kill_signal = false;
    thread_args_3.thread_results = 0;

    // Create the new thread to do CPU bound or IO bound stuff.
    scheduler.create_task("Task3", perform_cpu_work, &thread_args_3);
    wManager.log(" Thread 3 Created.\n");

    //------------------------------------------------------------------------------
    wManager.log(" All threads have been created...\n");

    //------------------------------------------------------------------------------
    // Set up keyboard I/O processing

    cbreak();                   // Disable line buffering
    noecho();                   // Disable auto echo of reading by getch(), wgetch()
    nodelay(Console_Win, TRUE); // causes getch to be a non-blocking call.

    keypad(Console_Win, TRUE);

    char buff[256];
    int input = -1;
    int CPU_Quantum = 1;

    //------------------------------------------------------------------------------
    while (input != 'q')
    {
        input = wgetch(Console_Win);

        switch (input)
        {
        case '1':
        case '2':
        case '3':
            if (input == '1')
            {
                thread_args_1.kill_signal = true;
            }
            else if (input == '2')
            {
                thread_args_2.kill_signal = true;
            }
            else if (input == '3')
            {
                thread_args_3.kill_signal = true;
            }

            sprintf(buff, " %c\n", input);
            wManager.write_window(Console_Win, MAIN_THREAD_ID, buff);
            sprintf(buff, " Kill = %c\n", input);
            wManager.write_window(Console_Win, MAIN_THREAD_ID, buff);
            wManager.log(buff);

            sleep(2);
            wManager.clear_window(Console_Win, MAIN_THREAD_ID);
            wManager.write_window(Console_Win, MAIN_THREAD_ID, 1, 1, "Ultima # ");
            break;
        case 'c':
            // Clear the console window
            refresh();
            wManager.clear_window(Console_Win, MAIN_THREAD_ID);
            wManager.write_window(Console_Win, MAIN_THREAD_ID, 1, 1, "Ultima # ");
            break;
        case 'h':
            display_help(Console_Win);
            wManager.write_window(Console_Win, MAIN_THREAD_ID, 8, 1, "Ultima # ");
            break;
        case 'q':
            // End the loop, and end the program.
            wManager.log(" Quitting the main program....\n");
            wManager.log(" Signal the remaining child processes to stop as well.\n");

            thread_args_1.kill_signal = true;
            thread_args_2.kill_signal = true;
            thread_args_3.kill_signal = true;
            break;
        case 's':
            // scheduler.dump(Log_Win);
            break;
        case ERR:
            break;
        case KEY_MOUSE:
            // Check and report mouse event
            // if (getmouse(&event) == OK)
            // {
            //     sprintf(buff, " Mouse event at x=%d, y=%d\n", event.x, event.y);
            //     wManager.log(buff);
            // }
            break;
        default:

            sprintf(buff, " %c\n", input);
            wManager.write_window(Console_Win, MAIN_THREAD_ID, buff);
            wManager.write_window(Console_Win, MAIN_THREAD_ID, " -Invalid Command\n");
            wManager.log(buff);
            wManager.log(" -Invalid Command\n");
            wManager.write_window(Console_Win, MAIN_THREAD_ID, "Ultima # ");

            break;
        }

        CPU_Quantum++;
    }

    wManager.log(" Waiting for living threads to complete...\n"); // Since scheduler is creating them, trying to join these null threads crashes the program
    // result_code = pthread_join(thread_1, NULL);         
    // result_code = pthread_join(thread_2, NULL);
    // result_code = pthread_join(thread_3, NULL);

    wManager.log(" All threads have now ended.....\n");
    wManager.log(" .........Main program ended........\n");

    wManager.log(" Thread 1 State = " + thread_args_1.thread_state + "\n");
    wManager.log(" Thread 2 State = " + thread_args_2.thread_state + "\n");
    wManager.log(" Thread 3 State = " + thread_args_3.thread_state + "\n");

    sleep(5);
    // getch();
    endwin(); // End the curses window
    return (0);
}