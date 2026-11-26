#ifndef QUEUE_H
#define QUEUE_H

// Configurable capacities for the pending queue and completed history
#ifndef QUEUE_CAPACITY
#define QUEUE_CAPACITY 50
#endif

#ifndef HISTORY_CAPACITY
#define HISTORY_CAPACITY 50
#endif

// Field and input limits
#define NAME_LENGTH 30
#define DOCUMENT_LENGTH 60
#define INPUT_BUFFER_SIZE 128
#define MAX_PAGES 500

// Status codes returned by circular queue operations
typedef enum {
    QUEUE_OK,
    QUEUE_EMPTY,
    QUEUE_FULL,
    QUEUE_NOT_FOUND,
    QUEUE_INVALID
} QueueStatus;

// Represents a single print job in the system
typedef struct {
    unsigned int jobId;
    char userName[NAME_LENGTH + 1];
    char documentName[DOCUMENT_LENGTH + 1];
    int pages;
} PrintJob;

// Circular queue backed by a fixed-size array and an element counter
typedef struct {
    PrintJob jobs[QUEUE_CAPACITY];
    int front;
    int rear;
    int count;
} Queue;

// Initializes an empty circular queue
void initQueue(Queue *queue);

// Inserts a print job at the rear of the queue in O(1) time
QueueStatus enqueue(Queue *queue, const PrintJob *job);

// Removes the front print job from the queue in O(1) time
QueueStatus dequeue(Queue *queue, PrintJob *job);

// Removes a pending job by ID while preserving FIFO order in O(n) time
QueueStatus queueRemoveById(Queue *queue, unsigned int jobId, PrintJob *job);

// Returns the current number of jobs in the queue in O(1) time
int queueSize(const Queue *queue);

// Reads the job at logical offset (0 = front) without modifying the queue
QueueStatus queueGetAt(const Queue *queue, int offset, PrintJob *job);

// Returns 1 if the queue is empty, 0 otherwise
int isEmpty(const Queue *queue);

// Returns 1 if the queue is full, 0 otherwise
int isFull(const Queue *queue);

// Prints all pending jobs from front to rear without modifying the queue
void displayQueue(const Queue *queue);

#endif
