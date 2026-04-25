/**
 * Ultima.cpp
 * 
 * Entry-pont for the ULTIMA operating system. Use `make run` to execute it.
 * 
 * Note: To disable debug messages, set DEBUG to 0 in Log.h
 */

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
#include "IPC.h"
#include "MMU.h"

using namespace std;

#define MAIN_TID 0 // Thread ID of the main thread

/**
 * Data Structure for each thread.
 * Note that each thread has access to its own WINDOW and should
 * be able to display its output to its private window.
 */
struct thread_data
{
    int thread_no;
    WINDOW *thread_win;
    int sleep_time;
    int memory_needed;
    int thread_results;
    int task_id; // ID assigned to this task once created
};

//--------------------------------------------------------
// Forward declaration
//--------------------------------------------------------
void display_help(WINDOW *Win);
WINDOW* create_heading_window();
void create_tasks(thread_data &args_1, thread_data &args_2, thread_data &args_3);
void *perform_simple_output(void *arguments);
void *perform_cpu_work(void *arguments);
void *perform_io_work(void *arguments);
void simulate_work(int amount);
//--------------------------------------------------------

// Initialize scheduler and window manager as global variables (we might change 
// this later)
Scheduler scheduler;
WindowManager wManager(MAIN_TID, &scheduler);
IPC ipc(MAX_TASKS);
MMU mmu(1024, '.', 64);
bool tasks_paused = false;

