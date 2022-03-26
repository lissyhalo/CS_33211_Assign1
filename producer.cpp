/*
 * Melissa Halozan
 * Assignment 1 : producer.cpp
 * CS OS
 */
////////////////////////////////////////////////////////////////////////////////
//  producer.cpp
//   produce items and put into buffer in shared memory
//   sync with consumer to maintain buffer level by using semaphores
//
//   use 3 named semaphores to control access to shared memory buffer
//     mutex: binary semaphore, lock access to shared memory, initial value = 1 unlocked
//     full: counting semaphore, number in buffer, initial value = 0 nothing in buffer
//     empty: counting semaphore, number of empty spots in buffer, initial value = 2 open locations in buffer
//     
//     producer processing:
//     create an item that can be transferred
//     wait for empty spot in shared memory : wait if empty semaphore = 0
//     wait for access to buffer : wait if mutex semephore = 0
//     producer writes an item to buffer in shared memory
//     increments full semaphore to signal that new item is in buffer
//     repeat (exit if exceed test MAX_TRANSFERS)
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
    
item next_produced; 
int item_num = 0;
const char *name = "OS";
int shm_fd;
item *ptr_one;
item temp_item;
sem_t *mutex, *full, *empty;
int sem_val;

if (argc > 1) {
    std::cerr << "Error: no args!!" << std::endl;
    std::cerr << std::endl << std::endl;
    return(1);
}
    
std::cout << "producer start" << std::endl;

if( BUFFER_SIZE != 2) { // hardcoded to 2 items in buffer
    printf("producer: BUFFER_SIZE != 2\n");
    exit(-1);
}

// open shared memory
shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
ftruncate(shm_fd, BUFFER_SIZE * sizeof(item));
ptr_one = (item *) mmap(0, BUFFER_SIZE * sizeof(item), PROT_WRITE, MAP_SHARED, shm_fd, 0);
printf ("producer: ptr_one %p\n", (void *) ptr_one);
//shm_unlink(name);

// open named semaphores - same names used by both processes
mutex = sem_open("/semaphore_mutex", O_CREAT, S_IRWXU | S_IRWXG  | S_IRWXO, 1);
full = sem_open("/semaphore_full", O_CREAT, S_IRWXU | S_IRWXG  | S_IRWXO, 0);
empty = sem_open("/semaphore_empty", O_CREAT, S_IRWXU | S_IRWXG  | S_IRWXO, BUFFER_SIZE);
    if( mutex == NULL || full == NULL || empty == NULL) {
        printf("producer unable to open sync semaphores\n");
        exit(-1);
    }
// immediately unlink to protect against lingering named semaphores after program crash
sem_unlink("/semaphore_mutex");
sem_unlink("/semaphore_full");
sem_unlink("/semaphore_empty");

std::cout << "producer create/transfer start" << std::endl;
//while (true) { 
while (item_num < MAX_TRANSFERS) { 
    //std::cout << "producer: while top " << std::endl;

    // create item - alternate locations 0, 1
    if (item_num%2 == 0) {
        strcpy(temp_item.message, "tick");
    }
    else {
        strcpy(temp_item.message, "tock");
    }
    temp_item.item_num = item_num;
    
    sem_wait(empty);
    sem_wait(mutex);
    
    // write item to shared memory, alternate locations 0, 1
    strcpy(ptr_one[item_num%2].message, temp_item.message);
    ptr_one[item_num%2].item_num = temp_item.item_num;
    //std::cout << "producer: wrote " << item_num << std::endl;
    item_num = item_num + 1; 

    sem_post(mutex);
    sem_post(full);

}
    
return 0;
}
