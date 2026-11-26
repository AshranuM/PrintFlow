#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "printer.h"

// Status codes for shared line-reading helper
typedef enum {
    INPUT_OK,
    INPUT_TOO_LONG,
    INPUT_EOF
} InputStatus;

// Trims leading and trailing whitespace using unsigned char for ctype safety
static void trimWhitespace(char *str) {
    size_t len;
    size_t start = 0;

    if (str == NULL) {
        return;
    }
    len = strlen(str);
    while (start < len && isspace((unsigned char)str[start])) {
        start++;
    }
    while (len > start && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    if (start > 0 && len > start) {
        memmove(str, str + start, len - start);
    }
    str[len - start] = '\0';
}

// Reads one trimmed line safely using fgets without truncating overlong lines
static InputStatus readLine(char *buffer, size_t size) {
    size_t len;

    if (buffer == NULL || size == 0) {
        return INPUT_EOF;
    }
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return INPUT_EOF;
    }

    if (strchr(buffer, '\n') == NULL) {
        int ch = getchar();
        if (ch != '\n' && ch != EOF) {
            // Flush the rest of the overlong line so next read starts clean
            while (ch != '\n' && ch != EOF) {
                ch = getchar();
            }
            buffer[0] = '\0';
            return INPUT_TOO_LONG;
        }
    }

    // Strip any trailing newline or carriage return, then trim whitespace
    len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
        buffer[--len] = '\0';
    }
    trimWhitespace(buffer);
    return INPUT_OK;
}

// Parses a strictly formatted integer in [minVal, maxVal] using strtol
static int parseLongInRange(const char *str, long minVal, long maxVal, long *outVal) {
    char *endptr = NULL;
    long value;

    if (str == NULL || *str == '\0') {
        return 0;
    }
    errno = 0;
    value = strtol(str, &endptr, 10);
    if (errno == ERANGE || endptr == str || *endptr != '\0') {
        return 0;
    }
    if (value < minVal || value > maxVal) {
        return 0;
    }
    *outVal = value;
    return 1;
}

// Prints the main application menu
static void displayMenu(void) {
    printf("\n========================================\n");
    printf("       PRINTFLOW - PRINTER QUEUE\n");
    printf("========================================\n\n");
    printf("1. Add Print Job\n");
    printf("2. Process Next Job\n");
    printf("3. Display Pending Jobs\n");
    printf("4. Cancel Print Job\n");
    printf("5. View Completed History\n");
    printf("6. Printer Status\n");
    printf("7. Statistics\n");
    printf("0. Exit\n\n");
    printf("Enter your choice: ");
}

// Reads a validated text field; returns 1 on success, 0 on validation error, -1 on EOF
static int promptTextField(const char *prompt, char *outBuf, size_t maxLength) {
    char buffer[INPUT_BUFFER_SIZE];
    InputStatus status;

    printf("%s", prompt);
    status = readLine(buffer, sizeof(buffer));
    if (status == INPUT_EOF) {
        printf("\n[INFO] End of input reached during job registration.\n");
        return -1;
    }
    if (status == INPUT_TOO_LONG) {
        printf("[ERROR] Input too long (exceeds input buffer limit).\n");
        return 0;
    }
    if (!isValidTextField(buffer, maxLength)) {
        printf("[ERROR] Invalid input. Must be 1-%u characters without control characters.\n",
               (unsigned int)maxLength);
        return 0;
    }
    strncpy(outBuf, buffer, maxLength);
    outBuf[maxLength] = '\0';
    return 1;
}

