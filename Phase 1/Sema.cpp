#include "Sema.h"

semaphore::semaphore(char* res_name, int initialValue)
    : resource_name(res_name), sema_value(initialValue)
{
    // Init lock and condition
    pthread_mutex_init(&lock, nullptr);
    pthread_cond_init(&cond, nullptr);
}

semaphore::~semaphore()
{
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    // Note: sema_queue will be freed when its destructor is called, so
    // there's not need to free it here.
}

void semaphore::down(int thread_id)
{
    pthread_mutex_lock(&lock);

    // Enter critical region
    if (sema_value >= 1)
        --sema_value;
    else
    {
        pthread_t self = pthread_self();
        std::cout << "\tThread - " << thread_id << " is being placed on queue (Internal Thread No - " << self << ")" << std::endl;
        sema_queue.enqueue(thread_id);

        do
        {
            pthread_cond_wait(&cond, &lock);
        } while (sema_value < 0);

        std::cout << "\tThread - " << thread_id << " just got released from the queue and re-acquired mutex lock" << std::endl;
    }
    
    // Exit critical region

    pthread_mutex_unlock(&lock);
}

void semaphore::up(int thread_id)
{
    pthread_mutex_lock(&lock);

    // Enter critical region

    if (sema_value <= 0)
    {
        int id;
        if (!sema_queue.is_empty())
        {
            id = sema_queue.dequeue();
        }
        std::cout << "\tSignal Blocked Thread " << id << " to be released\n";

        pthread_cond_signal(&cond); // Send signal to release blocked threads
    }
    else
        ++sema_value;
    
    // Exit critical region

    pthread_mutex_unlock(&lock);
}

void semaphore::dump(int level)
{
    if (level == 1) // Level 1 - Print single line
        std::cout << resource_name << " - Current value = " << sema_value << std::endl;
    
    else if (level == 2) // Level 2 - Print multiple lines
    {
        std::cout << "Resource:\t" << resource_name << std::endl;
        std::cout << "Sema_value:\t" << sema_value << std::endl;
        std::cout << "Sema_queue:\t" << sema_queue.to_string() << std::endl;
    }
    else // Print error if level is invalid
        std::cerr << level << " is an invalid level" << std::endl;
}
