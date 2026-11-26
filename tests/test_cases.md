# PrintFlow Test Cases and Expected Outputs

This document maps all 25 required test cases to the automated test input files (`t01.txt` through `t05.txt`) executed by `tests/run_tests.bat`.

## Test Builds

1. **Normal Build** (`QUEUE_CAPACITY=50`, `HISTORY_CAPACITY=50`):
   ```bat
   gcc -std=c99 -Wall -Wextra -Wpedantic -o printflow main.c queue.c printer.c
   ```
2. **Small-Capacity Build** (`QUEUE_CAPACITY=3`, `HISTORY_CAPACITY=3`):
   ```bat
   gcc -std=c99 -Wall -Wextra -Wpedantic -DQUEUE_CAPACITY=3 -DHISTORY_CAPACITY=3 -o printflow_small main.c queue.c printer.c
   ```

## Test Case Matrix

| ID | Scenario | Input File | Build | Expected Key Output |
|---|---|---|---|---|
| TC01 | Add one print job | `t01.txt` | Normal | `[OK] Print job #1 added for Aarav Sharma (OS Lab Report.pdf, 2 pages).` |
| TC02 | Add several print jobs | `t01.txt` | Normal | Jobs #1 through #5 added sequentially |
| TC03 | Verify FIFO processing | `t01.txt` | Normal | Job #1 processed first, then Job #2 |
| TC04 | Process an empty queue | `t01.txt` | Normal | `[INFO] No pending jobs in the queue. Printer remains IDLE.` |
| TC05 | Cancel the first pending job | `t01.txt` | Normal | `[OK] Cancelled Job #4` when pending jobs are 4, 5, 6 |
| TC06 | Cancel a middle pending job | `t01.txt` | Normal | `[OK] Cancelled Job #3` when pending jobs are 3, 4, 5 |
| TC07 | Cancel the last pending job | `t01.txt` | Normal | `[OK] Cancelled Job #6` when pending jobs are 5, 6 |
| TC08 | Cancel a non-existing job | `t01.txt` | Normal | `[ERROR] Job #99 does not exist.` |
| TC09 | Cancel a previously completed job | `t01.txt` | Normal | `[INFO] Job #1 has already been completed and cannot be cancelled.` |
| TC10 | Cancel the only pending job | `t01.txt` | Normal | `[OK] Cancelled Job #5` leaves queue empty (`0` pending) |
| TC11 | Queue-full condition | `t02.txt` | Small (3) | `[ERROR] Print queue is full (3/3). Process or cancel a job first.` |
| TC12 | Circular wrap-around | `t02.txt` | Small (3) | After processing #1 and #2, jobs #4 and #5 wrap around and display in FIFO order (`3, 4, 5`) |
| TC13 | Cancellation after wrap-around | `t02.txt` | Small (3) | `[OK] Cancelled Job #4` preserves order (`3, 5`) across wrapped slots |
| TC14 | Verify job IDs are not reused | `t02.txt` | Small (3) | Next job receives `#6`, never reusing cancelled `#4` or completed IDs |
| TC15 | Verify failed submissions do not consume IDs | `t03.txt` | Normal | After 11 failed submissions, next valid job gets `#2` |
| TC16 | Completed-history overflow | `t02.txt` | Small (3) | Completing 4 jobs overwrites Job #1; history shows `#2, #3, #5` |
| TC17 | Total completed count after history overflow | `t02.txt` | Small (3) | `[INFO] Showing last 3 of 4 completed jobs.` |
| TC18 | Invalid page input (`0`, `-5`, `501`, `abc`, `12abc`, overflow) | `t03.txt` | Normal | `[ERROR] Invalid page count. Enter an integer between 1 and 500.` |
| TC19 | Empty / whitespace-only names | `t03.txt` | Normal | `[ERROR] Invalid input. Must be 1-30 characters...` / `1-60 characters...` |
| TC20 | Overlong input (>30 user, >60 doc, >127 buffer) | `t03.txt` | Normal | Error displayed and remainder of >127-char line flushed cleanly |
| TC21 | Invalid menu input (empty, `12`, `x`, `9`) | `t03.txt` | Normal | `[ERROR] Invalid choice. Please enter a single digit from 0 to 7.` |
| TC22 | EOF at menu | `t05.txt` | Normal | `[INFO] End of input reached. Exiting PrintFlow.` (exit code 0) |
| TC23 | EOF during job registration | `t04.txt` | Normal | `[INFO] End of input reached during job registration.` (exit code 0) |
| TC24 | Statistics invariant (`submitted == completed + cancelled + pending`) | All | Both | Verified via `assert()` and printed stats (e.g., `6 == 2 + 1 + 3`, `6 == 4 + 1 + 1`) |
| TC25 | Failed operations leave state unchanged | `t02.txt`, `t03.txt` | Both | Queue size, pending IDs, history, and stats remain unchanged after errors |

## Demonstration Sequence Key Output (`t01.txt`)

After adding jobs 1..5 (pages 2, 3, 4, 5, 6), processing jobs 1 and 2, cancelling job 3, and adding job 6 (7 pages):
- Pending jobs: `4, 5, 6`
- Completed jobs: `1, 2` (`Showing last 2 of 2 completed jobs.`)
- Cancelled jobs: `3`
- Statistics:
  - `Total Submitted Jobs : 6`
  - `Total Completed Jobs : 2`
  - `Total Cancelled Jobs : 1`
  - `Pending Jobs         : 3`
  - `Total Pages Printed  : 5`
- Invariant check: `6 == 2 + 1 + 3`
