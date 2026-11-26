# PrintFlow Flowcharts and Diagrams

## 1. Main Menu Flow

```mermaid
flowchart TD
    A["Start Program"] --> B["Initialize PrinterSystem"]
    B --> C["Display Menu (0-7)"]
    C --> D["Read Line with readLine()"]
    D --> E{"Input EOF?"}
    E -- Yes --> F["Print EOF Exit Message & Exit"]
    E -- No --> G{"Single digit 0-7?"}
    G -- No --> H["Print Invalid Choice Error"]
    H --> C
    G -- Yes --> I{"Choice?"}
    I -- "1" --> J["Add Print Job"]
    I -- "2" --> K["Process Next Job"]
    I -- "3" --> L["Display Pending Jobs"]
    I -- "4" --> M["Cancel Print Job"]
    I -- "5" --> N["View Completed History"]
    I -- "6" --> O["Printer Status"]
    I -- "7" --> P["Statistics"]
    I -- "0" --> Q["Print Exit Message & Stop"]
    J --> C
    K --> C
    L --> C
    M --> C
    N --> C
    O --> C
    P --> C
```

## 2. Adding a Print Job

```mermaid
flowchart TD
    A["Select 1: Add Print Job"] --> B{"Queue Full?"}
    B -- Yes --> C["Print Queue Full Error (No State Change)"]
    B -- No --> D["Prompt & Validate User Name (1-30 chars)"]
    D --> E{"Valid?"}
    E -- No --> F["Abort & Return (No ID Consumed)"]
    E -- Yes --> G["Prompt & Validate Document Name (1-60 chars)"]
    G --> H{"Valid?"}
    H -- No --> F
    H -- Yes --> I["Prompt & Validate Pages (1-500 via strtol)"]
    I --> J{"Valid?"}
    J -- No --> F
    J -- Yes --> K["Assign nextJobId & Enqueue Job"]
    K --> L["Increment nextJobId & totalSubmitted"]
    L --> M["Verify Statistics Invariant & Print OK"]
```

## 3. Processing the Next Job

```mermaid
flowchart TD
    A["Select 2: Process Next Job"] --> B{"Queue Empty?"}
    B -- Yes --> C["Print No Pending Jobs Info"]
    B -- No --> D["Set Printer Status = BUSY"]
    D --> E["Dequeue Front Job (FIFO)"]
    E --> F["Display Job Details"]
    F --> G["Add Job to Completed History Buffer"]
    G --> H["Increment totalCompleted & totalPagesPrinted"]
    H --> I["Set Printer Status = IDLE"]
    I --> J["Verify Statistics Invariant & Print OK"]
```

## 4. Cancelling a Print Job

```mermaid
flowchart TD
    A["Select 4: Cancel Print Job"] --> B["Prompt & Validate Job ID (strtol)"]
    B --> C{"Valid Positive ID?"}
    C -- No --> D["Print Invalid Job ID Error"]
    C -- Yes --> E{"jobId >= nextJobId?"}
    E -- Yes --> F["Print Job Does Not Exist"]
    E -- No --> G["Call queueRemoveById()"]
    G --> H{"Found in Pending Queue?"}
    H -- Yes --> I["Shift Remaining Jobs & Update rear/count"]
    I --> J["Increment totalCancelled & Verify Invariant"]
    H -- No --> K{"In Completed History?"}
    K -- Yes --> L["Print Already Completed Message"]
    K -- No --> M["Print Already Completed or Cancelled Message"]
```

## 5. Circular Queue Wrap-Around Diagram (Capacity = 3)

```text
Initial (Empty): count = 0, front = 0, rear = 2
  Index:    [ 0 ]     [ 1 ]     [ 2 ]
  Data :    empty     empty     empty

After Enqueue(J1), Enqueue(J2), Enqueue(J3): count = 3, front = 0, rear = 2
  Index:    [ 0 ]     [ 1 ]     [ 2 ]
  Data :     J1        J2        J3
            ^front              ^rear

After Dequeue() x 2 (J1 and J2 printed): count = 1, front = 2, rear = 2
  Index:    [ 0 ]     [ 1 ]     [ 2 ]
  Data :    free      free       J3
                                ^front, ^rear

After Enqueue(J4), Enqueue(J5) (rear wraps 2 -> 0 -> 1): count = 3, front = 2, rear = 1
  Index:    [ 0 ]     [ 1 ]     [ 2 ]
  Data :     J4        J5        J3
                      ^rear     ^front

Logical FIFO Order via (front + offset) % 3:
  offset 0 -> (2 + 0) % 3 = index 2 -> J3
  offset 1 -> (2 + 1) % 3 = index 0 -> J4
  offset 2 -> (2 + 2) % 3 = index 1 -> J5
```
