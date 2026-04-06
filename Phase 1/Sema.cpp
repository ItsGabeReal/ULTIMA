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
            pthread_mutex_unlock(&lock);
        }
        else
        {
            LOG("down() | task_id=" << task_id << ": Resource is in use. Entering queue..." << std::endl);
            sema_queue.enqueue(task_id);
            sched_ptr->set_state(task_id, BLOCKED);

            pthread_mutex_unlock(&lock);

            // Wait until resource is available
            do {
                usleep(10000); // 10ms delay
            } while (lucky_task != task_id);


            LOG("down() | task_id=" << task_id << ": Gained access to resource" << std::endl);
            
            // Mark it as ready
            sched_ptr->set_state(task_id, READY);
        }
    }
}

void Semaphore::up()
{
    // Make sure the task that currently owns the resource is the task that's
    // currently running in the scheduler.
    if (sched_ptr->get_task_id() != lucky_task)
    {
        LOG( "Invalid Semaphore UP(). TaskID: "
            << sched_ptr->get_task_id()
            << " does not own the resource" << std::endl);
        
        return;
    }

    pthread_mutex_lock(&lock);

    if (sema_queue.is_empty())
    {
        lucky_task = -1;
        ++sema_value;
    }
    else
        lucky_task = sema_queue.dequeue();

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
        str << " Sema_Queue: " << sema_queue.to_string();
    }
    else
        str << " " << level << " is an invalid semaphore dump level";
    
    return str.str();
}
