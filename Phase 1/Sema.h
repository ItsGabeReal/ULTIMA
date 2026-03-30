/**
 * Sema.h
 * 
 * Basic semaphore class with variable values. Use the down() function to start
 * using a resource, and up() to release it. If all resources are in use when
 * down() is called, that thread will pause until a resource becomes available.
 *
 * @author Gabriel Wilson
 * @date 3/27/2026
 */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include "Queue.h"
#include "Sched.h"

class Semaphore
{
private:
    std::string resource_name; // Name of this semaphore
    int sema_value;
    Queue sema_queue;
    Scheduler* sched_ptr;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int lucky_task;

public:
    /**
     * Constructor
     * 
     * @param res_name Name of the resource this semaphore regulates.
     * @param initial_value Number of resources available in this semaphore
     * (default: 1).
     */
    Semaphore(std::string res_name, Scheduler *scheduler, int initial_value);

    /**
     * Destructor!!!
     */
    ~Semaphore();

    /**
     * Requests access to a resource. If value is <= 0, the calling thread
     * gets blocked and put on a waitlist for the resource.
     */
    void down(int thread_id);

    /**
     * Releases usage of the resource. If there are threads on the waitlist, they
     * will resume execution.
     */
    void up();

    /**
     * Prints details to the console, including 
     * 
     * @param level Specifies the amount of information printed (Default: 2).
     */
    void dump(int level = 1);
};

#endif
