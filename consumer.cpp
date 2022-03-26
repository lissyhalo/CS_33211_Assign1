/*
 * Melissa Halozan
 * Assignment 1 : consumer.cpp
 * CS OS
 */
////////////////////////////////////////////////////////////////////////////////
//  consumer.cpp
//   get items from shared buffer
//   sync with producer to maintain buffer level by using semaphores
//
//   use 3 named semaphores to control access to shared memory buffer
//     mutex: binary semaphore, lock access to shared memory, initial value = 1 unlocked
//     full: counting semaphore, number in buffer, initial value = 0 nothing in buffer
//     empty: counting semaphore, number of empty spots in buffer, initial value = 2 open locations in buffer
//     
//     consumer processing:
//     wait for item in buffer : wait if full semaphore = 0
//     wait for access to buffer : wait if mutex semephore = 0
//     reads an item from buffer in shared memory
//     output item data fields to stdout
//     increments empty semaphore to signal that spot is open in buffer
//     repeat (exit if exceed test MAX_TRANSFERS)
//
//     wait until data is in buffer before trying to access
//     this prevents possibility that producer did not create shared memory before consumer tries to access
//       (added this during debug, but problem may just have been wrong use of shm_unlink(name); in producer)
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include "share_def.hpp"

int main(int argc, char *argv[]) {
    
int item_num = 0;
const char *name = "OS";
int shm_fd = -1; // file descriptor for shared memory, set to -1 to force first open of shared memory
item *ptr_one;
sem_t *mutex, *full, *empty;
int sem_val;

if (argc > 1) {
    std::cerr << "Error: no args!!" << std::endl;
    std::cerr << std::endl << std::endl;
    return(1);
}

std::cout << "consumer start" << std::endl;

if( BUFFER_SIZE != 2) { // program is hardcoded to 2 items in buffer
    printf("consumer: BUFFER_SIZE != 2\n");
    exit(-1);
}

// open named semaphores - same names used by both processes    
mutex = sem_open("/semaphore_mutex", O_CREAT, S_IRWXU | S_IRWXG  | S_IRWXO, 1);
full = sem_open("/semaphore_full", O_CREAT, S_IRWXU | S_IRWXG  | S_IRWXO, 0);
empty = sem_open("/semaphore_empty", O_CREAT, S_IRWXU | S_IRWXG  | S_IRWXO, BUFFER_SIZE);
    if( mutex == NULL || full == NULL || empty == NULL) {
        printf("consumer unable to open sync semaphores\n");
        exit(-1);
    }
// immediately unlink to protect against lingering named semaphores after program crash
sem_unlink("/semaphore_mutex");
sem_unlink("/semaphore_full");
sem_unlink("/semaphore_empty");

std::cout << "consumer fetch start" << std::endl;
//while (true) { 
while (item_num < MAX_TRANSFERS) { 
    //std::cout << "consumer: while top " << std::endl;
    
    sem_wait(full);
    sem_wait(mutex);
    
    // open shared memory if not already open 
    // expect producer to set up shared memory
    if(shm_fd == -1) {
        shm_fd = shm_open(name, O_RDWR, 0666);
        if(shm_fd == -1) {
            printf ("consumer fail shm_open\n");
            printf("%s\n",strerror(errno));
            exit(-1);
        }
        ftruncate(shm_fd, BUFFER_SIZE * sizeof(item));
        ptr_one = (item *) mmap(0, BUFFER_SIZE * sizeof(item), PROT_READ, MAP_SHARED, shm_fd, 0);
        printf ("consumer: ptr_one %p\n", (void *) ptr_one);
        shm_unlink(name); // unlink here, but not at producer?
    }

    // get item from shared memory, alternate locations 0, 1
    // only output first 2 items per 10000 to reduce output
    if (((item_num % 10000) == 0) || ((item_num % 10000) == 1)) printf ("%s: %i\n", (char *) ptr_one[item_num%2].message, ptr_one[item_num%2].item_num);
    item_num = item_num + 1; 

    sem_post(mutex);
    sem_post(empty);
}


return 0;
}











#if 0
Do { 
wait(full); 
wait(mutex); 
...
/* remove an item from buffer to next_consumed */ 
... 
signal(mutex); 
signal(empty); 
...
/* consume the item in next consumed */ 
...
} while (true);
#endif



/*    x = sem_open("/semaphore_x", O_RDWR);
    y = sem_open("/semaphore_y", O_RDWR);

    if( x == NULL || y == NULL) {
        printf("Unable to open named semaphores\n");
        exit(-1);
    }*/

#if 0
    // get named semaphores, wait until prodcuer creates them
//    while ((x = sem_open("/semaphore_x", O_RDWR)) == SEM_FAILED)
//        std::cout << "consumer semaphore x: " << x << std::endl;
    while ((x = sem_open("/semaphore_x", O_RDWR)) == SEM_FAILED) {
        printf("x");
 //       sleep(1);
    }
    std::cout << "consumer semaphore x: " << x << std::endl;

    if ((y = sem_open("/semaphore_y", O_RDWR)) == SEM_FAILED) {
        printf("y");
        std::cout << "consumer semaphore y: " << y << std::endl;
    }

    if( x == NULL || y == NULL) { /// should never get here
        printf("consumer unable to open named semaphores\n");
        exit(-1);
    }
#endif    