int main()
{
    Semaphore::set_scheduler_ptr(&scheduler); // Initialize scheduler pointer for Semaphore class

    thread_data thread_args_1, thread_args_2, thread_args_3;
    int result_code;
    MEVENT event; // Object to handle mouse events

    //----------------------- Create Windows ------------------------
    
    // Heading
    WINDOW* Heading_Win = create_heading_window();

    // Console
    WINDOW *Console_Win = wManager.create_window(MAIN_TID, 14, 20, 30, 62);
    wManager.write_window(Console_Win, MAIN_TID, 1, 1, "....Console....\n");
    wManager.write_window(Console_Win, MAIN_TID, 2, 1, "Ultima # ");

    // Dump
    WINDOW* Dump_Win = wManager.create_window(MAIN_TID, 41, 66, 3, 82);
    wManager.write_window(Dump_Win, MAIN_TID, "\n..............Dump Window............\n");
    
    // Thread 1
    thread_args_1.thread_win = wManager.create_window(MAIN_TID, 15, 25, 15, 2);

    // Thread 2
    thread_args_2.thread_win = wManager.create_window(MAIN_TID, 15, 25, 15, 30);

    // Thread 3
    thread_args_3.thread_win = wManager.create_window(MAIN_TID, 15, 25, 15, 57);

    //----------------------- Run Tasks ------------------------

    create_tasks(thread_args_1, thread_args_2, thread_args_3);
    wManager.log(" All threads have been created.\n");

    wManager.log(" Starting scheduler...\n");
    scheduler.start();

    //----------------------- Set up I/O processing ------------------------

    mousemask(ALL_MOUSE_EVENTS, NULL); // Listen for all mouse events
    cbreak();                   // Disable line buffering
    noecho();                   // Disable auto echo of reading by getch(), wgetch()
    nodelay(Console_Win, TRUE); // causes getch to be a non-blocking call.
    keypad(Console_Win, TRUE);

    char buff[256];
    int input = -1;
    int CPU_Quantum = 1;

    //----------------------- Run Input Loop ------------------------
    while (input != 'q')
    {
        input = wgetch(Console_Win);

        switch (input)
        {
        case '1':
        case '2':
        case '3':
            // Kill a certain task
            if (input == '1')
                scheduler.kill_task(thread_args_1.task_id);
            else if (input == '2')
                scheduler.kill_task(thread_args_2.task_id);
            else if (input == '3')
                scheduler.kill_task(thread_args_3.task_id);

            sprintf(buff, " %c\n", input);
            wManager.write_window(Console_Win, MAIN_TID, buff);
            sprintf(buff, " Kill = %c\n", input);
            wManager.write_window(Console_Win, MAIN_TID, buff);
            wManager.log(buff);

            sleep(2);
            wManager.clear_window(Console_Win, MAIN_TID);
            wManager.write_window(Console_Win, MAIN_TID, 1, 1, "Ultima # ");
            break;
        case 'c':
            // Clear the console window
            refresh();
            wManager.clear_window(Console_Win, MAIN_TID);
            wManager.write_window(Console_Win, MAIN_TID, 1, 1, "Ultima # ");
            break;
        case 'h':
            display_help(Console_Win);
            wManager.write_window(Console_Win, MAIN_TID, " Ultima # ");
            break;
        case 'q':
            // End the loop, and end the program.
            wManager.log(" Quitting the main program....\n");
            wManager.log(" Signal the remaining child processes to stop as well.\n");

            scheduler.kill_task(thread_args_1.task_id);
            scheduler.kill_task(thread_args_2.task_id);
            scheduler.kill_task(thread_args_3.task_id);
            break;
        case 's':
            wManager.write_window(Dump_Win, MAIN_TID, scheduler.dump()+'\n');
            wManager.log(" Scheduler dumped.\n");
            break;
        case 'e':
            wManager.write_window(Dump_Win, MAIN_TID, wManager.get_window_lock().dump()+'\n');
            wManager.log(" Window semaphore dumped.\n");
            break;
        case 'i':
            wManager.write_window(Dump_Win, MAIN_TID, ipc.message_dump()+'\n');
            wManager.log(" Messages dumped.\n");
            break;
        case 'm':
            wManager.write_window(Dump_Win, MAIN_TID, mmu.mem_dump()+'\n');
            wManager.log(" Memory Table dumped.\n");
            break;
        case 'n':
            wManager.write_window(Dump_Win, MAIN_TID, mmu.core_dump()+'\n');
            wManager.log(" Memory Table dumped.\n");
            break;
        case 'p':
            tasks_paused = true;
            wManager.log(" Tasks paused.\n");
            break;
        case 'r':
            tasks_paused = false;
            wManager.log(" Tasks resumed.\n");
            break;
        case ERR:
            break;
        case KEY_MOUSE:
            // Check and report mouse event
            if (getmouse(&event) == OK)
            {
                sprintf(buff, " Mouse event at x=%d, y=%d\n", event.x, event.y);
                wManager.log(buff);
            }
            break;
        default:

            sprintf(buff, " %c\n", input);
            wManager.write_window(Console_Win, MAIN_TID, buff);
            wManager.write_window(Console_Win, MAIN_TID, " -Invalid Command\n");
            wManager.log(buff);
            wManager.log(" -Invalid Command\n");
            wManager.write_window(Console_Win, MAIN_TID, "Ultima # ");

            break;
        }

        CPU_Quantum++;
    }

    //----------------------- Shutdown Procedure ------------------------

    // Since scheduler is creating them, trying to join these null threads crashes the program
    wManager.log(" Waiting for living threads to complete...\n");
    // result_code = pthread_join(thread_1, NULL);         
    // result_code = pthread_join(thread_2, NULL);
    // result_code = pthread_join(thread_3, NULL);

    wManager.log(" All threads have now ended.....\n");
    wManager.log(" .........Main program ended........\n");

    wManager.log(" Thread 1 State = " + scheduler.get_state(thread_args_1.task_id) + "\n");
    wManager.log(" Thread 2 State = " + scheduler.get_state(thread_args_2.task_id) + "\n");
    wManager.log(" Thread 3 State = " + scheduler.get_state(thread_args_3.task_id) + "\n");

    sleep(2);
    return (0);
}

/**
 * Prints a list of available commands to the provided window.
 */
