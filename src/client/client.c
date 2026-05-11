#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>  // For reading Linux directories
#include <ctype.h>   // For isdigit()
#include <signal.h>  // For killing processes and catching ALARMs
#include <pthread.h> // For the live timer UI thread
#include "../common/protocol.h" // Shared configurations

int time_left = 80;
int exam_active = 1;

// POSIX Threads (Live UI Updates)
void *timer_thread(void *arg) 
{
    sleep(2); 
    while (time_left > 0 && exam_active) 
    {
        printf("\033[s\033[2;50H" C_BG_RED C_BOLD " ⏳ TIME LEFT: %02d s " C_RESET "\033[u", time_left);
        fflush(stdout); // Force screen update
        sleep(1);
        time_left--;
    }
    return NULL;
}

// Signals & Hardware Interrupts (Exam Timer)
void handle_timeout(int sig) 
{
    printf("\n\n" C_BG_RED " ⌛ TIME IS UP! 80 SECONDS ELAPSED ⌛ " C_RESET "\n");
    printf(C_RED C_BOLD "Your exam has been forcefully submitted by the Operating System.\n" C_RESET);
    exit(0);
}


// Process Management & /proc File System (Lab 4)
void monitor_processes() 
{
    while (1) 
    {
        DIR *dir = opendir("/proc");
        if (dir == NULL) return;
        struct dirent *ent;
        
        while ((ent = readdir(dir)) != NULL) 
        {
            if (isdigit(ent->d_name[0])) {
                char path[512]; // Increased to 512 to fix the compiler warning
                snprintf(path, sizeof(path), "/proc/%s/comm", ent->d_name);
                FILE *fp = fopen(path, "r");
                if (fp) {
                    char comm[256];
                    if (fgets(comm, sizeof(comm), fp) != NULL) {
                        comm[strcspn(comm, "\n")] = 0; // Remove newline
                        // If student opens Firefox or Chrome, trigger Anti-Cheat
                        if (strcmp(comm, "firefox") == 0 || strcmp(comm, "chrome") == 0) 
                        {
                            printf("\n\n" C_BG_RED " 🚨 [SECURITY ALERT] CHEAT DETECTED! 🚨 " C_RESET "\n");
                            printf(C_RED C_BOLD "Illegal application '%s' is running on your OS!\n" C_RESET, comm);
                            printf(C_RED "Terminating exam and reporting to server immediately...\n" C_RESET);
                            
                            // Send a kill signal to the parent process (the exam client)
                            kill(getppid(), SIGKILL);
                            exit(1);
                        }
                    }
                    fclose(fp);
                }
            }
        }
        closedir(dir);
        sleep(2); // Scan the OS every 2 seconds
    }
}

int main() {
    // OS CONCEPT: fork() System Call
    // Create a hidden child process to run the Anti-Cheat Scanner
    pid_t monitor_pid = fork();
    
    if (monitor_pid == 0) 
    {
        // Child Process executes this
        monitor_processes();
        exit(0);
    } else if (monitor_pid < 0) 
    {
        printf("Failed to initialize Anti-Cheat engine.\n");
        return -1;
    }

    // Parent Process continues with the exam below...
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER] = {0};

    // Fancy Boot Sequence
    printf(C_CYAN "Initializing OS-Level Security Modules...\n" C_RESET);
    sleep(1);
    printf(C_CYAN "Establishing secure socket connection to server...\n" C_RESET);
    sleep(1);
    
    // Setup OS Hardware Interrupt (80-second Exam Timer)
    signal(SIGALRM, handle_timeout);
    alarm(80);
    
    // Start the live timer UI thread
    pthread_t timer_tid;
    pthread_create(&timer_tid, NULL, timer_thread, NULL);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf(C_RED "\n Socket creation error \n" C_RESET);
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); // Port imported from protocol.h

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
    {
        printf("\nInvalid address or Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        printf("\nConnection Failed! Is the server running?\n");
        return -1;
    }

    while (1) {
        memset(buffer, 0, MAX_BUFFER);
        valread = read(sock, buffer, MAX_BUFFER - 1);
        
        if (valread <= 0) 
        {
            printf(C_RED "\n[Disconnected] Connection closed by server.\n" C_RESET);
            break;
        }
        
        // Print whatever the server sent us (It's already colored by the server!)
        printf("%s", buffer);
        
        // If the server tells us the exam is over, stop the loop
        if (strstr(buffer, "EXAM COMPLETE") != NULL) 
        {
            break;
        }
        
        // If the server asks for an answer, get input from the student
        if (strstr(buffer, "Your Answer:") != NULL)
        {
            char answer[256];
            if (fgets(answer, sizeof(answer), stdin) != NULL) 
            {
                // Send the answer to the server
                send(sock, answer, strlen(answer), 0);
            }
        }
    }

    close(sock);
    
    // Exam finished successfully, kill the hidden anti-cheat child process
    kill(monitor_pid, SIGKILL);
    
    // Stop the live timer thread
    exam_active = 0;
    pthread_join(timer_tid, NULL);
    
    return 0;
}
