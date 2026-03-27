#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "Queue.h"

class semaphore
{
private:
    char* resource_name; // Name of this semaphore
    int sema_value;
    queue sema_queue;
    pthread_mutex_t lock;
    pthread_cond_t cond;

public:
    /**
     * Default constructor
     * 
     * @param res_name Name of the resource this semaphore regulates.
     * @param initial_value Number of resources available in this semaphore
     * (default: 1).
     */
    semaphore(char* res_name, int initial_value = 1);

    /**
     * Destructor!!!
     */
    ~semaphore();

    /**
     * Requests access to a resource. If value is <= 0, the calling thread
     * gets blocked and put on a waitlist for the resource.
     */
    void down(int thread_id);

    /**
     * Releases usage of the resource. If there are threads on the waitlist, they
     * will resume execution.
     */
    void up(int thread_id);

    /**
     * Prints details to the console, including 
     * 
     * @param level Specifies the amount of information printed (Default: 2).
     */
    void dump(int level = 2);
};
