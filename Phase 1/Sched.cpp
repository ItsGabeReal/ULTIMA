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
    new_task->start_time = clock();
    new_task->next = process_table;
    process_table = new_task;

    int result = pthread_create(&(new_task->thread), nullptr, task_function, args);
    assert(!result); // if there are any problems with result, display it and end program.

    return new_task->task_id;
}

void Scheduler::dump(WINDOW *Win, int level)
{
    if (level != 1 && level != 2) // If not expected level, print error and exit
    {
        std::cerr << level << " is an invalid level" << std::endl;
        return;
    }
    wHelper.clear_window(Win);
    wHelper.write_window(Win, 1, 0, " -----------Process Table-----------\n");
    wHelper.write_window(Win, " Name\tID\tState\tStart\n");
    wHelper.write_window(Win, " -----------------------------------\n");

    TCB *current_task = process_table;
    int count = 0;
    while (current_task != nullptr)
    {
        wHelper.write_window(Win, " " + current_task->task_name + "\t" + std::to_string(current_task->task_id) + "\t" + current_task->state + "\t" + std::to_string(current_task->start_time) + "\n");

        current_task = current_task->next;
        count++;
    }

    wHelper.write_window(Win, " -----------------------------------\n");
}

// This testing should be handled in Ultima.cpp  \/
// int main()
// {
//     Scheduler scheduler;

//     scheduler.create_task("Task1");
//     scheduler.create_task("Task2");
//     scheduler.create_task("Task3");
//     scheduler.create_task("Task4");

//     scheduler.dump(1);
//     for (int i = 0; i < 1000000000; i++)
//     {
//         int j = i + i * i;
//         if (i % 100000000 == 0)
//         {
//             scheduler.create_task("Task" + std::to_string(i / 100000000));
//         }
//     }
//     scheduler.dump(1);
// }