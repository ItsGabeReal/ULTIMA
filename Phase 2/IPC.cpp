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

int IPC::message_send(int s_id, int d_id, std::string text, Message_Type type)
{
    Message* message = new Message();
    message->source_task_id = s_id;
    message->destination_task_id = d_id;
    message->arrival_time = clock();
    message->type = type;
    message->text = text;

    Mailbox* mailbox = get_mailbox(d_id);
    if (mailbox == nullptr) return -1;

    mailbox->sem.down(s_id);
    mailbox->messages.enqueue(*message);
    mailbox->sem.up(s_id);

    LOG("Task" << s_id << " just sent a message to Task" << d_id << "." << std::endl);

    return 0;
}

int IPC::message_receive(int task_id, Message *message)
{
    if (message == nullptr) return -1;

    Mailbox* mailbox = get_mailbox(task_id);
    if (mailbox == nullptr) return -1;

    mailbox->sem.down(task_id);
    int message_count = mailbox->messages.length();
    if (message_count != 0) {
        *message = mailbox->messages.dequeue();
    }
    mailbox->sem.up(task_id);

    return message_count;
}

int IPC::message_count(int task_id)
{
    Mailbox* mailbox = get_mailbox(task_id);

    if (mailbox == nullptr) return -1;

    return mailbox->messages.length();
}

int IPC::message_count()
{
    int total = 0;

    Mailbox* m = mailboxes;
    while (m != nullptr)
    {
        total += m->messages.length();
        m = m->next;
    }

    return total;
}

std::string IPC::message_dump()
{
    std::stringstream str;

    str << "---------------------- Total Message Dump ---------------------" << std::endl;
    str << "Total messages: " << message_count() << std::endl;
    str << "---------------------------------------------------------------" << std::endl;
    str << "Source\tDest\tContent\t\t\t\tType\tArrival" << std::endl;
    str << "TaskID\tTaskID" << std::endl;
    str << "---------------------------------------------------------------" << std::endl;

    Mailbox* m = mailboxes;
    while (m != nullptr)
    {
        int mailbox_count = m->messages.length();
        for (int i = 0; i < mailbox_count; i++)
        {
            Message temp = m->messages.dequeue();
            str << temp.source_task_id << "\t" << temp.destination_task_id << "\t" << temp.text
            << "\t" << temp.type << "\t" << temp.arrival_time << std::endl;
            m->messages.enqueue(temp);
        }
        m = m->next;
    }
    str << "---------------------------------------------------------------" << std::endl;

    return str.str();
}

std::string IPC::message_dump(int task_id)
{
    Mailbox* m = get_mailbox(task_id);

    if (m == nullptr) return "An error occurred when getting the mailbox.\n";

    std::stringstream str;

    str << "------------------------ Message Dump -------------------------" << std::endl;
    str << "Task" << task_id << " Mailbox\tMessage count: " << message_count(task_id) << std::endl;
    str << "---------------------------------------------------------------" << std::endl;
    str << "Source\tDest\tContent\t\t\t\tType\tArrival" << std::endl;
    str << "TaskID\tTaskID" << std::endl;
    str << "---------------------------------------------------------------" << std::endl;

    int mailbox_count = m->messages.length();
    for (int i = 0; i < mailbox_count; i++)
    {
        Message temp = m->messages.dequeue();
        str << temp.source_task_id << "\t" << temp.destination_task_id << "\t" << temp.text
        << "\t" << temp.type << "\t" << temp.arrival_time << std::endl;
        m->messages.enqueue(temp);
    }

    str << "---------------------------------------------------------------" << std::endl;

    return str.str();
}

int IPC::message_delete_all(int task_id)
{
    Mailbox* m = get_mailbox(task_id);

    if (m == nullptr) return -1;

    int deleted_count = 0;
    while (m->messages.length() > 0)
    {
        m->messages.dequeue();
        deleted_count++;
    }
    return deleted_count;
}