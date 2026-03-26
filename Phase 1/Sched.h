/*
Sched.h

Header file to contain the TCB struct, Scheduler class, constants, and function signatures for the Sched.cpp file.

Colin Christy
March 26, 2026
Last update: March 26, 2026
*/

#include <string>

struct TCB {
    int task_id;
    std::string state;
    clock_t start_time;
    TCB *next;
};

class Scheduler {
    TCB *process_table;

public:
    int create_task() {
        return 1; // Will return the task id after creation
    };
    bool kill_task() {
        return false; // Returns if the operation was successful
    }
    void yield() {

    };
    void garbage_collect() {

    };

    void dump(int level) {
        printf("Dumping task table...");
    };
};