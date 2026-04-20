#include <string>

class MMU
{
private:
    /* data */
public:
    MMU(int size, char default_initial_value, int page_size);
    ~MMU();

    int mem_alloc(int size, int thread_id);
    int mem_free(int mem_handle, int thread_id);
    int mem_read(int mem_handle, char *ch, int thread_id);
    int mem_read(int mem_handle, int offset_from_beg, int text_size, char*text, int thread_id);
    int mem_write(int mem_handle, char ch, int thread_id);
    int mem_write(int mem_handle, int offset_from_beg, int text_size, char*text, int thread_id);
    std::string mem_dump(int starting_from, int num_bytes); // Memory information, like 
    
    /**
     * Dumps the memory contents, with one page per line.
     * 
     * Example output where page_size=8 and size=32:
     * 
     * "........\n
     *  ........\n
     *  ........\n
     *  ........"
     */
    std::string core_dump(int starting_from, int num_bytes);

private:
    int mem_left();
    int mem_largest();
    int mem_smallest();
    int mem_coalesce();
};
