/*
Sched.h

Header file to contain the TCB struct, Scheduler class, constants, and function signatures for the Sched.cpp file.

Colin Christy
March 26, 2026
Last update: March 26, 2026
*/

#include <string>
#include <iostream>
#include <iomanip>

const std::string READY     = "READY";
const std::string RUNNING   = "RUNNING";
const std::string BLOCKED   = "BLOCKED";
const std::string DEAD      = "DEAD";

const int MAX_TASKS         = 100;

struct TCB {
    int task_id;
    std::string task_name;
    std::string state;
    clock_t start_time;
    TCB *next;
};

class Scheduler {
    TCB *process_table;

    int current_task;
    long current_quantum;
    int next_available_task_id;

public:
    Scheduler() {
        process_table           = nullptr;  // No processes yet
        current_task            = -1;       // No task is running
        next_available_task_id  = 0;        // Start ids at 0
        current_quantum         = 500;      // Set quantum length to 500ms
    }
    ~Scheduler() {
        std::cout << "Exiting Scheduler....." << std::endl;
    }
    int create_task(std::string task_name) {
        TCB *new_task = new TCB();

        new_task->task_id = next_available_task_id++;
        new_task->task_name = task_name;
        new_task->state = READY;
        new_task->start_time = clock();
        new_task->next = process_table;
        process_table = new_task;

        return new_task->task_id;
    };
    bool kill_task() {
        return false; // Returns if the operation was successful
    }
    void yield() {

    };
    void garbage_collect() {

    };

    void dump(int level) {
        std::cout << "-----------Process Table-----------" << std::endl;

        TCB *current_task = process_table;
        int count = 0;
        while (current_task != nullptr) {
            std::cout << std::setw(7) << current_task->task_name 
            << std::setw(7) << current_task->task_id
            << std::setw(7) << current_task->state
            << std::setw(7) << current_task->start_time
            << std::endl;

            current_task = current_task->next;
            count++;
        }

        std::cout << "-----------------------------------" << std::endl;
    };
};