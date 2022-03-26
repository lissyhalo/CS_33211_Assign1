#ifndef INCLUDES_SHAREDEF_H_
#define INCLUDES_SHAREDEF_H_

#define BUFFER_SIZE 2
#define ITEM_SIZE 80
#define MAX_TRANSFERS 1000000

typedef struct {
    char message[ITEM_SIZE];
    int item_num;
} item;

#endif

