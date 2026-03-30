#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "Sema.h"
#include "Sched.h"

Scheduler scheduler;
Semaphore sem("Test resource", &scheduler, 1);

void* worker(void* arg)
{
    int id = *((int*)arg);
    std::cout << "Task " << id << " started \n";
    
    //acquire the resources"
    sem.down(0);

    std::cout << "Task " << id << " Enter critical section \n";
    sleep(2); // Simulate work

    std::cout << "Task " << id << " Exit critical section \n";

    sem.up();

    scheduler.set_state(id, DEAD);
    return nullptr;
}

int main()
{
    std::cout << "Starting unit test for Semaphore and Scheduler \n";

    // Create worker threads
    int id1 = 1, id2 = 2, id3 = 3;
    //create tasks using scheduler
    scheduler.create_task("Task1", worker, &id1);
    scheduler.create_task("Task2", worker, &id2);
    scheduler.create_task("Task3", worker, &id3);

    //start scheduler
    scheduler.start();
    sleep(10); // Let the tasks run

    std::cout << "Unit test completed \n";
    return 0;
}
