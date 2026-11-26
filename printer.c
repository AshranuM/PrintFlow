#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "printer.h"

// Checks the conservation invariant: submitted == completed + cancelled + pending
static void verifyInvariant(const PrinterSystem *system) {
    assert(system != NULL);
    assert(system->totalSubmitted ==
           system->totalCompleted +
           system->totalCancelled +
           (unsigned int)queueSize(&system->pendingQueue));
}

// Appends a completed job to the circular history buffer, overwriting oldest if full
static void recordCompletedHistory(PrinterSystem *system, const PrintJob *job) {
    if (HISTORY_CAPACITY <= 0) {
        return;
    }
    if (system->historyCount == HISTORY_CAPACITY) {
        // History is full: advance front to overwrite the oldest completed record
        system->historyFront = (system->historyFront + 1) % HISTORY_CAPACITY;
        system->historyRear = (system->historyRear + 1) % HISTORY_CAPACITY;
        system->history[system->historyRear] = *job;
    } else {
        system->historyRear = (system->historyRear + 1) % HISTORY_CAPACITY;
        system->history[system->historyRear] = *job;
        system->historyCount++;
    }
}

// Checks whether a job ID is currently recorded in the completed history buffer
static int isJobInHistory(const PrinterSystem *system, unsigned int jobId) {
    int offset;

    for (offset = 0; offset < system->historyCount; offset++) {
        int index = (system->historyFront + offset) % HISTORY_CAPACITY;
        if (system->history[index].jobId == jobId) {
            return 1;
        }
    }
    return 0;
}

int isValidTextField(const char *text, size_t maxLength) {
    size_t i;
    size_t len;

    if (text == NULL) {
        return 0;
    }
    len = strlen(text);
    if (len == 0 || len > maxLength) {
        return 0;
    }
    // Allow internal spaces, but reject control characters such as tabs or escapes
    for (i = 0; i < len; i++) {
        if (iscntrl((unsigned char)text[i])) {
            return 0;
        }
    }
    return 1;
}

void initPrinterSystem(PrinterSystem *system) {
    if (system == NULL) {
        return;
    }
    initQueue(&system->pendingQueue);
    system->historyFront = 0;
    system->historyRear = HISTORY_CAPACITY - 1;
    system->historyCount = 0;
    system->nextJobId = 1;
    system->totalSubmitted = 0;
    system->totalCompleted = 0;
    system->totalCancelled = 0;
    system->totalPagesPrinted = 0;
    system->status = PRINTER_IDLE;
    verifyInvariant(system);
}

QueueStatus printerAddJob(PrinterSystem *system,
                          const char *userName,
                          const char *documentName,
                          int pages,
                          unsigned int *assignedJobId) {
    PrintJob newJob;
    QueueStatus status;

    if (system == NULL) {
        return QUEUE_INVALID;
    }
    // Validate every input field and capacity before changing any state
    if (!isValidTextField(userName, NAME_LENGTH) ||
        !isValidTextField(documentName, DOCUMENT_LENGTH) ||
        pages < 1 || pages > MAX_PAGES) {
        return QUEUE_INVALID;
    }
    if (isFull(&system->pendingQueue)) {
        return QUEUE_FULL;
    }

    newJob.jobId = system->nextJobId;
    strncpy(newJob.userName, userName, NAME_LENGTH);
    newJob.userName[NAME_LENGTH] = '\0';
    strncpy(newJob.documentName, documentName, DOCUMENT_LENGTH);
    newJob.documentName[DOCUMENT_LENGTH] = '\0';
    newJob.pages = pages;

    status = enqueue(&system->pendingQueue, &newJob);
    if (status != QUEUE_OK) {
        return status;
    }

    // Advance ID and counters only after enqueue succeeds
    system->nextJobId++;
    system->totalSubmitted++;
    if (assignedJobId != NULL) {
        *assignedJobId = newJob.jobId;
    }
    verifyInvariant(system);
    return QUEUE_OK;
}

QueueStatus printerProcessNextJob(PrinterSystem *system) {
    PrintJob job;
    QueueStatus status;

    if (system == NULL) {
        return QUEUE_INVALID;
    }
    if (isEmpty(&system->pendingQueue)) {
        printf("[INFO] No pending jobs in the queue. Printer remains IDLE.\n");
        return QUEUE_EMPTY;
    }

    // Mark printer BUSY while simulating processing of the front job
    system->status = PRINTER_BUSY;
    printf("[INFO] Printer status: BUSY - Processing next job...\n");

    status = dequeue(&system->pendingQueue, &job);
    if (status != QUEUE_OK) {
        system->status = PRINTER_IDLE;
        return status;
    }

    printf("[OK] Printed Job #%u | User: %s | Document: %s | Pages: %d\n",
           job.jobId, job.userName, job.documentName, job.pages);

    recordCompletedHistory(system, &job);
    system->totalCompleted++;
    system->totalPagesPrinted += (unsigned long)job.pages;
    system->status = PRINTER_IDLE;

    verifyInvariant(system);
    printf("[INFO] Job #%u moved to completed history. Printer is now IDLE.\n",
           job.jobId);
    return QUEUE_OK;
}

