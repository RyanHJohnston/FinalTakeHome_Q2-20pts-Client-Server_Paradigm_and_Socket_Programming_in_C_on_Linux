#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Define the structure for student information
struct student_info {
    char ID[7];
    char name[20];
    double grade;
};

// Initialize the database with 10 students
struct student_info DB[10] = {
    {"abc123", "Alice", 85.0},
    {"def456", "Bob", 90.5},
    {"ghi789", "Charlie", 88.0},
    {"jkl012", "Diana", 92.5},
    {"mno345", "Evan", 76.0},
    {"pqr678", "Fiona", 84.0},
    {"stu901", "George", 79.5},
    {"vwx234", "Hannah", 91.0},
    {"yza567", "Ian", 67.5},
    {"bcd890", "Julia", 95.0}
};

// Declare a function prototype for handling client requests
void processClientRequest(int connectionSocket);

int main(int argc, char *argv[]) {
    int i;

    // Variable declarations for server address, client address, etc.
    struct sockaddr_in sad; // Structure to hold server's address
    struct sockaddr_in cad; // Structure to hold client's address
    int welcomeSocket, connectionSocket; // Socket descriptors
    int alen; // Length of address
    int port; // Protocol port number

    // Check command-line argument for protocol port and extract it
    if (argc > 1) {
        port = atoi(argv[1]);
    } else {
        fprintf(stderr,"Usage: %s port-number\n",argv[0]);
        exit(1);
    }

    // Create a TCP socket
    welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (welcomeSocket < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }

    // Bind a local address to the socket
    memset((char *)&sad, 0, sizeof(sad)); // Clear sockaddr structure
    sad.sin_family = AF_INET; // Set family to Internet
    sad.sin_addr.s_addr = INADDR_ANY; // Set the local IP address
    sad.sin_port = htons((u_short)port); // Set the port number

    if (bind(welcomeSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"bind failed\n");
        exit(1);
    }

    // Specify the size of the request queue
    if (listen(welcomeSocket, 10) < 0) {
        fprintf(stderr,"listen failed\n");
        exit(1);
    }

    // Main server loop - accept and handle requests
    while (1) {
        alen = sizeof(cad);
        if ((connectionSocket = accept(welcomeSocket, (struct sockaddr *)&cad, &alen)) < 0) {
            fprintf(stderr, "accept failed\n");
            exit(1);
        }

        // Create a child process to handle the client
        if (fork() == 0) { // Child process
            close(welcomeSocket); // Close the listening socket in the child process
            printf("Closing listening socket in the child process...\n");
            printf("Processing client requests...\n");
            processClientRequest(connectionSocket); // Process client requests
            exit(0); // Terminate child process after handling the client
        }
        // Parent process continues here
        printf("Closing parent process socket...\n");
        close(connectionSocket); // Close the connected socket in the parent process
    }
    // Closing the listening socket (optional, as the loop is infinite)
    printf("Closing socket...\n");
    close(welcomeSocket);
}

// Define the function to process client requests
void processClientRequest(int sock) {
    char clientSentence[128];
    int n;
    int i;
    char response[128]; // Buffer for the response

    while (1) {
        // Receive a message from the client
        n = read(sock, clientSentence, sizeof(clientSentence) - 1);
        clientSentence[n] = '\0'; // Null-terminate the string

        if (strcmp(clientSentence, "STOP") == 0) {
            break; // Exit the loop and close the socket
        } else if (strncmp(clientSentence, "GET_GRADE ", 10) == 0) {
            // Extract student ID and find grade
            char* id = clientSentence + 10;
            int found = 0;
            for (i = 0; i < 10; ++i) {
                if (strcmp(DB[i].ID, id) == 0) {
                    sprintf(response, "Name: %s, Grade: %.1f", DB[i].name, DB[i].grade);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                strcpy(response, "Student not found.");
            }
        } else if (strcmp(clientSentence, "GET_MAX") == 0) {
            // Find the maximum grade
            double maxGrade = DB[0].grade;
            for (i = 1; i < 10; ++i) {
                if (DB[i].grade > maxGrade) {
                    maxGrade = DB[i].grade;
                }
            }
            sprintf(response, "Max Grade: %.1f", maxGrade);
        } else if (strcmp(clientSentence, "GET_MIN") == 0) {
            // Find the minimum grade
            double minGrade = DB[0].grade;
            for (i = 1; i < 10; ++i) {
                if (DB[i].grade < minGrade) {
                    minGrade = DB[i].grade;
                }
            }
            sprintf(response, "Min Grade: %.1f", minGrade);
        } else if (strcmp(clientSentence, "GET_AVG") == 0) {
            // Calculate the average grade
            double sum = 0;
            for (i = 0; i < 10; ++i) {
                sum += DB[i].grade;
            }
            double avg = sum / 10;
            sprintf(response, "Average Grade: %.1f", avg);
        } else {
            strcpy(response, "Invalid command.");
        }

        // Send the response back to the client
        write(sock, response, strlen(response) + 1);
    }

    close(sock); // Close the client socket after processing requests
}