void display_help(WINDOW *Win)
{
    wManager.clear_window(Win, MAIN_TID);
    wManager.write_window(Win, MAIN_TID, "\n");
    wManager.write_window(Win, MAIN_TID, " ...Help...\n");
    wManager.write_window(Win, MAIN_TID, " 1,2,3 = Kill Task\n");
    wManager.write_window(Win, MAIN_TID, " c = clear screen\n");
    wManager.write_window(Win, MAIN_TID, " h = help screen\n");
    wManager.write_window(Win, MAIN_TID, " s = dump scheduler\n");
    wManager.write_window(Win, MAIN_TID, " e = dump semaphore\n");
    wManager.write_window(Win, MAIN_TID, " i = dump messages\n");
    wManager.write_window(Win, MAIN_TID, " m = dump mem segs\n");
    wManager.write_window(Win, MAIN_TID, " n = dump mem core\n");
    wManager.write_window(Win, MAIN_TID, " p,r = pause/resume\n");
    wManager.write_window(Win, MAIN_TID, " q = Quit\n");
}

/**
 * Creates the header window and prints initial messages to it.
 */
WINDOW* create_heading_window()
{
    WINDOW *Heading_Win = wManager.create_window(MAIN_TID, 12, 80, 3, 2);
    // box(Heading_Win, 0, 0);
    wManager.write_window(Heading_Win, MAIN_TID, 2, 28, "ULTIMA 2.0 (Spring 2026)");

    wManager.write_window(Heading_Win, MAIN_TID, 4, 2, "Starting ULTIMA 2.0.....");
    wManager.write_window(Heading_Win, MAIN_TID, 5, 2, "Starting Thread 1....");
    wManager.write_window(Heading_Win, MAIN_TID, 6, 2, "Starting Thread 2....");
    wManager.write_window(Heading_Win, MAIN_TID, 7, 2, "Starting Thread 3....");
    wManager.write_window(Heading_Win, MAIN_TID, 9, 2, "Press 'q' or Ctrl-C to exit the program...");

    return Heading_Win;
}

/**
 * Initializes thread arguments and adds a task for each thread in the
 * scheduler. Assumes the windows have already been created.
 */
void create_tasks(thread_data &args_1, thread_data &args_2, thread_data &args_3)
{
    int id;

    //----------------------- Thread 1 -----------------------
    args_1.thread_no = 1;
    args_1.sleep_time = 1 + rand() % 3;
    args_1.thread_results = 0;
    args_1.memory_needed = 16 + rand() % 333;

    id = scheduler.create_task("Task1", perform_simple_output, &args_1);
    args_1.task_id = id;

    wManager.log(" Thread 1 Created.\n");

    //----------------------- Thread 2 -----------------------
    args_2.thread_no = 2;
    args_2.sleep_time = 1 + rand() % 3;
    args_2.thread_results = 0;
    args_2.memory_needed = 16 + rand() % 333;

    id = scheduler.create_task("Task2", perform_simple_output, &args_2);
    args_2.task_id = id;

    wManager.log(" Thread 2 Created.\n");

    //----------------------- Thread 3 -----------------------
    args_3.thread_no = 3;
    args_3.sleep_time = 1 + rand() % 3;
    args_3.thread_results = 0;
    args_3.memory_needed = 16 + rand() % 333;

    id = scheduler.create_task("Task3", perform_simple_output, &args_3);
    args_3.task_id = id;

    wManager.log(" Thread 3 Created.\n");
}

/**
 * A basic task that prints a message every few seconds.
 */
