#include <stdio.h>
#include <stdlib.h>
#include "../common/protocol.h"

#define MAX_CLIENTS 10

int main() {
    printf(C_CYAN "\n[Recovery System]" C_RESET " Accessing memory-mapped binary backup...\n");

    FILE *fp = fopen("scores_backup.dat", "rb");
    if (!fp) {
        printf(C_RED "Error: No backup file found. Has anyone taken the exam yet?\n" C_RESET);
        return 1;
    }
    
    // Read the raw binary data back into our C struct array
    StudentSession recovered_scores[MAX_CLIENTS];
    fread(recovered_scores, sizeof(StudentSession), MAX_CLIENTS, fp);
    fclose(fp);
    
    printf(C_GREEN "\n============================================\n");
    printf("         RECOVERED FAULT-TOLERANT SCORES\n");
    printf("============================================\n" C_RESET);
    
    int found = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // If the student ID isn't empty, print it
        if (recovered_scores[i].student_id[0] != '\0') {
            printf(C_YELLOW "ID: " C_RESET "%-15s | " C_YELLOW "Score: " C_RESET "%d\n", 
                   recovered_scores[i].student_id, 
                   recovered_scores[i].score);
            found = 1;
        }
    }

    if (!found) {
        printf("No scores recorded yet.\n");
    }
    printf("\n");

    return 0;
}
