#include "Sched.h"

Scheduler::Scheduler()
{
    process_table = nullptr;    // No processes yet
    current_task = -1;          // No task is running
    current_quantum = 500;      // Set quantum length to 500ms
}

Scheduler::~Scheduler()
{
    std::cout << "Exiting Scheduler....." << std::endl;
}

int Scheduler::create_task(std::string task_name, void *(*task_function)(void *))
{
    TCB *new_task = new TCB();

    new_task->task_name = task_name;
    new_task->state = READY;
    new_task->start_time = clock();
    new_task->next = process_table;
    process_table = new_task;

    new_task->task_id = pthread_create(&(new_task->thread), nullptr, task_function, (void*)1);

    return new_task->task_id;
}

void Scheduler::dump(int level)
{
    if (level != 1 && level != 2) // If not expected level, print error and exit
    {
        std::cerr << level << " is an invalid level" << std::endl;
        return;
    }

    std::cout << "-----------Process Table-----------" << std::endl;

    TCB *current_task = process_table;
    int count = 0;
    while (current_task != nullptr)
    {
        std::cout << std::setw(7) << current_task->task_name
                  << std::setw(7) << current_task->task_id
                  << std::setw(7) << current_task->state
                  << std::setw(7) << current_task->start_time
                  << std::endl;

        current_task = current_task->next;
        count++;
    }

    std::cout << "-----------------------------------" << std::endl;
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