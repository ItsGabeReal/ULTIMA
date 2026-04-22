#include "MMU.h"

MMU::MMU(int size, char default_initial_value, int page_size)
{
    if (page_size > size) throw "'size' cannot be less than 'page_size'.";

    if (size % page_size != 0) throw "'page_size' must evenly divide 'size'.";

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
}

MMU::~MMU()
{
}

int MMU::mem_alloc(int size, int thread_id)
{
    MemorySegment* current_segment = segments;
    // Check if task already has allocated memory
    while (current_segment != nullptr)  
    {
        if (current_segment->task_id == thread_id) return -1;
        current_segment = current_segment->next;
    }

    // Task was found to not have any allocated memory

    // Reset current_segment pointer
    current_segment = segments;

    int allocate_size = ceil((float)size/page_size) * page_size;
    

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
            MemorySegment* new_segment = new MemorySegment();
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
        return current_segment->handle;
    }

    // No segments were found to be free and large enough
    return -1;
}

int MMU::mem_free(int mem_handle, int thread_id)
{
    return 0;
}

int MMU::mem_read(int mem_handle, char *ch, int thread_id)
{
    return 0;
}

int MMU::mem_read(int mem_handle, int offset_from_beg, int text_size, std::string *text, int thread_id)
{
    return 0;
}

int MMU::mem_write(int mem_handle, char ch, int thread_id)
{
    return 0;
}

int MMU::mem_write(int mem_handle, int offset_from_beg, std::string text, int thread_id)
{
    return 0;
}

std::string MMU::mem_dump()
{
    std::stringstream str;
    str << " Status\tHandle\tStart\tEnd\tSize\tCurrent\tTask-ID";

    MemorySegment* current_segment = segments;

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

    return str.str();
}

std::string MMU::core_dump()
{
    std::string output = "";

    for (int i = 0; i < size; i++)
    {
        output += data[i];
    }

    return output;
}

std::string MMU::core_dump(int starting_from, int num_bytes)
{
    return std::string();
}

int MMU::mem_left()
{
    return 0;
}

int MMU::mem_largest()
{
    return 0;
}

int MMU::mem_smallest()
{
    return 0;
}

int MMU::mem_coalesce()
{
    return 0;
}
