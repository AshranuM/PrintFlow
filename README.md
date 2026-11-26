# PrintFlow - Printer Queue Management Simulator

## Introduction

**PrintFlow** is a console-based Data Structures and Algorithms (DSA) mini-project written in standard C99. It simulates a shared printer used in a college lab, library, or office environment. Users can submit print jobs, process pending jobs in First-In-First-Out (FIFO) order, cancel pending jobs by ID, inspect recent completed history, and view live printer status and statistics.

> **Note:** This is a pure software simulation. It does not connect to or control any physical printer hardware.

## Objective

- Demonstrate the practical application of the **Queue** and **Circular Queue** data structures using fixed-size arrays.
- Implement safe, beginner-friendly modular C code without dynamic memory allocation (`malloc`/`calloc`/`realloc`) or unsafe input functions (`gets`/`scanf`).
- Maintain strict state integrity and statistics conservation (`submitted == completed + cancelled + pending`).

## Features

- **Add Print Job:** Validates user name (1-30 chars), document name (1-60 chars), and page count (1-500), then enqueues the job with a unique sequential ID.
- **Process Next Job:** Dequeues the oldest pending job (FIFO), transitions printer state (`BUSY` -> `IDLE`), records it in completed history, and updates page/job counters.
- **Display Pending Jobs:** Lists all pending jobs in FIFO order from `front` to `rear`, transparently handling circular wrap-around.
- **Cancel Print Job:** Removes a pending job by its ID in O(n) time while preserving the FIFO order of remaining jobs.
- **Completed Job History:** Uses a second fixed-size circular buffer that overwrites the oldest record when full while keeping the accurate total completed count.
- **Printer Status & Statistics:** Shows current status (`IDLE`/`BUSY`), queue usage, and cumulative counters.

## Technologies

- **Language:** C (ISO/IEC 9899:1999 / C99)
- **Compiler:** GCC / MinGW (`gcc.exe`)
- **Standard Library Only:** `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<ctype.h>`, `<limits.h>`, `<errno.h>`, `<assert.h>`
- **Interface:** ASCII-only terminal/console I/O (`fgets` + `strtol`)

## Project Structure

```text
PrintFlow/
|-- main.c
|-- queue.c
|-- queue.h
|-- printer.c
|-- printer.h
|-- README.md
|-- tests/
|   |-- test_cases.md
|   |-- t01.txt
|   |-- t02.txt
|   |-- t03.txt
|   |-- t04.txt
|   |-- t05.txt
|   |-- run_tests.bat
|-- documentation/
    |-- algorithms.md
    |-- flowcharts.md
    |-- viva_questions.md
```

## Compilation Instructions

Compile the standard build (capacity 50 for queue and history) on Windows with GCC:

```bat
gcc -std=c99 -Wall -Wextra -Wpedantic -o printflow main.c queue.c printer.c
```

Compile the small-capacity test build (capacity 3) to easily test queue-full, wrap-around, and history overflow:

```bat
gcc -std=c99 -Wall -Wextra -Wpedantic -DQUEUE_CAPACITY=3 -DHISTORY_CAPACITY=3 -o printflow_small main.c queue.c printer.c
```

## Execution Instructions

Run interactively:

```bat
printflow.exe
```

Run the automated test suite on Windows:

```bat
tests\run_tests.bat
```

## Sample Usage (Demonstration Sequence)

1. Submit 5 print jobs (IDs `1` to `5`) with page counts `2, 3, 4, 5, 6`.
2. Process Job `#1` (2 pages) and Job `#2` (3 pages).
3. Cancel Job `#3` (4 pages).
4. Submit 1 new print job (automatically receives ID `6`).

Resulting state:
- **Pending jobs:** `4, 5, 6`
- **Completed jobs:** `1, 2` (`Showing last 2 of 2 completed jobs.`)
- **Cancelled jobs:** `3`
- **Statistics:** `Submitted = 6`, `Completed = 2`, `Cancelled = 1`, `Pending = 3`, `Pages printed = 5`
- **Invariant check:** `6 = 2 + 1 + 3`

## Limitations

- Uses fixed-size arrays (`QUEUE_CAPACITY` and `HISTORY_CAPACITY` set at compile time).
- Data is stored in memory only and is not saved to disk after exiting.
- Simulates single-printer FIFO scheduling only (no priority levels or multi-printer scheduling).
- Does not interact with real OS print spoolers or physical hardware.

## DSA Concepts Demonstrated

- **Linear Queue & FIFO Principle:** First job submitted is the first job printed.
- **Circular Queue with Modulo Arithmetic:** Reuses freed array slots at the front without shifting on every dequeue (`O(1)` enqueue and dequeue).
- **Ring Buffer / Overwriting Circular Buffer:** Used for bounded completed-job history.
- **In-place Array Compaction on Arbitrary Removal:** `O(n)` cancellation by ID using logical-to-physical index mapping.
- **State Invariant Verification:** Runtime `assert()` checks guaranteeing conservation of submitted jobs.
