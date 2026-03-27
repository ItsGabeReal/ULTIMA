#include "Queue.h"
#include <sstream>

queue::~queue()
{
    clear();
}

void queue::clear()
{
    node* n;
    while (head != nullptr)
    {
        n = head->next;
        delete head;
        head = n;
    }

    tail = nullptr;
}

void queue::enqueue(int value)
{
    node* n = new node();
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

int queue::dequeue()
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
        node* n = head;
        head = head->next;
        delete n;
    }

    return value;
}

int queue::length()
{
    int output = 0;
    node* n = head;
    while (n != nullptr)
    {
        ++output;
        n = n->next;
    }

    return output;
}

bool queue::is_empty()
{
    return head == nullptr;
}

std::string queue::to_string()
{
    std::stringstream str;

    node* n = head;
    while (n != nullptr)
    {
        str << n->value;
        if (n->next != nullptr) // Do not add arrow after last item
            str << "->";

        n = n->next; // Move to next item
    }

    return str.str();
}

void queue::print()
{
    // Print size
    int size = 0;
    node* n = head;
    while (n != nullptr)
    {
        ++size;
        n = n->next;
    }
    std::cout << "Queue Size = " << size << std::endl;

    // Print queue contents
    std::cout << "Queue Head<-";
    n = head;
    while (n != nullptr)
    {
        std::cout << n->value << "<-";
        n = n->next;
    }
    std::cout << "Tail" << std::endl;
}
