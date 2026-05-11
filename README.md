# 🎓 Parallel Online Examination System (POES)

A highly robust, multithreaded client-server examination system built entirely in C for Linux. This project was developed as a comprehensive demonstration of core **Operating System concepts**, including Concurrency, Inter-Process Communication (IPC), Memory Management, and Hardware Interrupts.

## 👥 Group Members
- **Abdullah** (24k-0859)
- **Abdul Majid** (24k-0895)
- **Muhammad Hammad** (24k-0544)

## 🌟 Key Features & OS Concepts Implemented

1. **Multithreaded Backend (`pthreads`)**: The server spawns a detached POSIX thread for every connecting student, allowing dozens of students to take the exam simultaneously without blocking the server.
2. **Thread Synchronization (Mutexes)**: Critical sections of the code, such as updating the global scoreboard, are protected using `pthread_mutex_t` to completely eliminate race conditions.
3. **OS-Level Anti-Cheat (`fork` & `/proc`)**: The client application forks a hidden child process that continuously scans the Linux `/proc` filesystem. If it detects forbidden applications (e.g., Firefox, Chrome), it fires a `SIGKILL` signal to instantly terminate the exam.
4. **Fault-Tolerant Backups (`mmap`)**: The server mounts a binary `.dat` file directly into RAM using Memory-Mapped Files. Student scores are updated in memory and automatically flushed to the disk by the Linux Kernel with zero CPU overhead, protecting data against server crashes.
5. **Hardware Timer Interrupts (`SIGALRM`)**: The client sets an OS-level hardware timer. If the student does not complete the exam within 80 seconds, the kernel throws an interrupt signal to forcefully submit and terminate the exam.
6. **Concurrent UI Threading**: The client uses a detached background thread and ANSI escape sequences to draw a live countdown timer on the screen without blocking the user's standard input (`stdin`).
7. **Fisher-Yates Randomization**: Each student thread seeds a unique Random Number Generator to shuffle the exam questions, ensuring no two students get the exact same test sequence.

## 📁 Project Structure

```text
poes_project/
├── Makefile
├── README.md
└── src/
    ├── server/
    │   ├── server.c         # Multithreaded TCP backend & mmap backups
    │   └── read_backup.c    # Utility to parse binary mmap files back to C Structs
    ├── client/
    │   └── client.c         # UI Threads, Sockets, and Anti-Cheat Monitor
    └── common/
        └── protocol.h       # Shared data structures, config, and ANSI colors
```

## 🚀 How to Build & Run

**Requirements:** A Linux environment (Ubuntu recommended) and `gcc`. The Anti-Cheat system relies heavily on the Linux `/proc` architecture and will not work natively on Windows or macOS.

1. **Compile the System:**
   ```bash
   make clean
   make
   ```

2. **Start the Server:**
   Open a terminal and run the backend daemon:
   ```bash
   ./server
   ```

3. **Connect a Client:**
   Open a second terminal window and connect to the exam:
   ```bash
   ./client
   ```

4. **Test the Recovery System:**
   After an exam is completed, you can view the raw binary memory-mapped data by running:
   ```bash
   ./read_backup
   ```

## 🛡️ Testing the Security
**Anti-Cheat Demo:** To verify the OS-level security monitor, start the `./client`. While taking the exam, open `firefox` or `chrome` on your Ubuntu machine. The hidden child process will detect the application in memory and immediately terminate your session!

**Timeout Demo:** Start the exam and do not touch your keyboard for 80 seconds. The hardware interrupt will automatically submit your exam and force close the client.

