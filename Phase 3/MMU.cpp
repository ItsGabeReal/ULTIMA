#include "MMU.h"

MMU::MMU(int size, char default_initial_value, int page_size)
{
    if (page_size > size)
        throw "'size' cannot be less than 'page_size'.";

    if (size % page_size != 0)
        throw "'page_size' must evenly divide 'size'.";

    this->size = size;
    this->page_size = page_size;

    for (int i = 0; i < size; i++)
    {
        data[i] = default_initial_value;
    }

    segments = new MemorySegment();
    segments->start = 0;
    segments->handle = 0;
    segments->size = size;

    sem = new Semaphore("MMU");
}

MMU::~MMU()
{
    MemorySegment *current_segment = segments;
    while (current_segment != nullptr)
    {
        MemorySegment *temp = current_segment->next;
        delete current_segment;
        current_segment = temp;
    }
}

int MMU::mem_alloc(int size, int thread_id)
{
    sem->down(thread_id);

    MemorySegment *current_segment = segments;
    // Check if task already has allocated memory
    while (current_segment != nullptr)
    {
        if (current_segment->task_id == thread_id)
        {
            sem->up(thread_id);
            return -1;
        }
        current_segment = current_segment->next;
    }

    // Task was found to not have any allocated memory

    // Reset current_segment pointer
    current_segment = segments;

    int allocate_size = ceil((float)size / page_size) * page_size;

    while (current_segment != nullptr)
    {
        if (current_segment->get_status() != "Free" || current_segment->size < allocate_size)
        {
            current_segment = current_segment->next;
            continue;
        }

        // Status is "Free" and size is big enough

        int leftover_size = current_segment->size - allocate_size;
        if (leftover_size >= page_size)
        {
            MemorySegment *new_segment = new MemorySegment();
            new_segment->start = current_segment->start + allocate_size;
            new_segment->handle = new_segment->start;
            new_segment->size = leftover_size;

            // Reassign pointers to splice new segment into linked list
            new_segment->next = current_segment->next;
            current_segment->next = new_segment;
        }

        current_segment->allocated = true;
        current_segment->size = allocate_size;
        current_segment->current = 0;
        current_segment->task_id = thread_id;

        sem->up(thread_id);
        return current_segment->handle;
    }

    // No segments were found to be free and large enough
    sem->up(thread_id);
    return -1;
}

int MMU::mem_free(int mem_handle, int thread_id)
{
    sem->down(thread_id);

    MemorySegment *current_segment = segments;
    // Find segment based on mem_handle
    while (current_segment != nullptr)
    {
        if (current_segment->handle == mem_handle)
        {
            // Check if task was the task to allocate it
            if (current_segment->task_id != thread_id)
            {
                sem->up(thread_id);
                return -1;
            }

            // Set all memory in segment to '#'
            for (int i = 0; i < current_segment->size; i++)
            {
                data[current_segment->start + i] = '#';
            }

            // Reset segment data
            current_segment->allocated = false;
            current_segment->current = -1;
            current_segment->task_id = -1;

            sem->up(thread_id);

            // Coalesce memory to combine any possible double free segments (and reset # to .)
            mem_coalesce();

            return 0;
        }
        current_segment = current_segment->next;
    }

    // Memory handle was not found in segments

    sem->up(thread_id);
    return -1;
}

int MMU::mem_read(int mem_handle, char *ch, int thread_id)
{
    sem->down(thread_id);

    MemorySegment *current_segment = segments;
    // Find segment based on mem_handle
    while (current_segment != nullptr)
    {
        if (current_segment->handle == mem_handle)
        {
            // Check if task was the task to allocate it
            if (current_segment->task_id != thread_id)
            {
                sem->up(thread_id);
                return -1;
            }

            // Reset current location to 0 if last character and return -1 for end of bounds
            if (current_segment->current >= current_segment->size)
            {
                current_segment->current = 0;

                sem->up(thread_id);
                return -1;
            }

            *ch = data[current_segment->start + current_segment->current];
            current_segment->current += 1;

            sem->up(thread_id);
            return 0;
        }
        current_segment = current_segment->next;
    }

    // Memory handle was not found in segments

    sem->up(thread_id);
    return -1;
}

int MMU::mem_read(int mem_handle, int offset_from_beg, int text_size, std::string *text, int thread_id)
{
    sem->down(thread_id);

    MemorySegment *current_segment = segments;
    // Find segment based on mem_handle
    while (current_segment != nullptr)
    {
        if (current_segment->handle == mem_handle)
        {
            // Check if task was the task to allocate it
            if (current_segment->task_id != thread_id)
            {
                sem->up(thread_id);
                return -1;
            }

            // Check if read size will fit in segment
            if (current_segment->size - offset_from_beg < text_size)
            {
                sem->up(thread_id);
                return -1;
            }
            std::string temp;

            // Read each character to the segment
            for (int i = 0; i < text_size; i++)
            {
                temp += data[current_segment->start + offset_from_beg + i];
            }

            *text = temp;

            sem->up(thread_id);
            return 0;
        }
        current_segment = current_segment->next;
    }

    // Memory handle was not found in segments

    sem->up(thread_id);
    return -1;
}