void *perform_simple_output(void *arguments)
{
    // extract the thread arguments:   (method 1)
    // cast arguments in to thread_data
    thread_data *td = (thread_data *)arguments;
    int task_id = td->task_id;
    int thread_no = td->thread_no;
    int sleep_time = td->sleep_time;
    WINDOW *Win = td->thread_win;
    Message incoming_message;
    int memory_handle = -1;
    int CPU_Quantum = 0;
    char buff[256];

    // Wait until task is ready to run
    while (scheduler.get_state(td->task_id) != RUNNING)
        usleep(10000); // Wait 10ms

    // Startup message
    wManager.write_window(Win, task_id, 6, 1, "Starting Thread "+std::to_string(task_id)+"...\n");
    wManager.write_window(Win, task_id, " Asking for "+std::to_string(td->memory_needed)+"B of memory...\n");

    while (memory_handle == -1)
    {
        memory_handle = mmu.mem_alloc(td->memory_needed, task_id);

        if (memory_handle > -1)
        {
            wManager.write_window(Win, task_id, " Received mem at " + std::to_string(memory_handle) + "\n");
            break;
        }
        
        wManager.write_window(Win, td->task_id, " Unable to allocate\n");

        // Let the scheduler decide if this task should pause or not
        scheduler.yield();
        sleep(2);
        while (scheduler.get_state(td->task_id) != RUNNING || tasks_paused)
            usleep(10000); // Wait 10ms
        
        wManager.write_window(Win, td->task_id, " Retrying...\n");
    }

    // Label memory segment
    mmu.mem_write(memory_handle, 0, "Task-" + std::to_string(td->task_id) + " memory ", td->task_id);


    while (scheduler.get_state(td->task_id) != DEAD)
    {
        // Process message queue
        int messages = ipc.message_receive(td->task_id, incoming_message);
        while (messages > 0)
        {
            wManager.write_window(Win, task_id,
                " From T"+std::to_string(incoming_message.source_task_id)+": "+incoming_message.text+"\n");

            messages = ipc.message_receive(td->task_id, incoming_message);
        }

        // Do some work
        wManager.write_window(Win, thread_no, " Task-" + std::to_string(thread_no) + " running #" + std::to_string(CPU_Quantum++) + "\n");
        simulate_work(500'000);

        // Write to memory
        wManager.write_window(Win, thread_no, " Writing to memory\n");
        int result = mmu.mem_write(memory_handle, 64 + ((CPU_Quantum - 1) * 34), "<Using memory for important stuff>", task_id);

        // Check if write failed (ran out of room in memory)
        if (result == -1)
        {
            wManager.write_window(Win, thread_no, " Failed! Ending task..\n");
            mmu.mem_free(memory_handle, task_id);
            wManager.write_window(Win, thread_no, " Freed memory at " + std::to_string(memory_handle) + "\n");
            sleep(2);
            break;
        }

        // Send message
        int recipient = ((td->task_id + 1) % MAX_TASKS) + 1;
        ipc.message_send(td->task_id, recipient, "#"+std::to_string(CPU_Quantum-1)+" completed", Message_Type(2));

        // Let the scheduler decide if this task should pause or not
        scheduler.yield();
        while (scheduler.get_state(td->task_id) != RUNNING || tasks_paused)
            usleep(10000); // Wait 10ms
    }

    // End the task
    scheduler.kill_task(td->task_id);
    wManager.write_window(Win, thread_no, " TERMINATED");
    return (NULL);
}

/**
 * 
 */
void *perform_io_work(void *arguments)
{
    // cast arguments in to thread_data
    thread_data *td = (thread_data *)arguments;

    int thread_no = td->thread_no;
    int sleep_time = td->sleep_time;
    WINDOW *Win = td->thread_win;

    // do some I/O, notice that we may get an interrupt during
    // one or more of these I/O operations!

    wManager.write_window(Win, thread_no, " T-" + std::to_string(thread_no) + " Started\n");

    wManager.write_window(Win, thread_no, " Sleeping for " + std::to_string(sleep_time) + " seconds\n");
    sleep(sleep_time);

    wManager.write_window(Win, thread_no, " T-" + std::to_string(thread_no) + " Results = " + std::to_string(td->thread_results) + "\n");

    wManager.write_window(Win, thread_no, " T-" + std::to_string(thread_no) + " Finished its work\n");

    wManager.write_window(Win, thread_no, " TERMINATED");

    return (NULL);
}

/**
 * 
 */
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
        if (scheduler.get_state(td->task_id) == DEAD)
            break;
        srand(time(NULL)); // init random seed
        td->thread_results += i * (rand() % 10);
        sprintf(buff, " T-%d Result=%d\n", thread_no, td->thread_results);
        wManager.write_window(Win, thread_no, buff);
        sleep(thread_no * 2);
    }

    scheduler.kill_task(td->task_id);
    wManager.write_window(Win, thread_no, " TERMINATED");
    return (NULL);
}

/**
 * Wastes time.
 */
void simulate_work(int amount)
{
    usleep(amount);
}
