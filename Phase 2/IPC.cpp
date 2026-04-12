#include "IPC.h"

void IPC::create_mailbox(int tid, std::string name)
{
    Mailbox* new_mailbox = new Mailbox(tid, name);
    new_mailbox->next = mailboxes;
    mailboxes = new_mailbox;
}

Mailbox* IPC::get_mailbox(int tid)
{
    Mailbox* current_mailbox = mailboxes;
    while (current_mailbox != nullptr)
    {
        if (current_mailbox->task_id == tid) return current_mailbox;
        current_mailbox = current_mailbox->next;
    }
    return nullptr;
}

IPC::IPC(int max_tasks)
{
    this->max_tasks = max_tasks;

    // Initialize the mailboxes
    for (int i = max_tasks; i >=0; i--)
    {
       create_mailbox(i, "Mailbox" + std::to_string(i));
    }
    LOG("Created all " << max_tasks << " mailboxes." << std::endl);
    // LOG("Getting task 3's mailbox..." << std::endl);
    // Mailbox* t3 = get_mailbox(3);
    // LOG(t3->sem.dump());
}

// int IPC::message_send(int s_id, int d_id, std::string text, Message_Type type)
// {

// }