int MMU::mem_write(int mem_handle, char ch, int thread_id)
{
    sem->down(thread_id);

    MemorySegment *current_segment = segments;
    // Find segment based on mem_handle
    while (current_segment != nullptr)
    {
        if (current_segment->handle == mem_handle)
        {
            // Check if task was the task to allocate it
            if (current_segment->task_id != thread_id)
            {
                sem->up(thread_id);
                return -1;
            }

            // Reset current location to 0 if last character and return -1 for end of bounds
            if (current_segment->current >= current_segment->size)
            {
                current_segment->current = 0;
                sem->up(thread_id);
                return -1;
            }

            data[current_segment->start + current_segment->current] = ch;
            current_segment->current += 1;

            sem->up(thread_id);
            return 0;
        }
        current_segment = current_segment->next;
    }

    // Memory handle was not found in segments

    sem->up(thread_id);
    return -1;
}

int MMU::mem_write(int mem_handle, int offset_from_beg, std::string text, int thread_id)
{
    sem->down(thread_id);

    MemorySegment *current_segment = segments;
    // Find segment based on mem_handle
    while (current_segment != nullptr)
    {
        if (current_segment->handle == mem_handle)
        {
            // Check if task was the task to allocate it
            if (current_segment->task_id != thread_id)
            {
                sem->up(thread_id);
                return -1;
            }

            // Check if text will fit in the segment
            if (current_segment->size - offset_from_beg < text.size())
            {
                sem->up(thread_id);
                return -1;
            }

            // Write each character to the segment
            for (int i = 0; i < text.size(); i++)
            {
                data[current_segment->start + offset_from_beg + i] = text[i];
            }

            sem->up(thread_id);
            return 0;
        }
        current_segment = current_segment->next;
    }

    // Memory handle was not found in segments

    sem->up(thread_id);
    return -1;
}

std::string MMU::mem_dump()
{
    sem->down(0);

    std::stringstream str;
    str << " Status\tHandle\tStart\tEnd\tSize\tCurrent\tTask-ID";

    MemorySegment *current_segment = segments;

    while (current_segment != nullptr)
    {
        str << "\n "; // New line and extra padding for nCurses Window
        str << current_segment->get_status() << "\t";
        str << current_segment->handle << "\t";
        str << current_segment->start << "\t";
        str << current_segment->get_end() << "\t";
        str << current_segment->size << "\t";
        str << current_segment->get_current_location() << "\t";
        str << current_segment->get_task_id();
        current_segment = current_segment->next;
    }

    sem->up(0);
    return str.str();
}

std::string MMU::core_dump()
{
    sem->down(0);

    std::string output = "";

    for (int i = 0; i < size; i++)
    {
        output += data[i];
    }

    sem->up(0);
    return output;
}

std::string MMU::core_dump(int starting_from, int num_bytes)
{
    sem->down(0);

    if (size - starting_from < num_bytes)
    {
        sem->up(0);
        return "Invalid core memory range to read from.";
    }

    std::string output = "";

    for (int i = 0; i < num_bytes; i++)
    {
        output += data[starting_from + i];
    }

    sem->up(0);
    return output;
}

int MMU::mem_left()
{
    sem->down(0);

    MemorySegment *current_segment = segments;
    int total_unallocated = 0;

    while (current_segment != nullptr)
    {
        if (current_segment->get_status() == "Free")
            total_unallocated += current_segment->size;

        current_segment = current_segment->next;
    }

    sem->up(0);
    return total_unallocated;
}

int MMU::mem_largest()
{
    sem->down(0);

    MemorySegment *current_segment = segments;
    int largest_unallocated = -1;

    while (current_segment != nullptr)
    {
        if (current_segment->get_status() == "Free" && current_segment->size > largest_unallocated)
            largest_unallocated = current_segment->size;

        current_segment = current_segment->next;
    }

    sem->up(0);
    return largest_unallocated;
}

int MMU::mem_smallest()
{
    sem->down(0);

    MemorySegment *current_segment = segments;
    int smallest_unallocated = size + 1;

    while (current_segment != nullptr)
    {
        if (current_segment->get_status() == "Free" && current_segment->size < smallest_unallocated)
            smallest_unallocated = current_segment->size;

        current_segment = current_segment->next;
    }

    sem->up(0);

    // If still impossible size, no unallocated segments were found
    if (smallest_unallocated > size)
        return -1;

    return smallest_unallocated;
}

int MMU::mem_coalesce()
{
    sem->down(0);

    MemorySegment *segment1 = segments;

    // Do nothing if no segments for some reason
    if (segment1 == nullptr)
    {
        sem->up(0);
        return 0;
    }

    // Make sure the first segment gets reset to '.' if free
    if (segment1->get_status() == "Free")
    {
        for (int i = 0; i < segment1->size; i++)
        {
            data[segment1->start + i] = '.';
        }
    }

    // Do nothing else if only 1 segment is initialized
    if (segment1->next == nullptr)
    {
        sem->up(0);
        return 0;
    }

    MemorySegment *segment2 = segment1->next;

    // Loop through all segments
    while (segment2 != nullptr)
    {
        // Update any unallocated segment memory to '.' values
        if (segment2->get_status() == "Free")
        {
            for (int i = 0; i < segment2->size; i++)
            {
                data[segment2->start + i] = '.';
            }
        }

        // Combine adjacent unallocated segments
        if (segment1->get_status() == "Free" && segment2->get_status() == "Free")
        {
            segment1->size += segment2->size;
            segment1->next = segment2->next;

            delete segment2;

            // Modify increment logic and skip normal end of loop
            segment2 = segment1->next;
            continue;
        }
        segment1 = segment2;
        segment2 = segment2->next;
    }

    sem->up(0);
    return 0;
}
