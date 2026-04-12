/**
 * IPC.h
 *
 * Class to manage message passing between tasks.
 *
 * @author Colin Christy
 * @date 4/11/2026
 */

#ifndef IPC_H
#define IPC_H

#include "Queue.h"
#include "Sema.h"

enum Message_Type
{
    TEXT = 0,
    SERVICE = 1,
    NOTIFICATION = 2
};

struct Message
{
    int source_task_id;
    int destination_task_id;
    time_t arrival_time;
    Message_Type type;
    std::string text;
};

struct Mailbox
{
    int task_id;
    Queue<Message> messages;
    Semaphore sem;
    Mailbox *next;

    /**
     * Constructor for a new Mailbox. Needed constructor to initialize the semaphore.
     *
     * @param tid Id of the task that will use this mailbox
     * @param name Name assigned to the mailbox semaphore for debug purposes
     */
    Mailbox(int tid, std::string name) : sem(name)
    {
        task_id = tid;
    };
};

class IPC
{
private:
    int max_tasks;
    Mailbox *mailboxes;

    /**
     * Creates a new mailbox and adds it to the mailboxes linked list.
     * 
     * @param tid Id of the task that will use this mailbox
     * @param name Name assigned to the mailbox semaphore for debug purposes
     */
    void create_mailbox(int tid, std::string name);

    /**
     * Gets a pointer to the mailbox for the specified task.
     * 
     * @param tid Id of the task that uses the mailbox
     */
    Mailbox* get_mailbox(int tid);

public:
    /**
     * Constructor
     */
    IPC(int max_tasks);

    /**
     * Send a message from one task to another.
     *
     * @param s_id Sender's task id
     * @param d_id Destination's task id
     * @param text Text content of the message
     * @param type Type of message (Text = 0, Service = 1, Notification = 2)
     */
    int message_send(int s_id, int d_id, std::string text, Message_Type type);

    /**
     * Read a message from specified task's mailbox.
     *
     * @param task_id Id of task
     * @param message Location to put the received message
     */
    int message_receive(int task_id, Message *message);

    /**
     * Returns the number of messages in the specified task's mailbox.
     *
     * @param task_id Id of task
     */
    int message_count(int task_id);

    /**
     * Returns the number of messages in all mailboxes.
     */
    int message_count();

    /**
     * Dump all messages from all mailboxes.
     */
    std::string message_dump();

    /**
     * Dump all messages from the specified task's mailbox.
     */
    std::string message_dump(int task_id);

    /**
     * Delete all messages from the specified task's mailbox.
     */
    int message_delete_all(int task_id);
};

#endif