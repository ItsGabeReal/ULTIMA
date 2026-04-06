#include "Sema.h"

Semaphore::Semaphore(std::string res_name, Scheduler *scheduler, int initial_value)
    : resource_name(res_name), sched_ptr(scheduler), sema_value(initial_value), lucky_task(-1)
{
    // Init lock and condition
    pthread_mutex_init(&lock, nullptr);
    pthread_cond_init(&cond, nullptr);
}

Semaphore::~Semaphore()
{
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    // Note: sema_queue will be freed when its destructor is called, so
    // there's not need to free it here.
}

void Semaphore::down(int task_id)
{
    pthread_mutex_lock(&lock);

    if (task_id == lucky_task)
    {
        // This task already has the resource, so do nothing
    }
    else
    {
        if (sema_value >= 1)
        {
            --sema_value;
            lucky_task = task_id;
        }
        else
        {
            sema_queue.enqueue(task_id);
            sched_ptr->set_state(task_id, BLOCKED);

            do
            {
                pthread_cond_wait(&cond, &lock);
            } while (sema_value < 0);
            
            sched_ptr->yield();
        }
    }

    pthread_mutex_unlock(&lock);
}

void Semaphore::up()
{
    pthread_mutex_lock(&lock);

    int task_id;

    // std::cout << "TaskID: " << sched_ptr->get_task_id() << ", LuckyID: " << lucky_task << std::endl;

    if (sched_ptr->get_task_id() == lucky_task)
    {
        if (sema_queue.is_empty())
        {
            lucky_task = -1;
            dump();
        }
        else
        {
            task_id = sema_queue.dequeue();
            sched_ptr->set_state(task_id, READY);
            // std::cout << "UnBlock: " << task_id << " and release from the queue" << std::endl;
            sched_ptr->yield();
            dump();
        }

        pthread_cond_signal(&cond); // Send signal to release blocked threads
    }
    else
    {
        // std::cout << "Invalid Semaphore UP(). TaskID: "
        //     << sched_ptr->get_task_id()
        //     << " does not own the resource" << std::endl;
        
        dump();
    }

    pthread_mutex_unlock(&lock);
}

std::string Semaphore::dump(int level) const
{
    std::stringstream str;
    str << " ---------- SEMAPHORE DUMP ----------" << std::endl;
    if (level == 0)
    {
        str << " Sema_Value: " << sema_value << std::endl;
        str << " Sema_Name: " << resource_name << std::endl;
        str << " Obtained by Task-ID: " << lucky_task << std::endl;
    }
    else if (level == 1)
    {
        str << " Sema_Value: " << sema_value << std::endl;
        str << " Sema_Name: " << resource_name << std::endl;
        str << " Obtained by Task-ID: " << lucky_task << std::endl;
        str << " Sema_Queue: " << sema_queue.to_string() << std::endl;
    }
    else
        str << " " << level << " is an invalid semaphore dump level" << std::endl;
    
    return str.str();
}
