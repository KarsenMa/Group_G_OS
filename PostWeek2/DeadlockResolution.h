/*  
    Group G
    Author Name: Eric Vo
    Email: eric.t.vo@okstate.edu
    Date: 17 April 2025
*/

#ifndef DEADLOCK_RESOLUTION_H
#define DEADLOCK_RESOLUTION_H

#include "shared_Mem.h"
#include "Resource_Allocation.h"
#include <vector>
#include <string>

// calls when a deadlock is detected to forcibly release a held intersection
void resolveDeadlock(shared_mem_t* shm, const std::vector<Intersection>& intersections, const char* trainToPreempt, const char* intersectionToRelease);

#endif // DEADLOCKRESOLUTION_H
