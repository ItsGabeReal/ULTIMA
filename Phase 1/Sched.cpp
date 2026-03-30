#include "Sched.h"

Scheduler::Scheduler()
{
    process_table = nullptr;    // No processes yet
    current_task = -1;          // No task is running
    current_quantum = 500;      // Set quantum length to 500ms
    next_available_id = 1;      // Start ids at 1
}

Scheduler::~Scheduler()
{
    std::cout << "Exiting Scheduler....." << std::endl;
}

int Scheduler::create_task(std::string task_name, void *(*task_function)(void *), void *args)
{
    TCB *new_task = new TCB();

    new_task->task_id = next_available_id++;
    new_task->task_name = task_name;
    new_task->state = READY;
    // new_task->start_time = clock();
    new_task->next = process_table;
    process_table = new_task;

    int result = pthread_create(&(new_task->thread), nullptr, task_function, args);
    assert(!result); // if there are any problems with result, display it and end program.

    return new_task->task_id;
}

void Scheduler::start()
{
    // process_table points to first task
    process_table->start_time = clock();
    process_table->state = RUNNING;
    current_task = process_table->task_id;
    current_quantum = 1000 / MAX_TASKS;

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
    t->state = new_state;
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
    TCB *t = get_tcb_pointer(task_id);
    t->state = DEAD;
}

void Scheduler::yield()
{
    int counter = 0;
    // wManager.log("Current Task # " + std::to_string(current_task) + " is trying to Yield\n");

    // Calculate elapsed_time since the task last started to run.
    clock_t elapsed_time = clock() - get_tcb_pointer(current_task)->start_time;
    // wManager.log("Task: " + std::to_string(current_task) + ", Elapsed time: " + std::to_string(elapsed_time) + "\n");
    // wManager.log("Current Quantum: " + std::to_string(current_quantum) + "\n");

    if (elapsed_time < current_quantum) 
    {
        // wManager.log("NO Yield!   (task: " + std::to_string(current_task) + " still has some quantum left)\n");
        return;
    }

    // wManager.log("Yielding.... (Switching from task #" + std::to_string(current_task) + " to next ready task)");

    // If current task is RUNNING we change its state to READY
    TCB *running_task = get_tcb_pointer(current_task);
    if (running_task != nullptr && running_task->state == RUNNING)
        running_task->state = READY;

    // Find the next READY task and make it RUNNING
    current_task = (current_task % MAX_TASKS) + 1;
    while (get_tcb_pointer(current_task)->state != READY && counter < MAX_TASKS)
    {
        current_task = (current_task % MAX_TASKS) + 1;
        counter++;
    }

    // If we find a READY task, start it. If not, possible DEAD LOCK situtation
    TCB *found_task = get_tcb_pointer(current_task);
    if (counter < MAX_TASKS && found_task->state == READY)
    {
        found_task->start_time = clock();
        found_task->state = RUNNING;
        // wManager.log("Started running task # " + std::to_string(current_task) + "\n");
    }
    else {}
        // wManager.log("POSSIBLE DEAD LOCK");
}


void Scheduler::dump(int level)
{
    return;
    if (level != 1 && level != 2) // If not expected level, print error and exit
    {
        std::cerr << level << " is an invalid level" << std::endl;
        return;
    }
    // wManager.clear_window(Win, 0);
    // wManager.write_window(Win, 0, 1, 0, " -----------Process Table-----------\n");
    // wManager.write_window(Win, 0, " Name\tID\tState\tStart\n");
    // wManager.write_window(Win, 0, " -----------------------------------\n");

    TCB *current_task = process_table;
    int count = 0;
    while (current_task != nullptr)
    {
        // wManager.write_window(Win, 0, " " + current_task->task_name + "\t" + std::to_string(current_task->task_id) + "\t" + current_task->state + "\t" + std::to_string(current_task->start_time) + "\n");

        current_task = current_task->next;
        count++;
    }

    // wManager.write_window(Win, 0, " -----------------------------------\n");
}