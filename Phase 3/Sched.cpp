#include "Sched.h"

Scheduler::Scheduler()
{
    process_table = nullptr;    // No processes yet
    current_task = -1;          // No task is running
    current_quantum = 3000;     // Alloted runtime for each task (milliseconds)
    next_available_id = 1;      // Start ids at 1
}

Scheduler::~Scheduler()
{
    LOG("Exiting Scheduler....." << std::endl);
}

int Scheduler::create_task(std::string task_name, void *(*task_function)(void *), void *args)
{
    TCB *new_task = new TCB();

    new_task->task_id = next_available_id++;
    new_task->task_name = task_name;
    new_task->state = READY;
    
    pthread_mutex_lock(&lock);
    new_task->next = process_table;
    process_table = new_task;
    pthread_mutex_unlock(&lock);

    int result = pthread_create(&(new_task->thread), nullptr, task_function, args);

    assert(!result); // if there are any problems with result, display it and end program.

    return new_task->task_id;
}

void Scheduler::start()
{
    // process_table points to first task
    process_table->start_time = std::chrono::steady_clock::now();
    process_table->state = RUNNING;
    current_task = process_table->task_id;
    

    sleep(1);
}

TCB *Scheduler::get_tcb_pointer(int task_id)
{
    TCB *t = process_table;
    while (t != nullptr)
    {
        if (t->task_id == task_id) return t;
        t = t->next;
    }
    return nullptr;
}

void Scheduler::set_state(int task_id, std::string new_state)
{
    TCB *t = get_tcb_pointer(task_id);
    std::string old_state = t->state;

    pthread_mutex_lock(&lock);

    t->state = new_state;

    LOG("task_id="<<task_id<<": " << old_state << " -> " << new_state << std::endl);

    // The currently running task stopped, so switch to another task.
    // If there are no ready tasks, then nothing is running.
    if (old_state == RUNNING && new_state != RUNNING)
    {
        bool task_switched = try_switch_task();

        if (!task_switched)
        {
            // The running task stopped, and no other task is ready to replace
            // it. Now there is no currently running task.
            current_task = -1;
        }
    }
    // If no task is running and this task becomes ready, start it.
    else if (current_task == -1 && new_state == READY)
    {
        LOG("set_state(): No tasks are running. Starting task_id " << task_id << "..." << std::endl);
        LOG(dump() << std::endl);
        t->state = RUNNING;
        current_task = task_id;
        LOG("task_id="<<task_id<<": " << READY << " -> " << RUNNING << std::endl);
    }

    pthread_mutex_unlock(&lock);
}

std::string Scheduler::get_state(int task_id)
{
    TCB *t = get_tcb_pointer(task_id);
    return t->state;
}

int Scheduler::get_task_id()
{
    return current_task;
}

void Scheduler::kill_task(int task_id)
{
    set_state(task_id, DEAD);
}

void Scheduler::yield()
{
    pthread_mutex_lock(&lock);

    TCB *running_task = get_tcb_pointer(current_task);

    if (running_task == nullptr)
    {
        LOG("Running task not found" << std::endl);
        return;
    }

    // Calculate elapsed_time since the task last started to run.
    // Note: To simplify testing, this is based on a timer instead of the CPU
    // clock.
    // clock_t elapsed_time = clock() - running_task->start_time;
    auto now = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - running_task->start_time).count();

    // If current task is RUNNING we change its state to READY
    if (elapsed_time >= current_quantum) {
        try_switch_task();
    }

    pthread_mutex_unlock(&lock);
}

void Scheduler::wait_for_all_threads()
{
    TCB *t = process_table;
    while (t != nullptr)
    {
        pthread_join(t->thread, NULL);
        t = t->next;
    }
}

std::string Scheduler::dump(int level)
{
    std::stringstream str;

    if (level != 1 && level != 2) // If not expected level, print error and exit
    {
        str << level << " is an invalid level" << std::endl;
        return str.str();
    }
    str << " -----------Process Table-----------\n";
    str << " Name\tID\tState\tStart\n";
    str << " -----------------------------------\n";

    TCB *current_task = process_table;
    int count = 0;
    while (current_task != nullptr)
    {
        auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_task->start_time.time_since_epoch()).count();
        str << " " << current_task->task_name << "\t" << std::to_string(current_task->task_id) << "\t" << current_task->state << "\t" << std::to_string(start_time) << "\n";

        current_task = current_task->next;
        count++;
    }

    str << " -----------------------------------";

    return str.str();
}

bool Scheduler::try_switch_task()
{
    // Note: This function assumes pthread_mutex_lock(&lock); has already been
    // called.

    // Loop through every task in the process table until the original task is
    // reached again
    TCB *t = get_tcb_pointer(current_task);
    t = t->next;
    if (t == nullptr)
        t = process_table;
    while (t->task_id != current_task)
    {
        if (t->state == READY)
        {
            // Another ready task is found
            TCB* current = get_tcb_pointer(current_task);
            if (current->state == RUNNING)
            {
                LOG("task_id="<<current->task_id<<": " << current->state << " -> " << READY << std::endl);
                current->state = READY;
            }

            current_task = t->task_id;
            LOG("task_id="<<current_task<<": " << t->state << " -> " << RUNNING << std::endl);
            t->state = RUNNING;
            t->start_time = std::chrono::steady_clock::now();

            LOG("Task switched\n" << dump() << std::endl << std::endl);
            return true;
        }

        // Advance task pointer
        t = t->next;
        if (t == nullptr)
            t = process_table;
    }

    LOG("try_switch_task(): No other task found" << std::endl << std::endl);
    return false;
}