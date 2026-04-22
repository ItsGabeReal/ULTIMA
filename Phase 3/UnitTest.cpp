/**
 * UnitTest.cpp
 * 
 * For testing the functionality of individual components. Run `make test` to
 * compile/execute this file.
 * 
 * Note: To enable debug messages, set DEBUG to 1 in Log.h
 */

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cmath>
#include "Sema.h"
#include "Sched.h"
#include "IPC.h"
#include "MMU.h"

int total_tests = 0, good_tests = 0;

/**
 * Prints the test's name with either a checkmark or cross, depending on the
 * value of the condition.
 */
#define TEST(name, cond) { \
    if (cond) { \
        std::cout << "\033[32m\u2713 " << (name) << "\033[0m\n"; \
        ++good_tests; \
    } else { \
        std::cout << "\033[31m\u2717 " << (name) \
                  << " (" << __FILE__ << ":" << __LINE__ << ")\033[0m\n"; \
    } \
    ++total_tests; \
} 
/**
 * Indicates a successful test if the statement throws an exception.
 */
#define THROWS(name, statement) { \
    bool threw = false; \
    try { statement; } catch (...) { threw = true; } \
    TEST((name), threw); \
}

void* worker(void*);

int main()
{
    Scheduler scheduler;
    IPC ipc(3);
    Semaphore::set_scheduler_ptr(&scheduler); // Initialize scheduler pointer for Semaphore class
    // MMU* mmu;
    int mem_handle, res;
    char ch;
    std::string str;

    /**
     * Setup
     */
    int task1 = scheduler.create_task("Task 1", worker, nullptr);
    int task2 = scheduler.create_task("Task 2", worker, nullptr);
    int task3 = scheduler.create_task("Task 3", worker, nullptr);


    /**
     * Constructor
     */
    std::cout << "---------- Constructor Tests ----------" << std::endl;

    THROWS("Throws exception when page_size > size",
        MMU test(32, '.', 64));

    THROWS("Throws exception when page_size doesn't evenly divide size",
        MMU test(32, '.', 15));
    
    MMU mmu(32, '.', 8);
    TEST("MMU initializes properly / core_dump() and mem_dump() work properly",
        mmu.core_dump() == "................................"
        && mmu.mem_dump() ==
        " Status\tHandle\tStart\tEnd\tSize\tCurrent\tTask-ID\n"
        " Free\t0\t0\t31\t32\tNA\tMMU");

    /**
     * mem_alloc()
     */
    std::cout << "\n---------- mem_alloc() Tests ----------" << std::endl;

    mmu.mem_alloc(7, task1);
    TEST("Single page allocated", mmu.mem_dump() ==
        " Status\tHandle\tStart\tEnd\tSize\tCurrent\tTask-ID\n"
        " Used\t0\t0\t7\t8\t0\t1\n"
        " Free\t8\t8\t31\t24\tNA\tMMU");
    
    mem_handle = mmu.mem_alloc(9, task2);
    TEST("Multiple pages allocated", mmu.mem_dump() ==
        " Status\tHandle\tStart\tEnd\tSize\tCurrent\tTask-ID\n"
        " Used\t0\t0\t7\t8\t0\t1\n"
        " Used\t8\t8\t23\t16\t0\t2\n"
        " Free\t24\t24\t31\t8\tNA\tMMU");

    // With the first 8 bytes already by task1, task2's handle should be 8
    TEST("Expected memory handle returned", mem_handle == 8);

    mem_handle = mmu.mem_alloc(7, task2);
    TEST("Do not allocate if task already has memory allocated",
        mem_handle == -1);

    mem_handle = mmu.mem_alloc(16, task3);
    TEST("Do not allocate if there is insufficient space", mem_handle == -1);


    /**
     * mem_write()
     */
    std::cout << "\n---------- mem_write() Tests ----------" << std::endl;
    
    std::cout << "Single character" << std::endl;

    res = mmu.mem_write(8, 'H', task2);
    TEST("Write single character",
        mmu.core_dump() == "........H.......................");

    res = mmu.mem_write(8, 'i', task2);
    TEST("Characters write sequentially",
        mmu.core_dump() == "........Hi......................");

    for (int i = 2; i < 16; ++i)
        mmu.mem_write(8, '*', task2); // Fill remaining memory with '*'
    res = mmu.mem_write(8, '!', task2); // Try (and fail) to write next character
    TEST("End of memory reached, and current_location gets reset to beginning",
        mmu.core_dump() == "........Hi**************........"
        && res == -1
        && scheduler.get_tcb_pointer(task2)->current_location == 0);

    res = mmu.mem_write(1000, '!', task2);
    TEST("Return -1 when memory handle is invalid", res == -1);

    res = mmu.mem_write(8, '!', task3);
    TEST("Return -1 when task does not own memory handle", res == -1);

    std::cout << "Multiple characters" << std::endl;

    mmu.mem_write(0, 1, "Hello", task1);
    TEST("Write multiple memory values at once", mmu.core_dump() ==
        ".Hello..Hi**************........");

    res = mmu.mem_write(0, 1, "********", task1); // Try to write 1 char beyond the allocated size
    TEST("Return -1 when memory range is invalid", res == -1);

    res = mmu.mem_write(1000, 1, "*****", task1);
    TEST("Return -1 when memory handle is invalid)", res == -1);

    res = mmu.mem_write(0, 1, "*****", task2);
    TEST("Return -1 when task does not own memory handle)", res == -1);


    /**
     * mem_free()
     */
    std::cout << "\n---------- mem_free() Tests ----------" << std::endl;
    
    res = mmu.mem_free(1000, task1);
    TEST("Return -1 if memory handle is out of bounds", res == -1);

    res = mmu.mem_free(0, task3);
    TEST("Return -1 if task does not own memory handle", res == -1);

    mmu.mem_free(0, task1);
    TEST("Free single block of memory", mmu.mem_dump() ==
        " Status\tHandle\tStart\tEnd\tSize\tCurrent\tTask-ID\n"
        " Free\t0\t0\t7\t8\tNA\tMMU\n"
        " Used\t8\t8\t23\t16\t0\t2\n"
        " Free\t24\t24\t31\t8\tNA\tMMU"
        && mmu.core_dump() == "........Hi**************........");

    mmu.mem_free(8, task2);
    TEST("Free multiple blocks of memory & coalesce empty space", mmu.mem_dump() ==
        " Status\tHandle\tStart\tEnd\tSize\tCurrent\tTask-ID\n"
        " Free\t0\t0\t31\t32\tNA\tMMU"
        && mmu.core_dump() == "................................");

    mmu.mem_alloc(8, task1);
    TEST("Task acquires new memory after freeing", mmu.mem_dump() ==
        " Status\tHandle\tStart\tEnd\tSize\tCurrent\tTask-ID\n"
        " Used\t0\t0\t7\t8\t0\t1\n"
        " Free\t8\t8\t31\t24\tNA\tMMU");
    

    /**
     * mem_read()
     */
    std::cout << "\n---------- mem_free() Tests ----------" << std::endl;

    std::cout << "Single character" << std::endl;

    mmu.mem_alloc(8, task2); // Test memory somewhere in the middle of memory space
    mmu.mem_write(8, 0, "shrimped", task2); // Fill memory with 8-letter word
    mmu.mem_read(8, &ch, task2);
    TEST("Character is read", ch == 's');
    
    mmu.mem_read(8, &ch, task2);
    TEST("Characters read sequentially", ch == 'h');

    for (int i = 2; i < 8; ++i)
        mmu.mem_read(8, &ch, task2); // Read remaining characters in memory
    res = mmu.mem_read(8, &ch, task2); // Try (and fail) to read next character
    TEST("End of memory reached, and current_location gets reset to beginning",
        res == -1
        && scheduler.get_tcb_pointer(task2)->current_location == 0);
    
    res = mmu.mem_read(1000, &ch, task2);
    TEST("Return -1 when memory handle is invalid", res == -1);

    res = mmu.mem_read(8, &ch, task1);
    TEST("Return -1 when task does not own memory handle", res == -1);

    std::cout << "Multiple characters" << std::endl;

    mmu.mem_read(8, 2, 4, &str, task2);
    TEST("Read multiple characters at once", str == "rimp");
    
    res = mmu.mem_read(8, 2, 8, &str, task2);
    TEST("Return -1 when range is invalid", res == -1);

    res = mmu.mem_read(1000, 2, 4, &str, task2);
    TEST("Return -1 when memory handle is invalid", res == -1);

    res = mmu.mem_read(8, 2, 4, &str, task1);
    TEST("Return -1 when task does not own memory handle", res == -1);


    /**
     * Recap
     */
    std::cout << "\nTests finished: " << good_tests << "/" << total_tests << " passed." << std::endl;

    
    return 0;
}

// Placeholder thread function
void* worker(void* args) { return nullptr; }
