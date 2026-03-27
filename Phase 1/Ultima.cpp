// This is all temporary code to test the semaphore

#include "Sema.h"
#include <pthread.h>
#include <unistd.h>

void* test_thread(void* arg);

semaphore sem("Test semaphore", 1);

int main()
{
    const int THREAD_COUNT = 3;
    pthread_t threads[THREAD_COUNT];

    for (long i = 0; i < THREAD_COUNT; ++i)
        pthread_create(&threads[i], nullptr, test_thread, (void*)i);
    
    for (long i = 0; i < THREAD_COUNT; ++i)
        pthread_join(threads[i], nullptr);

    return 0;
}

void* test_thread(void* arg)
{
    long id = (long)arg;

    sem.down(id);

    sem.dump();

    sleep(3);

    sem.up(id);

    return nullptr;
}