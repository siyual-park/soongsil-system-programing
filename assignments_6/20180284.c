#define BUFFER_SIZE 4096
#define LINE_SIZE 100

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "20180284.h"

// An example of a semphore for the mutual exclusion
semaphore_t mutex;
FILE *file;

char buffer[BUFFER_SIZE] = { 0 };
int allocated_size = 0;

void reader(void)
{
    while (1) {
        P(&mutex);
        if (feof(file)) {
            V(&mutex);
            return;
        }

        char *line = malloc(LINE_SIZE);
        memset(line, 0, LINE_SIZE);

        if (fgets(line, LINE_SIZE, file) == NULL) {
            free(line);
            V(&mutex);
            return;
        }
        int line_size = strlen(line);
        memcpy(buffer + allocated_size, line, line_size);
        printf("Thread %d is reading a line\n", get_thread_id());
        allocated_size += line_size;

        free(line);

        V(&mutex);
    }
}

void writer(void)
{
    while (1) {
        P(&mutex);
        if (allocated_size == 0) {
            V(&mutex);
            return;
        }

        printf("Thread %d is writing a lines\n", get_thread_id());
        printf("%s", buffer);
        memset(buffer, 0, BUFFER_SIZE);
        allocated_size = 0;

        V(&mutex);
    }
}

int main(int argc, char *argv[])
{
	// example of initializing a semaphore
    mutex = sem_create(1);

    file = fopen(argv[1], "r+");
    assert(file != NULL);

    // create threads
    create_thread(reader);
    create_thread(reader);
    create_thread(reader);
    create_thread(writer);
    
    // Let's run
    run();

    fclose(file);
}   




