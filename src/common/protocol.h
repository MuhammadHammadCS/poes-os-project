#ifndef PROTOCOL_H
#define PROTOCOL_H

#define C_RESET   "\x1b[0m"
#define C_RED     "\x1b[31m"
#define C_GREEN   "\x1b[32m"
#define C_YELLOW  "\x1b[33m"
#define C_BLUE    "\x1b[34m"
#define C_CYAN    "\x1b[36m"
#define C_BOLD    "\x1b[1m"
#define C_BG_RED  "\x1b[41m\x1b[37;1m" // Red background, bold white text

// Network Configuration
#define PORT 8080
#define MAX_BUFFER 1024
#define NUM_QUESTIONS 10

// This struct will be used to pass questions over the network
typedef struct {
    int question_id;
    char text[256];
    char expected_answer[256]; // For simple grading
} Question;

// This struct will be used to keep track of a student's state
typedef struct {
    char student_id[50];
    int score;
} StudentSession;

#endif
