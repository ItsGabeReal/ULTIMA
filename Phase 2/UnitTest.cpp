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
#include "Sema.h"
#include "Sched.h"

// Forward Declarations
void* worker(void* arg);
void simulate_work(int amount);

/**
 * Data Structure for each thread.
 * Note that each thread has access to its own WINDOW and should
 * be able to display its output to its private window.
 */
struct thread_data
{
    int thread_no;
    std::string thread_state;
    bool kill_signal;
    int sleep_time;
    int thread_results;
    int task_id; // ID assigned to this task once created
};

Scheduler scheduler;
Semaphore sem("Test resource", 1);

int main()
{
    // Reduce the amount of work each task can do at a time (this may need to be
    // fine-tuned depending on the machine it's run on).
    scheduler.set_quantum(10000);

    std::cout << "Starting unit test for Semaphore and Scheduler\n\n";

    // Create multiple tasks, and add them to the scheduler
    const int NUM_THREADS = 3;
    thread_data threads[NUM_THREADS];
    int id;
    std::string name;
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threads[i].kill_signal = false;
        threads[i].thread_no = i+1;
        threads[i].thread_state = READY;
        threads[i].sleep_time = 1 + rand() % 3;
        threads[i].thread_results = 0;

        name = ("Task "+std::to_string(i+1)).c_str();
        threads[i].task_id = scheduler.create_task(name, worker, &threads[i]);
    }

    // Start scheduler
    scheduler.start();

    // Pause until all threads have completed
    scheduler.wait_for_all_threads();

    std::cout << "Unit test completed \n";
    return 0;
}

void* worker(void* arg)
{
    const int WORK_AMOUNT = 10;
    int work_done = 0;
    thread_data *td = (thread_data *)arg;

    // Wait until thread is running
    while (scheduler.get_state(td->task_id) != RUNNING)
    {
        // std::cout << "TaskID=" << td->task_id << ": Waiting to start\n";
        sleep(1);
    }
    
    std::cout << " -------------------- Task " << td->thread_no << " Started --------------------\n";
    
    // Try to claim resource
    sem.down(td->task_id);


    // Do work while yielding to the scheduler
    while (work_done < WORK_AMOUNT)
    {
        // Simulate doing work
        std::cout << "TaskID=" << td->task_id << ": Doing work (" << (float(work_done)/WORK_AMOUNT)*100 << "%)" << std::endl;
        simulate_work(1000000);
        ++work_done;
        
        // Let the scheduler decide if we should pause or not
        scheduler.yield();
        while (scheduler.get_state(td->task_id) != RUNNING)
            sleep(1);
    }

    // Release resource
    sem.up(td->task_id);

    std::cout << " -------------------- Task " << td->thread_no << " Finished --------------------\n\n";

    scheduler.set_state(td->task_id, DEAD);
    return nullptr;
}

/**
 * Wastes CPU time.
 */
void simulate_work(int amount)
{
    int counter = 0;
    for (int i = 0; i < amount; ++i)
    {
        ++counter;
    }
    usleep(100000);
}
