#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "phonebook_opt.h"
#include "debug.h"

entry *findName(char lastname[], entry *pHead)
{
    size_t len = strlen(lastname);
    while (pHead != NULL) {
        if (strncasecmp(lastname, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            pHead->lastName[len] = '\0';
            if (pHead->dtl == NULL)
                pHead->dtl = (pdetail) malloc(sizeof(detail));
            return pHead;
        }
        DEBUG_LOG("find string = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
    return NULL;
}

thread_arg *createThead_arg(char *data_begin, char *data_end,
                            int threadID, int numOfThread,
                            entry *entryPool)
{
    thread_arg *new_arg = (thread_arg *) malloc(sizeof(thread_arg));

    new_arg->data_begin = data_begin;
    new_arg->data_end = data_end;
    new_arg->threadID = threadID;
    new_arg->numOfThread = numOfThread;
    new_arg->lEntryPool_begin = entryPool;
    new_arg->lEntry_head = new_arg->lEntry_tail = entryPool;
    return new_arg;
}

/**
 * Generate a local linked list in thread.
 */
void append(void *arg)
{
    struct timespec start, end;
    double cpu_time;
    int count = 0;

    clock_gettime(CLOCK_REALTIME, &start);

    thread_arg *t_arg = (thread_arg *) arg;

    entry *cur_entry = t_arg->lEntryPool_begin;
    // Put the first element to the first entry
    char *cur_data = t_arg->data_begin;
    cur_entry->lastName = cur_data;
    cur_entry->pNext    = NULL;
    cur_entry->dtl      = NULL;
    ++cur_entry;
    // Start appending the entry from the second one.
    for (cur_data += MAX_LAST_NAME_SIZE;
            cur_data < t_arg->data_end;
            ++cur_entry, cur_data += MAX_LAST_NAME_SIZE, ++count) {
        cur_entry->lastName = cur_data;
        cur_entry->pNext    = NULL;
        cur_entry->dtl      = NULL;
        // Append the new at the end of the local linked list.
        t_arg->lEntry_tail->pNext = cur_entry;
        t_arg->lEntry_tail = t_arg->lEntry_tail->pNext;
        DEBUG_LOG("thread %d t_argend string = %s\n",
                  t_arg->threadID, t_arg->lEntry_tail->lastName);
    }
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time = diff_in_second(start, end);

    DEBUG_LOG("thread take %lf sec, count %d\n", cpu_time, count);

    pthread_exit(NULL);
}

void show_entry(entry *pHead)
{
    while (pHead != NULL) {
        printf("%s", pHead->lastName);
        pHead = pHead->pNext;
    }
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}
