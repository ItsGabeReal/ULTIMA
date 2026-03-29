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
#include <format>
#include <iostream>
#include <iomanip>
#include <pthread.h>
#include "WindowManager.h"
#include <unistd.h>
#include <assert.h>

extern WindowManager wManager; // Using global wManager from Ultima.cpp

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
    TCB *process_table;
    int current_task;
    long current_quantum;
    int next_available_id;

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
     */
    int create_task(std::string task_name, void *(*task_function)(void *), void *args);

    /**
     * Gets the TCB pointer of the task by id if it exists.
     */
    TCB *get_tcb_pointer(int task_id);

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
     * Prints the process table with various info about the current state of all tasks.
     * Use the level parameter to change how much detail is given.
     * 
     * @param Win Window to output the dump
     * @param level Amount of detail shown (Default: 2)
     */
    void dump(WINDOW *Win, int level = 2);
};
#endif