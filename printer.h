#ifndef PRINTER_H
#define PRINTER_H

#include "queue.h"

// Simulated printer operating states
typedef enum {
    PRINTER_IDLE,
    PRINTER_BUSY
} PrinterStatus;

// Holds all printer state, queues, history, and statistics counters
typedef struct {
    Queue pendingQueue;
    PrintJob history[HISTORY_CAPACITY];
    int historyFront;
    int historyRear;
    int historyCount;
    unsigned int nextJobId;
    unsigned int totalSubmitted;
    unsigned int totalCompleted;
    unsigned int totalCancelled;
    unsigned long totalPagesPrinted;
    PrinterStatus status;
} PrinterSystem;

// Initializes all queues, buffers, counters, and printer status
void initPrinterSystem(PrinterSystem *system);

// Validates job fields and adds a new job to the pending queue
QueueStatus printerAddJob(PrinterSystem *system,
                          const char *userName,
                          const char *documentName,
                          int pages,
                          unsigned int *assignedJobId);

// Processes the next pending job in FIFO order and records it in history
QueueStatus printerProcessNextJob(PrinterSystem *system);

// Displays all pending jobs in FIFO order
void printerDisplayPendingJobs(const PrinterSystem *system);

// Cancels a pending job by ID without disturbing the order of remaining jobs
QueueStatus printerCancelJob(PrinterSystem *system, unsigned int jobId);

// Displays completed jobs stored in the circular history buffer
void printerDisplayHistory(const PrinterSystem *system);

// Displays current simulated printer status and queue summary
void printerDisplayStatus(const PrinterSystem *system);

// Displays cumulative job and page statistics
void printerDisplayStatistics(const PrinterSystem *system);

// Validates a user or document text field against length and control rules
int isValidTextField(const char *text, size_t maxLength);

// Returns 1 if the pending print queue is full, 0 otherwise
int printerIsQueueFull(const PrinterSystem *system);

#endif