// Handles menu option 1: Add Print Job; returns 0 if EOF should end program
static int handleAddPrintJob(PrinterSystem *system) {
    char userName[NAME_LENGTH + 1];
    char documentName[DOCUMENT_LENGTH + 1];
    char pageInput[INPUT_BUFFER_SIZE];
    InputStatus status;
    long pages = 0;
    unsigned int jobId = 0;
    int fieldResult;

    if (printerIsQueueFull(system)) {
        printf("[ERROR] Print queue is full (%d/%d). Process or cancel a job first.\n",
               QUEUE_CAPACITY, QUEUE_CAPACITY);
        return 1;
    }

    fieldResult = promptTextField("Enter user name: ", userName, NAME_LENGTH);
    if (fieldResult <= 0) {
        return fieldResult == -1 ? 0 : 1;
    }

    fieldResult = promptTextField("Enter document name: ", documentName, DOCUMENT_LENGTH);
    if (fieldResult <= 0) {
        return fieldResult == -1 ? 0 : 1;
    }

    printf("Enter number of pages (1-%d): ", MAX_PAGES);
    status = readLine(pageInput, sizeof(pageInput));
    if (status == INPUT_EOF) {
        printf("\n[INFO] End of input reached during job registration.\n");
        return 0;
    }
    if (status == INPUT_TOO_LONG || !parseLongInRange(pageInput, 1, MAX_PAGES, &pages)) {
        printf("[ERROR] Invalid page count. Enter an integer between 1 and %d.\n",
               MAX_PAGES);
        return 1;
    }

    if (printerAddJob(system, userName, documentName, (int)pages, &jobId) == QUEUE_OK) {
        printf("[OK] Print job #%u added for %s (%s, %ld pages).\n",
               jobId, userName, documentName, pages);
    } else {
        printf("[ERROR] Could not add print job. Queue state left unchanged.\n");
    }
    return 1;
}

// Handles menu option 4: Cancel Print Job; returns 0 if EOF should end program
static int handleCancelPrintJob(PrinterSystem *system) {
    char idInput[INPUT_BUFFER_SIZE];
    InputStatus status;
    long jobIdLong = 0;
    long maxJobId = (long)UINT_MAX;

    if ((unsigned long)LONG_MAX < (unsigned long)UINT_MAX) {
        maxJobId = LONG_MAX;
    }

    printf("Enter job ID to cancel: ");
    status = readLine(idInput, sizeof(idInput));
    if (status == INPUT_EOF) {
        printf("\n[INFO] End of input reached during job cancellation.\n");
        return 0;
    }
    if (status == INPUT_TOO_LONG || !parseLongInRange(idInput, 1, maxJobId, &jobIdLong)) {
        printf("[ERROR] Invalid job ID. Enter a positive whole number.\n");
        return 1;
    }

    printerCancelJob(system, (unsigned int)jobIdLong);
    return 1;
}

int main(void) {
    PrinterSystem system;
    char menuInput[INPUT_BUFFER_SIZE];
    int running = 1;

    initPrinterSystem(&system);
    printf("[INFO] PrintFlow simulator initialized (Queue capacity: %d, History capacity: %d).\n",
           QUEUE_CAPACITY, HISTORY_CAPACITY);

    while (running) {
        InputStatus status;

        displayMenu();
        status = readLine(menuInput, sizeof(menuInput));
        if (status == INPUT_EOF) {
            printf("\n[INFO] End of input reached. Exiting PrintFlow.\n");
            break;
        }
        if (status == INPUT_TOO_LONG || strlen(menuInput) != 1 ||
            menuInput[0] < '0' || menuInput[0] > '7') {
            printf("[ERROR] Invalid choice. Please enter a single digit from 0 to 7.\n");
            continue;
        }

        switch (menuInput[0]) {
            case '1':
                running = handleAddPrintJob(&system);
                break;
            case '2':
                printerProcessNextJob(&system);
                break;
            case '3':
                printerDisplayPendingJobs(&system);
                break;
            case '4':
                running = handleCancelPrintJob(&system);
                break;
            case '5':
                printerDisplayHistory(&system);
                break;
            case '6':
                printerDisplayStatus(&system);
                break;
            case '7':
                printerDisplayStatistics(&system);
                break;
            case '0':
                printf("[INFO] Exiting PrintFlow. Goodbye!\n");
                running = 0;
                break;
            default:
                printf("[ERROR] Invalid choice. Please enter a single digit from 0 to 7.\n");
                break;
        }
    }

    return 0;
}
