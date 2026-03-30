#include <iostream>
#include "Sema.h"
#include "Sched.h"

int main(int argc, char const *argv[])
{
    std::cout << "Hello, world!" << std::endl;

    Scheduler s;
    Semaphore sem("Test resource", &s, 1);
    return 0;
}
