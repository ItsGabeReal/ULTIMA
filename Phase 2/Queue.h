/**
 * Queue.h
 *
 * A singly-linked-list FIFO queue.
 *
 * @author Gabriel Wilson
 * @date 3/27/2026
 */

#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

/**
 * Internal linked-list node used by Queue.
 */
template <typename T>
struct Node
{
    T value;
    Node* next;
};

/**
 * A FIFO queue using a singly linked list.
 */
template <typename T>
class Queue
{
private:
    Node<T>* head;
    Node<T>* tail;

public:
    // Constructs an empty queue
    Queue() : head(nullptr), tail(nullptr) {}

    // Destructor that frees all items
    ~Queue()
    {
        clear();
    }

    // Deletes all items
    void clear()
    {
        Node<T>* n;
        while (head != nullptr)
        {
            n = head->next;
            delete head;
            head = n;
        }

        tail = nullptr;
    }

    // Adds an element to the back of the queue
    void enqueue(T value)
    {
        Node<T>* n = new Node<T>();
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

    /**
     * Removes and returns the element at the front of the queue
     *
     * @return The value that was at the front.
     * @throws `std::underflow_error` if the queue is empty.
     */
    T dequeue()
    {
        if (is_empty())
            throw std::underflow_error("Queue is empty");

        T value = head->value;

        if (head == tail)
        {
            delete head;
            head = tail = nullptr;
        }
        else
        {
            Node<T>* n = head;
            head = head->next;
            delete n;
        }

        return value;
    }

    // Returns the number of items in the queue
    int length()
    {
        int output = 0;
        Node<T>* n = head;
        while (n != nullptr)
        {
            ++output;
            n = n->next;
        }

        return output;
    }

    // Returns true if the queue is empty
    bool is_empty()
    {
        return head == nullptr;
    }

    /**
     * Returns a string in the form "value1 -> value2 -> ... -> valueN", where
     * value1 is the head, and valueN is the tail.
     */
    std::string to_string() const
    {
        std::stringstream str;

        Node<T>* n = head;
        while (n != nullptr)
        {
            str << n->value;
            if (n->next != nullptr)
                str << "->";

            n = n->next;
        }

        return str.str();
    }
};
