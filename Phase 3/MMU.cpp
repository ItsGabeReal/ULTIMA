#include "MMU.h"

MMU::MMU(int size, char default_initial_value, int page_size)
{
}

MMU::~MMU()
{
}

int MMU::mem_alloc(int size, int thread_id)
{
    return 0;
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
    return std::string();
}

std::string MMU::core_dump()
{
    return std::string();
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
