Description of program and overview of functions:


Parent Process:
The parent process begins in the main function. It acts as the server for
the message queues and the setup process for the shared memory and resource 
allocation table. The parent process creates a child process for each train
in the input file. 

Child Process: 
The child process is the train, and holds the train ID and the information
about the path the train will take. The child process uses the message queue
to acquire and release semaphore and mutex locks. 




To compile: 
g++ shared_Mem.cpp DeadlockDetection.cpp DeadlockResolution.cpp Resource_Allocation.cpp sync.cpp TrainCommunication.cpp trainCommExtension.cpp main.cpp -pthread -lrt -o RailwaySim

Best practice during testing: 
Before closing your session on csx server, check to make sure no shared memory objects are leftover
from aborted processes. 
Run 

ls -l /dev/shm/sharedMemory3
If sharedMemory3 exists and is owned by you, then remove it.
rm /dev/shm/sharedMemory3

This will ensure you do not have further issues with the shared memory.



Files written by 

Clayton: 
- TrainCommunication.cpp
- DeadlockDetection.cpp
- DeadlockDetection.h

Cosette: 
- shared_Mem.cpp
- shared_Mem.h
- sync.cpp
- sync.h
- trainCommExtension.cpp
- trainCommExtension.h

Damian: 
- MutexThread.cpp
- MutexThread.h
- IPCSemaphore.cpp
- IPCSemaphore.h
- TrainCommunication.h

Eric: 
- trainFiles.cpp
- DeadlockResolution.cpp
- DeadlockResolution.h

Karsen: 
- Forking_Trains.cpp
- Resource_Allocation.cpp
- Resource_Allocation.h
- Edge_Case/intersections.txt
- Edge_Case/trains.txt

Reid:
- main.cpp
- easy/intersections.txt
- easy/trains.txt
- intermediate/intersections.txt
- intermediate/trains.txt
- hard/intersections.txt
- hard/trains.txt

