#include "Queue.h"
#include <sstream>

Queue::~Queue()
{
    clear();
}

void Queue::clear()
{
    Node* n;
    while (head != nullptr)
    {
        n = head->next;
        delete head;
        head = n;
    }

    tail = nullptr;
}

void Queue::enqueue(int value)
{
    Node* n = new Node();
    n->value = value;
    n->next = nullptr;

    if (tail == nullptr)
    {
        tail = head = n;
    }
    else
    {
        tail->next = n;
        tail = n;
    }
}

int Queue::dequeue()
{
    if (is_empty())
        throw std::underflow_error("Stack is empty");
    
    int value = head->value;

    if (head == tail)
    {
        delete head;
        head = tail = nullptr;
    }
    else
    {
        Node* n = head;
        head = head->next;
        delete n;
    }

    return value;
}

int Queue::length()
{
    int output = 0;
    Node* n = head;
    while (n != nullptr)
    {
        ++output;
        n = n->next;
    }

    return output;
}

bool Queue::is_empty()
{
    return head == nullptr;
}

std::string Queue::to_string() const
{
    std::stringstream str;

    Node* n = head;
    while (n != nullptr)
    {
        str << n->value;
        if (n->next != nullptr) // Do not add arrow after last item
            str << "->";

        n = n->next; // Move to next item
    }

    return str.str();
}
