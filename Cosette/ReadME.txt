This file is written in C++. It is not usable without being integrated with the other files.
Please use main.cpp to run the program. 


this file requires that the compilation statement include 
gcc shared_Mem.cpp -pthread -lrt 

to use the test file compile with
gcc shared_Mem.cpp test.cpp -pthread -lrt -o sharedMemTest