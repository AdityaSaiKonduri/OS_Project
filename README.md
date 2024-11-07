# Operating System Project  
**Group Members:**  
1. Aditya Sai Sriram Konduri - CS22B2012 
2. Navaneethakrishnan R - CS22B2014
3. Harshith B - CS22B2015
4. Pranaav Prasad - CS22B2022 

---

## Question 1:  
**[Dummy Heading for Question 1]**

---

### Problem Overview

---

## Question 2: Multi-Threaded File Management System  
**Topics Implemented:** `pthread`, `synchronization`, `signals`

In this project, we aim to implement a file management system where multiple clients (threads) can perform concurrent file operations on a server. The operations include reading, writing, renaming, copying, deleting files, and managing metadata, as well as logging activities. We used socket programming to simulate the client-server interaction, and synchronized access to resources using pthreads, semaphores, and signal handling to ensure thread safety.

### Features Implemented:
1. **Concurrent File Reading**  
   Multiple clients can read the same file at the same time without causing conflicts.

2. **One Client Only at a Time File Writing**  
   File write operations are mutually exclusive, meaning only one client can write to a file at any given time.

3. **File Deletion (One Client at a Time)**  
   File deletion is also mutually exclusive, ensuring no two clients can delete the same file simultaneously.

4. **File Renaming (One Client at a Time)**  
   Similar to deletion, file renaming is handled in a way that only one client can rename a file at a time.

5. **File Copying**  
   A client can copy a file to a new location, but this operation does not block other clients from performing other tasks.

6. **File Metadata Reading**  
   Clients can read the metadata of files (e.g., size, creation date, etc.) without affecting other operations.

7. **Activity Logging (Readable Only by Server)**  
   All activities performed by the clients are logged, but only the server can access and read these logs.

8. **File Compression**  
   Clients can compress files to save space. This operation is handled by the server and executed in a controlled manner.

9. **File Decompression**  
   Clients can decompress files, which are then made available for further operations.

### Methodology:
1. **Server and Client Setup**  
   - The server is implemented using `server.c`, and clients are represented using `client1.c` and `client2.c` (scalable to more clients).
   - The server listens for incoming connections from clients through a TCP socket.

2. **Thread Creation**  
   - Once the client-server connection is established, each client is assigned a unique `pthread` using `pthread_create`. The client IDs and pthreads are stored in arrays.

3. **Menu-driven Interface**  
   - The client program runs infinitely in a terminal, allowing users to interact with the server by selecting different options.
   - The server receives the client’s choice and spawns threads to execute the requested tasks concurrently.

4. **Semaphore for Synchronization**  
   - The server manages the read-write-execute (rwx) permissions using semaphores. Depending on the requested task, the server either executes the operation or waits for the necessary permissions to be released.

5. **Signal Handling**  
   - Error handling is done using signals to manage unexpected events or invalid operations during communication between the server and clients.

### Synchronization and Error Handling:
- **Semaphores** are used to synchronize access to critical resources such as file writing, deletion, renaming, and compression. This ensures that one client’s operation does not interfere with another’s, especially when modifying files.
  
- **Signals** are used to handle errors, such as invalid file paths, insufficient permissions, or unsuccessful file operations. The server communicates these errors to the client via signals.

### How to Run:
1. Compile all files (`server.c`, `client1.c`, `client2.c`).
2. Start the server:  
   `gcc server.c -o server`  
   `./server`
3. Start multiple clients:  
   `gcc client1.c -o client1`  
   `./client1`
   
   Repeat for `client2.c` or more clients.
   
4. The client will prompt for an option from the menu. The server will process the request based on the client’s input and handle synchronization.
