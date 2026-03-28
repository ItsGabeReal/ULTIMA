#include "Sched.h"

int main() {
    Scheduler scheduler;

    scheduler.create_task("Task1");
    scheduler.create_task("Task2");
    scheduler.create_task("Task3");
    scheduler.create_task("Task4");

    scheduler.dump(1);
    for (int i = 0; i < 1000000000; i++) {
        int j = i + i * i;
        if (i % 100000000 == 0) {
            scheduler.create_task("Task" + std::to_string(i/100000000));
        }
    }
    scheduler.dump(1);
}