/**
 * Sched.h
 *
 * Basic scheduler class that handles managing tasks.
 *
 * @author Colin Christy
 * @date 3/28/2026
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <sstream>
#include <format>
#include <iostream>
#include <iomanip>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include "Log.h"

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

const std::string READY = "READY";
const std::string RUNNING = "RUNNING";
const std::string BLOCKED = "BLOCKED";
const std::string DEAD = "DEAD";

const int MAX_TASKS = 3;

struct TCB
{
    int task_id;
    std::string task_name;
    std::string state;
    clock_t start_time;
    pthread_t thread;
    TCB *next;
};

/**
 * Basic scheduler class that handles managing tasks.
 */
class Scheduler
{
private:
    TCB *process_table; // Pointer to the head of the process table linked list
    int current_task;
    long current_quantum;
    int next_available_id;
    pthread_mutex_t lock; // Prevent multiple threads from making changes simultaneously

public:
    /**
     * Default constructor
     */
    Scheduler();

    /**
     * Destructor
     */
    ~Scheduler();

    /**
     * Creates a task and creates a TCB in the process table. Returns the task id.
     * 
     * @param task_name Name of the task being created.
     * @param task_function Function that the task will execute.
     * @param args Additional arguments for the task.
     * @returns Thread ID for the newly created task
     */
    int create_task(std::string task_name, void *(*task_function)(void *), void *args);

    /**
     * Initiates the scheduler and starts the first task.
     */
    void start();

    /**
     * Gets the TCB pointer of the task by id if it exists.
     */
    TCB *get_tcb_pointer(int task_id);

    /**
     * Sets the state of the specified task to the given state.
     * 
     * @param task_id Id of the task to change state.
     * @param new_state State the task will change to.
     */
    void set_state(int task_id, std::string new_state);

    /**
     * Gets the state of the specified task.
     * 
     * @param task_id Id of the specified task.
     */
    std::string get_state(int task_id);

    /**
     * Gets the current task id for the Scheduler.
     */
    int get_task_id();

    /**
     * Kills the specified task.
     * 
     * @param task_id Id of the task to be killed.
     */
    void kill_task(int task_id);

    /**
     * Attempts to give the scheduler back control from the task. The scheduler could
     * decide to run a different task or continue to run the same task.
     */
    void yield();

    /**
     * Cleans up dead tasks from the process table that do not need to be tracked any longer.
     */
    void garbage_collect();

    /**
     * Halts the calling thread until all threads in the process table have
     * completed.
     */
    void wait_for_all_threads();

    /**
     * Prints the process table with various info about the current state of all tasks.
     * Use the level parameter to change how much detail is given.
     * 
     * @param level Amount of detail shown (Default: 2)
     */
    std::string dump(int level = 2);

private:
    /**
     * Looks for another READY task to switch to. If a task is found, the current
     * task will be changed to READY and the next task will become RUNNING.
     * 
     * @returns True if the current task was successfully switched.
     */
    bool try_switch_task();
};
#endif