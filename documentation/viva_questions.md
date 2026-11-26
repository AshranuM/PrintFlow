# PrintFlow Viva Questions and Answers

### 1. What is a queue?
A queue is a linear data structure in which insertions are performed at one end (the `rear`) and deletions are performed at the other end (the `front`). It models real-world waiting lines such as a printer spooler or ticket counter.

### 2. What is FIFO?
**FIFO** stands for **First-In, First-Out**. It means the element inserted first into the queue is the first one to be removed and processed. In PrintFlow, the oldest pending print job is always printed next.

### 3. What is a circular queue?
A circular queue is a queue implemented on a fixed-size array where the last position connects logically back to the first position using modulo (`%`) arithmetic. This forms a ring so freed spaces at the front can be reused.

### 4. Why is a circular queue used instead of a simple array queue?
In a simple linear array queue, once `rear` reaches the end of the array, you cannot insert new elements even if earlier elements were dequeued and left empty slots at the front. A circular queue reuses those slots in `O(1)` time without shifting all elements on every dequeue.

### 5. What is enqueue?
**Enqueue** is the operation of adding a new element (a `PrintJob`) to the `rear` of the queue. In PrintFlow, it first checks whether the queue is full, updates `rear = (rear + 1) % QUEUE_CAPACITY`, writes the job, and increments `count`.

### 6. What is dequeue?
**Dequeue** is the operation of removing and returning the element at the `front` of the queue. In PrintFlow, it checks whether the queue is empty, copies `jobs[front]`, updates `front = (front + 1) % QUEUE_CAPACITY`, and decrements `count`.

### 7. What is queue overflow?
**Overflow** occurs when we attempt to `enqueue` a new element into a queue that is already at maximum capacity (`count == QUEUE_CAPACITY`). PrintFlow rejects the submission with `[ERROR] Print queue is full` and never overwrites pending jobs.

### 8. What is queue underflow?
**Underflow** occurs when we attempt to `dequeue` from an empty queue (`count == 0`). PrintFlow handles this safely by returning `QUEUE_EMPTY` and displaying a friendly `[INFO]` message instead of crashing or reading garbage data.

### 9. Why do we use `front` and `rear`?
`front` tracks the index of the oldest pending job (the next job to be dequeued), while `rear` tracks the index of the most recently added job. Having both pointers lets both `enqueue` and `dequeue` run in `O(1)` constant time.

### 10. What is the purpose of `count` in the `Queue` struct?
In a circular array without `count`, `front` and `rear` can collide in ways that make distinguishing between "empty" and "full" tricky (often wasting one array slot). Keeping `count` makes `isEmpty` (`count == 0`), `isFull` (`count == QUEUE_CAPACITY`), and `queueSize` (`count`) completely clear and `O(1)`.

### 11. How does wrap-around work?
Wrap-around uses the modulo operator `%`. For example, if `QUEUE_CAPACITY` is `50` and `rear` is `49`, then `(49 + 1) % 50` evaluates to `0`, wrapping `rear` back to the start of the array.

### 12. Why is cancellation by ID `O(n)`?
Because a job can be cancelled from anywhere in the queue (front, middle, or rear), we may need to scan up to `n` pending jobs to find the matching `jobId`, and then shift up to `n - 1` subsequent jobs one logical step forward to keep the queue contiguous and preserve FIFO order.

### 13. What is the difference between a normal queue and a circular queue?
A normal linear queue moves `front` and `rear` only forward and cannot reuse freed front slots without shifting elements. A circular queue wraps `front` and `rear` around the array using modulo arithmetic, utilizing 100% of the array capacity efficiently.

### 14. What is completed history in PrintFlow?
Completed history is a second circular buffer inside `PrinterSystem` that stores jobs after they have finished printing. Unlike the pending queue, it acts as a log of recently printed jobs and never includes cancelled jobs.

### 15. What happens when the completed history buffer becomes full?
When `historyCount == HISTORY_CAPACITY`, the oldest completed record is overwritten by advancing both `historyFront` and `historyRear` modulo `HISTORY_CAPACITY`. However, `totalCompleted` continues to count every job ever completed.

### 16. What is time complexity, and what are the complexities in PrintFlow?
Time complexity describes how an operation's runtime grows as the number of elements `n` grows. In PrintFlow: `enqueue`, `dequeue`, `isEmpty`, `isFull`, and `queueSize` are all `O(1)`, while `queueRemoveById` (cancellation) and `displayQueue` are `O(n)`.

### 17. What is space complexity?
Space complexity measures the memory used by the data structures. PrintFlow uses fixed-size arrays of size `QUEUE_CAPACITY` and `HISTORY_CAPACITY`, so total memory is constant `O(QUEUE_CAPACITY + HISTORY_CAPACITY)` with `O(1)` extra space per operation.

### 18. Why are fixed-size arrays used instead of linked lists or `malloc`?
Fixed-size arrays are simple, fast, cache-friendly, avoid pointer overhead and memory leaks, and guarantee bounded memory usage—ideal for embedded systems and beginner DSA understanding.

### 19. Why is the project structured into separate modules (`queue`, `printer`, `main`)?
Modular programming separates responsibilities: `queue.c` manages only the circular queue mechanics, `printer.c` manages business rules (IDs, history, statistics, invariants), and `main.c` handles only the CLI menu and input validation. This makes the code easier to read, test, and maintain.

### 20. How does PrintFlow ensure statistics are always accurate?
PrintFlow maintains the conservation invariant `totalSubmitted == totalCompleted + totalCancelled + pending` after every state-changing operation using `assert()`, and ensures failed validations or full-queue rejections never modify counters or consume job IDs.

### 21. What are the limitations of PrintFlow?
PrintFlow uses compile-time fixed capacities, stores data in memory without file persistence across restarts, supports only single-printer FIFO scheduling (no priority queues), and simulates printing without connecting to real printer hardware.
