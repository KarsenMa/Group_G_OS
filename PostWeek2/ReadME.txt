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
g++ shared_Mem.cpp DeadlockDetection.cpp Resource_Allocation.cpp sync.cpp TrainCommunication.cpp main.cpp -pthread -lrt -o RailwaySim



Files written by 
Clayton: 
- TrainCommunication.cpp
- DeadlockDetection.cpp

Cosette: 
- shared_Mem.cpp
- shared_Mem.h
- main.cpp
- sync.cpp
- sync.h

Damian: 
- MutexThread.cpp
- MutexThread.h
- IPCSemaphore.cpp
- IPCSemaphore.h

Eric: 
- trainFiles.cpp

Karsen: 
- Forking_Trains.cpp

Reid:
- 

