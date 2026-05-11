#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h> //Memory Mapped Files-(Lab 7)
#include <fcntl.h>
#include <time.h> //Randomizing questions
#include "../common/protocol.h" //Shared configurations

//At a time
#define MAX_CLIENTS 10 

//Shared Data
Question question_bank[NUM_QUESTIONS] = {
    {1, "What is the command to list files in Linux?", "ls"},
    {2, "What command changes directories?", "cd"},
    {3, "What system call creates a new process?", "fork"},
    {4, "What system call replaces the current process image?", "exec"},
    {5, "What function maps a file directly into RAM?", "mmap"},
    {6, "What lock prevents thread race conditions?", "mutex"},
    {7, "Which tool compiles C programs in Linux?", "gcc"},
    {8, "What command displays the current working directory?", "pwd"},
    {9, "What command creates a new directory?", "mkdir"},
    {10, "What system call pauses until a child process finishes?", "wait"}
};

//Mutex for Synchronization-(Lab 11)
int total_students_finished = 0;
pthread_mutex_t scoreboard_lock = PTHREAD_MUTEX_INITIALIZER;

//Memory Mapped pointer for Fault Tolerance-(Lab 7)
StudentSession *mapped_scores;

void *handle_client(void *client_socket){
    int sock = *(int*)client_socket;
    char buffer[MAX_BUFFER] = {0};
    int score = 0;
    
    char *welcome = C_CYAN C_BOLD "\n======================================================\n"
                                  "      PARALLEL ONLINE EXAMINATION SYSTEM (POES)\n"
                                  "======================================================\n" C_RESET
                    C_YELLOW "Initializing secure session... Let's begin the exam!\n" C_RESET;
    send(sock, welcome, strlen(welcome), 0);
    
    //Seed random number generator uniquely for this student thread...
    srand(time(NULL) + sock);

    int q_order[NUM_QUESTIONS];
    for(int i = 0; i < NUM_QUESTIONS; i++) q_order[i] = i;

    //Fisher-Yates Shuffle Algorithm(Randomizes the array, changes the indexes) 
    for(int i = NUM_QUESTIONS - 1; i > 0; i--){
        int j = rand() % (i + 1);
        int temp = q_order[i];
        q_order[i] = q_order[j];
        q_order[j] = temp;
    }
    
    for(int i = 0; i < NUM_QUESTIONS; i++){
        int q_idx = q_order[i]; //randomized index
        
        char q_msg[512];
        sprintf(q_msg, C_BOLD "\n▶ Question %d of %d:\n" C_RESET C_CYAN "%s" C_RESET "\n" C_YELLOW "Your Answer: " C_RESET, 
                i + 1, NUM_QUESTIONS, question_bank[q_idx].text);
        send(sock, q_msg, strlen(q_msg), 0);
        
        memset(buffer, 0, MAX_BUFFER); //Read Answer
        int valread = read(sock, buffer, MAX_BUFFER);
        if(valread <= 0) break;
        
        buffer[strcspn(buffer, "\n")] = 0; //Remove trailing newline (C-pf concept)
        printf(C_YELLOW "[%d] [RX]" C_RESET " Received answer: '%s'\n", sock, buffer);

        if(strcmp(buffer, question_bank[q_idx].expected_answer) == 0) score++;
    }
    
    char final_msg[256];
    sprintf(final_msg, C_GREEN C_BOLD "\n======================================================\n"
                                      "      EXAM COMPLETE! Your final score is: %d/%d\n"
                                      "======================================================\n" C_RESET, score, NUM_QUESTIONS);
    send(sock, final_msg, strlen(final_msg), 0);
    
    printf(C_RED "[%d] [OS-LOCK]" C_RESET " Requesting Mutex lock for scoreboard...\n", sock); //CRITICAL-SECTION: Protecting shared data with Mutex Lock.
    pthread_mutex_lock(&scoreboard_lock);
    printf(C_RED "[%d] [OS-LOCK]" C_RESET " Lock acquired! Syncing data to mmap...\n", sock);
    
    int slot = total_students_finished % MAX_CLIENTS; //Assign a slot in our Memory Mapped File for this student.
    sprintf(mapped_scores[slot].student_id, "Student_%d", slot);
    mapped_scores[slot].score = score;
    
    total_students_finished++;
    
    printf(C_GREEN "[%d] [SYNC]" C_RESET " Saved Student_%d with score %d.\n", sock, slot, score);
    pthread_mutex_unlock(&scoreboard_lock);
    printf(C_RED "[%d] [OS-LOCK]" C_RESET " Mutex lock released.\n", sock);
    
    close(sock);
    free(client_socket);
    return NULL;
}

int main(){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    printf(C_CYAN C_BOLD "\n"
           "  ██████╗  ██████╗ ███████╗███████╗ \n"
           "  ██╔══██╗██╔═══██╗██╔════╝██╔════╝ \n"
           "  ██████╔╝██║   ██║█████╗  ███████╗ \n"
           "  ██╔═══╝ ██║   ██║██╔══╝  ╚════██║ \n"
           "  ██║     ╚██████╔╝███████╗███████║ \n"
           "  ╚═╝      ╚═════╝ ╚══════╝╚══════╝ \n" C_RESET);
    printf(C_YELLOW "  Starting Parallel Online Exam System...\n\n" C_RESET);

    //OS-CONCEPT: Memory Mapped Files(Lab 7)
    int fd = open("scores_backup.dat", O_RDWR | O_CREAT, 0666);
    ftruncate(fd, MAX_CLIENTS * sizeof(StudentSession)); 
    mapped_scores = mmap(NULL, MAX_CLIENTS * sizeof(StudentSession), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd); 
    printf(C_GREEN "[ OK ]" C_RESET " Memory-Mapped backup file mounted (Fault Tolerance Active)\n");

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    printf(C_GREEN "[ OK ]" C_RESET " IPC Socket bound successfully to Port %d\n", PORT);

    if(listen(server_fd, MAX_CLIENTS) < 0){
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf(C_GREEN "[ OK ]" C_RESET " POSIX Thread Pool capacity set to %d concurrent clients\n", MAX_CLIENTS);
    printf(C_CYAN C_BOLD "\n▶ SERVER ACTIVE & LISTENING FOR CONNECTIONS ◀\n\n" C_RESET);

    while(1){
        if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
            perror("Accept failed");
            continue;
        }
        
        printf(C_BLUE "\n[+] [CONNECT]" C_RESET " Spawning worker thread for new student (Socket ID: %d)\n", new_socket);
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;
        
        pthread_t thread_id;
        if(pthread_create(&thread_id, NULL, handle_client, (void*)new_sock) < 0){
            perror("Could not create thread");
            free(new_sock);
            close(new_socket);
        }
        pthread_detach(thread_id);
    }
    return 0;
}
