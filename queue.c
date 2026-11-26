#include <stdio.h>
#include "queue.h"

// Converts a logical offset from front (0 .. count-1) to a physical array index
static int toPhysicalIndex(const Queue *queue, int offset) {
    return (queue->front + offset) % QUEUE_CAPACITY;
}

void initQueue(Queue *queue) {
    if (queue == NULL) {
        return;
    }
    queue->front = 0;
    // Setting rear to the last slot ensures first enqueue lands at index 0
    queue->rear = QUEUE_CAPACITY - 1;
    queue->count = 0;
}

int isEmpty(const Queue *queue) {
    if (queue == NULL) {
        return 1;
    }
    return queue->count == 0;
}

int isFull(const Queue *queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->count == QUEUE_CAPACITY;
}

int queueSize(const Queue *queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->count;
}

QueueStatus enqueue(Queue *queue, const PrintJob *job) {
    if (queue == NULL || job == NULL) {
        return QUEUE_INVALID;
    }
    // Reject enqueue when full to avoid overwriting pending jobs
    if (isFull(queue)) {
        return QUEUE_FULL;
    }
    // Wrap rear around the fixed-size array using modulo arithmetic
    queue->rear = (queue->rear + 1) % QUEUE_CAPACITY;
    queue->jobs[queue->rear] = *job;
    queue->count++;
    return QUEUE_OK;
}

QueueStatus dequeue(Queue *queue, PrintJob *job) {
    if (queue == NULL) {
        return QUEUE_INVALID;
    }
    if (isEmpty(queue)) {
        return QUEUE_EMPTY;
    }
    if (job != NULL) {
        *job = queue->jobs[queue->front];
    }
    // Advance front circularly and decrement element count
    queue->front = (queue->front + 1) % QUEUE_CAPACITY;
    queue->count--;
    return QUEUE_OK;
}

QueueStatus queueGetAt(const Queue *queue, int offset, PrintJob *job) {
    int index;

    if (queue == NULL || job == NULL) {
        return QUEUE_INVALID;
    }
    if (isEmpty(queue)) {
        return QUEUE_EMPTY;
    }
    if (offset < 0 || offset >= queue->count) {
        return QUEUE_INVALID;
    }
    index = toPhysicalIndex(queue, offset);
    *job = queue->jobs[index];
    return QUEUE_OK;
}

QueueStatus queueRemoveById(Queue *queue, unsigned int jobId, PrintJob *job) {
    int offset;
    int foundOffset = -1;

    if (queue == NULL) {
        return QUEUE_INVALID;
    }
    if (isEmpty(queue)) {
        return QUEUE_EMPTY;
    }

    // Step 1: Search in logical FIFO order (0 to count - 1)
    for (offset = 0; offset < queue->count; offset++) {
        int pos = toPhysicalIndex(queue, offset);
        if (queue->jobs[pos].jobId == jobId) {
            foundOffset = offset;
            if (job != NULL) {
                *job = queue->jobs[pos];
            }
            break;
        }
    }

    if (foundOffset == -1) {
        return QUEUE_NOT_FOUND;
    }

    // Step 2: Shift subsequent jobs one logical step toward front
    for (offset = foundOffset; offset < queue->count - 1; offset++) {
        int destPos = toPhysicalIndex(queue, offset);
        int srcPos = toPhysicalIndex(queue, offset + 1);
        queue->jobs[destPos] = queue->jobs[srcPos];
    }

    // Step 3: Decrease count and recompute rear safely (handles count == 0 too)
    queue->count--;
    queue->rear = (queue->front + queue->count + QUEUE_CAPACITY - 1) % QUEUE_CAPACITY;
    return QUEUE_OK;
}

void displayQueue(const Queue *queue) {
    int offset;

    if (queue == NULL) {
        printf("[ERROR] Queue is unavailable.\n");
        return;
    }
    if (isEmpty(queue)) {
        printf("[INFO] No pending print jobs in the queue.\n");
        return;
    }

    printf("\nPending Print Jobs (%d/%d):\n", queue->count, QUEUE_CAPACITY);
    printf("%-6s %-20s %-30s %-6s\n", "ID", "User", "Document", "Pages");
    printf("------------------------------------------------------------------\n");

    // Traverse using logical offsets so wrap-around is invisible to the user
    for (offset = 0; offset < queue->count; offset++) {
        int pos = toPhysicalIndex(queue, offset);
        const PrintJob *job = &queue->jobs[pos];
        printf("%-6u %-20s %-30s %-6d\n",
               job->jobId, job->userName, job->documentName, job->pages);
    }
}
