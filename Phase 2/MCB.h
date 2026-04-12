#ifndef MCB_H
#define MCB_H

#include "Sched.h"
#include "IPC.h"
#include "Sema.h"

struct MCB
{
    Scheduler scheduler;
    IPC messenger;
    Semaphore monitor;
    Semaphore printer;

    MCB(int max_tasks)
        : scheduler(),
          messenger(max_tasks),
          monitor("Monitor", 1),
          printer("Printer", 1)
    {
        Semaphore::set_scheduler_ptr(&scheduler);
    }
};

#endif