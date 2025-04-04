/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/3/2025
    Program Description: This file contains methods to create a shared memory to be used to hold
    mutex and semaphore states.
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

class shared_Mem { 
    public: 
        void mem_setup();
        void mem_close(int fd, unsigned int length, const char* name);

        
    
    
}