void printerDisplayPendingJobs(const PrinterSystem *system) {
    if (system == NULL) {
        printf("[ERROR] Printer system is unavailable.\n");
        return;
    }
    displayQueue(&system->pendingQueue);
}

QueueStatus printerCancelJob(PrinterSystem *system, unsigned int jobId) {
    PrintJob removedJob;
    QueueStatus status;

    if (system == NULL || jobId == 0) {
        return QUEUE_INVALID;
    }

    if (jobId >= system->nextJobId) {
        printf("[ERROR] Job #%u does not exist.\n", jobId);
        return QUEUE_NOT_FOUND;
    }

    status = queueRemoveById(&system->pendingQueue, jobId, &removedJob);
    if (status == QUEUE_OK) {
        system->totalCancelled++;
        verifyInvariant(system);
        printf("[OK] Cancelled Job #%u (%s - %s, %d pages).\n",
               removedJob.jobId,
               removedJob.userName,
               removedJob.documentName,
               removedJob.pages);
        return QUEUE_OK;
    }

    // Job ID was issued previously, so it was already completed or cancelled
    if (isJobInHistory(system, jobId)) {
        printf("[INFO] Job #%u has already been completed and cannot be cancelled.\n",
               jobId);
    } else {
        printf("[INFO] Job #%u is no longer pending (already completed or cancelled).\n",
               jobId);
    }
    return QUEUE_NOT_FOUND;
}

void printerDisplayHistory(const PrinterSystem *system) {
    int offset;

    if (system == NULL) {
        printf("[ERROR] Printer system is unavailable.\n");
        return;
    }
    if (system->historyCount == 0) {
        printf("[INFO] No completed print jobs yet.\n");
        return;
    }

    printf("\nCompleted Job History:\n");
    printf("[INFO] Showing last %d of %u completed jobs.\n",
           system->historyCount, system->totalCompleted);
    printf("%-6s %-20s %-30s %-6s\n", "ID", "User", "Document", "Pages");
    printf("------------------------------------------------------------------\n");

    // Traverse circular history buffer from oldest (historyFront) to newest
    for (offset = 0; offset < system->historyCount; offset++) {
        int index = (system->historyFront + offset) % HISTORY_CAPACITY;
        const PrintJob *job = &system->history[index];
        printf("%-6u %-20s %-30s %-6d\n",
               job->jobId, job->userName, job->documentName, job->pages);
    }
}

void printerDisplayStatus(const PrinterSystem *system) {
    const char *statusText;
    int pending;

    if (system == NULL) {
        printf("[ERROR] Printer system is unavailable.\n");
        return;
    }
    statusText = (system->status == PRINTER_BUSY) ? "BUSY" : "IDLE";
    pending = queueSize(&system->pendingQueue);

    printf("\nPrinter Status:\n");
    printf("----------------------------------------\n");
    printf("Printer State     : %s\n", statusText);
    printf("Pending Jobs      : %d / %d\n", pending, QUEUE_CAPACITY);
    printf("Simulation Status : %s\n",
           pending > 0 ? "Ready to process pending jobs" : "Waiting for new print jobs");
    printf("[INFO] Note: Printing is simulated; no physical printer is used.\n");
}

void printerDisplayStatistics(const PrinterSystem *system) {
    int pending;

    if (system == NULL) {
        printf("[ERROR] Printer system is unavailable.\n");
        return;
    }
    verifyInvariant(system);
    pending = queueSize(&system->pendingQueue);

    printf("\nPrinter Statistics:\n");
    printf("----------------------------------------\n");
    printf("Total Submitted Jobs : %u\n", system->totalSubmitted);
    printf("Total Completed Jobs : %u\n", system->totalCompleted);
    printf("Total Cancelled Jobs : %u\n", system->totalCancelled);
    printf("Pending Jobs         : %d\n", pending);
    printf("Total Pages Printed  : %lu\n", system->totalPagesPrinted);
}

int printerIsQueueFull(const PrinterSystem *system) {
    if (system == NULL) {
        return 0;
    }
    return isFull(&system->pendingQueue);
}
