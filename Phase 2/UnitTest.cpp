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
#include "IPC.h"

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
IPC ipc(3);

int main()
{
    Semaphore::set_scheduler_ptr(&scheduler); // Initialize scheduler pointer for Semaphore class

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

    std::cout << ipc.message_dump() << std::endl;
    
    ipc.message_delete_all(2);
    std::cout << "Deleted all messages in Task2's mailbox. After:" << std::endl;
    
    std::cout << ipc.message_dump() << std::endl;

    std::cout << "Unit test completed \n";
    return 0;
}

void* worker(void* arg)
{
    const int WORK_AMOUNT = 10;
    int work_done = 0;
    thread_data *td = (thread_data *)arg;
    Message* messageStorage = new Message();

    // Wait until thread is running
    while (scheduler.get_state(td->task_id) != RUNNING)
        usleep(10000);
    
    std::cout << " -------------------- Task " << td->thread_no << " Started --------------------\n";
    
    // Try to claim resource
    sem.down(td->task_id);


    // Do work while yielding to the scheduler
    while (work_done < WORK_AMOUNT)
    {
        // Simulate doing work
        std::cout << "TaskID=" << td->task_id << ": Doing work (" << (float(work_done)/WORK_AMOUNT)*100 << "%)" << std::endl;
        simulate_work(500'000);
        ++work_done;

        ipc.message_send(td->task_id, ((td->task_id + 1) % MAX_TASKS) + 1, "I am " + std::to_string((float(work_done)/WORK_AMOUNT)*100) + "%) my work!", Message_Type(2));

        std::cout << "Mailbox message count: " << ipc.message_count(td->task_id) << " Total: " << ipc.message_count() << std::endl;

        // int result = ipc.message_receive(td->task_id, messageStorage);

        // if (result == -1) std::cout << "An error occurred when reading mailbox.\n";
        // else if (result > 0) std::cout << "Message Received: " << messageStorage->text << " Sender: " << messageStorage->source_task_id << "\n";
        
        // Let the scheduler decide if we should pause or not
        scheduler.yield();
        while (scheduler.get_state(td->task_id) != RUNNING)
            usleep(10000);
    }

    // Release resource
    sem.up(td->task_id);

    std::cout << " -------------------- Task " << td->thread_no << " Finished --------------------\n\n";

    ipc.message_send(td->task_id, ((td->task_id + 1) % MAX_TASKS) + 1, "I just finished my work!", Message_Type(2));
    std::cout << ipc.message_dump(td->task_id) << std::endl;

    scheduler.set_state(td->task_id, DEAD);
    return nullptr;
}

/**
 * Wastes time.
 */
void simulate_work(int amount)
{
    usleep(amount);
}
