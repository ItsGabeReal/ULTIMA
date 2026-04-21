#include <string>
#include <iostream>
#include <sstream>

class MMU
{
private:
    unsigned char data[1024];
    int size;
    int page_size;
public:
    MMU(int size, char default_initial_value, int page_size);
    ~MMU();

    /**
     * Returns the memory handle of the newly allocated memory block, or -1
     * if there is no memory available, or the task already has memory
     * allocated.
     */
    int mem_alloc(int size, int task_id);

    /**
     * Frees memory at given mem_handle.
     * 
     * Returns -1 if the memory handle is invalid, or is not owned by task.
     */
    int mem_free(int mem_handle, int task_id);

    int mem_read(int mem_handle, char *ch, int task_id);
    int mem_read(int mem_handle, int offset_from_beg, int text_size, std::string *text, int task_id);
    int mem_write(int mem_handle, char ch, int task_id);
    int mem_write(int mem_handle, int offset_from_beg, std::string text, int task_id);
    std::string mem_dump(); // Memory information, like 
    
    /**
     * Dumps the memory contents.
     */
    std::string core_dump();

    /**
     * Dumps contents of a certain memory region.
     */
    std::string core_dump(int starting_from, int num_bytes);

private:
    int mem_left();
    int mem_largest();
    int mem_smallest();
    int mem_coalesce();
};
