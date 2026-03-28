/**
 * Queue.h
 * 
 * A singly-linked-list FIFO queue of type int.
 *
 * @author Gabriel Wilson
 * @date 3/27/2026
 */

#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

/**
 * Internal linked-list node used by queue.h.
 */
struct Node
{
    int value;
    Node* next;
};

/**
 * A FIFO queue using a singly linked list.
 */
class Queue
{
private:
    Node* head;
    Node* tail;

public:
    // Constructs an empty queue
    Queue() : head(nullptr), tail(nullptr) {}

    // Destructor that frees all items
    ~Queue();

    // Deletes all items
    void clear();

    // Adds an element to the back of the queue
    void enqueue(int value);

    /**
     * Removes and returns the element at the front of the queue
     * 
     * @return The value that was at the front.
     * @throws `std::underflow_error` if the queue is empty.
     */
    int dequeue();

    // Returns the number of items in the queue
    int length();

    // Returns true if the queue is empty
    bool is_empty();

    /**
     * Returns a string in the form "value1 -> value2 -> ... -> valueN", where
     * value1 is the head, and valueN is the tail.
     */
    std::string to_string();
};

