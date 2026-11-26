# Algorithms and Data Structures in PrintFlow

## 1. What Is a Queue?

A **queue** is a linear data structure where elements are added at one end (called the **rear**) and removed from the other end (called the **front**). It works just like a queue of students waiting at a college printer: the person who joins first gets their printout first.

## 2. The FIFO Principle

**FIFO** stands for **First-In, First-Out**.
- New print jobs always enter at the **rear** of the queue.
- The printer always picks the job at the **front** of the queue.
- No job can jump ahead of earlier jobs (unless an earlier job is cancelled).

## 3. Circular Queue Using an Array

In a simple linear array queue, each `dequeue` moves `front` forward and leaves empty slots behind. Eventually `rear` reaches the end of the array even though there is free space at the beginning.

A **circular queue** solves this by treating the fixed-size array as a circle using modulo arithmetic (`% QUEUE_CAPACITY`). When `rear` or `front` reaches the last index (`QUEUE_CAPACITY - 1`), adding `1` wraps it back to index `0`.

PrintFlow tracks:
- `front`: index of the oldest pending job
- `rear`: index of the newest pending job
- `count`: total number of jobs currently in the queue

Using `count` makes empty/full checks simple and unambiguous:
- **Empty condition:** `count == 0`
- **Full condition:** `count == QUEUE_CAPACITY`

## 4. Enqueue Operation

1. Check if `isFull(queue)` (`count == QUEUE_CAPACITY`). If full, return `QUEUE_FULL` without overwriting any job.
2. Advance `rear` circularly:
   ```c
   rear = (rear + 1) % QUEUE_CAPACITY;
   ```
3. Store the new `PrintJob` at `jobs[rear]`.
4. Increment `count` by `1`.
5. Return `QUEUE_OK`.

## 5. Dequeue Operation

1. Check if `isEmpty(queue)` (`count == 0`). If empty, return `QUEUE_EMPTY`.
2. Copy `jobs[front]` into the caller's `PrintJob` output struct.
3. Advance `front` circularly:
   ```c
   front = (front + 1) % QUEUE_CAPACITY;
   ```
4. Decrement `count` by `1`.
5. Return `QUEUE_OK`.

## 6. Cancellation by Job ID

Unlike `dequeue`, which always removes from the front, cancelling by ID can target the first, middle, or last job in the queue.

Algorithm in `queueRemoveById`:
1. Scan logical positions `offset = 0` to `count - 1`.
2. Convert each logical offset to its physical array index:
   ```c
   pos = (front + offset) % QUEUE_CAPACITY;
   ```
3. If `jobs[pos].jobId == jobId`, save the job details and record `foundOffset = offset`.
4. If not found, return `QUEUE_NOT_FOUND`.
5. Shift all elements after `foundOffset` one logical position toward `front`:
   ```c
   for (offset = foundOffset; offset < count - 1; offset++) {
       dest = (front + offset) % QUEUE_CAPACITY;
       src  = (front + offset + 1) % QUEUE_CAPACITY;
       jobs[dest] = jobs[src];
   }
   ```
6. Decrement `count` by `1`.
7. Update `rear` safely (works even when `count` becomes `0`):
   ```c
   rear = (front + count + QUEUE_CAPACITY - 1) % QUEUE_CAPACITY;
   ```

This preserves the FIFO order of all remaining jobs and handles wrap-around seamlessly.

## 7. Completed Job History (Overwriting Circular Buffer)

Completed jobs are stored in a second fixed-size circular buffer `history[HISTORY_CAPACITY]` with `historyFront`, `historyRear`, and `historyCount`.
- When a job finishes printing, if `historyCount < HISTORY_CAPACITY`, it is appended at `(historyRear + 1) % HISTORY_CAPACITY` and `historyCount` increases.
- When `historyCount == HISTORY_CAPACITY` (buffer full), both `historyFront` and `historyRear` advance by `1` modulo `HISTORY_CAPACITY`, overwriting the oldest completed entry.
- `totalCompleted` is tracked separately so the user always sees `Showing last X of Y completed jobs.`
- Cancelled jobs are **never** added to completed history.

## 8. Circular Wrap-Around Example (Capacity = 3)

1. Start empty: `front = 0, rear = 2, count = 0`
2. Enqueue J1, J2, J3 -> indices `0, 1, 2` (`front = 0, rear = 2, count = 3`)
3. Dequeue J1, J2 -> `front = 2, rear = 2, count = 1` (only J3 at index `2`; slots `0` and `1` are free!)
4. Enqueue J4 -> `rear = (2 + 1) % 3 = 0` -> J4 stored at index `0` (`front = 2, rear = 0, count = 2`)
5. Enqueue J5 -> `rear = (0 + 1) % 3 = 1` -> J5 stored at index `1` (`front = 2, rear = 1, count = 3`)
6. Logical traversal (`(front + offset) % 3` for `offset = 0, 1, 2`) visits indices `2 -> 0 -> 1` (J3, J4, J5 in exact FIFO order).

## 9. Time and Space Complexity Table

| Operation | Time Complexity | Explanation |
|---|---|---|
| `enqueue` | `O(1)` | Direct index update using `(rear + 1) % QUEUE_CAPACITY` |
| `dequeue` | `O(1)` | Direct index update using `(front + 1) % QUEUE_CAPACITY` |
| `isEmpty` | `O(1)` | Single comparison `count == 0` |
| `isFull` | `O(1)` | Single comparison `count == QUEUE_CAPACITY` |
| `queueSize` | `O(1)` | Returns stored `count` field directly |
| `queueRemoveById` (Cancellation) | `O(n)` | Searches up to `n` jobs and shifts up to `n - 1` jobs |
| `displayQueue` | `O(n)` | Visits each of the `n` pending jobs once |

**Space Complexity:** `O(QUEUE_CAPACITY + HISTORY_CAPACITY)` fixed static array space (`O(1)` auxiliary space per operation).
