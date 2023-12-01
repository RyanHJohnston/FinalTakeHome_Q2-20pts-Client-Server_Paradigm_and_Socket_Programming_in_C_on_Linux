// Include necessary header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void sendRequest(int clientSocket, const char* request) {
    write(clientSocket, request, strlen(request) + 1); // Send request
}

void receiveResponse(int clientSocket) {
    char response[128];
    int n = read(clientSocket, response, sizeof(response) - 1);
    response[n] = '\0'; // Null-terminate the string
    printf("Response from server: %s\n", response);
}

int main(int argc, char *argv[]) {
    // Variable declarations for client program
    struct sockaddr_in sad; // Structure to hold server's address
    int clientSocket; // Socket descriptor
    struct hostent *ptrh; // Pointer to a host table entry
    char *host; // Pointer to host name
    int port; // Protocol port number
    char Sentence[128]; // Buffer for input sentence
    char modifiedSentence[128]; // Buffer for the modified sentence
    int n; // Number of bytes read
    char input[10];

    // Check for correct number of arguments and parse them
    if (argc != 3) {
        fprintf(stderr,"Usage: %s server-name port-number\n",argv[0]);
        exit(1);
    }
    host = argv[1]; // Extract host-name from command-line argument
    port = atoi(argv[2]); // Extract port number and convert to binary

    // Create a TCP socket
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }

    // Connect the socket to the specified server
    memset((char *)&sad, 0, sizeof(sad)); // Clear sockaddr structure
    sad.sin_family = AF_INET; // Set family to Internet
    sad.sin_port = htons((u_short)port); // Set the port number
    ptrh = gethostbyname(host); // Convert host name to IP address
    if (((char *)ptrh) == NULL) {
        fprintf(stderr,"invalid host: %s\n", host);
        exit(1);
    }

    memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

    if (connect(clientSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        fprintf(stderr,"connect failed\n");
        exit(1);
    }
    
    while (1) {
        printf("1. Get GRADE for student\n");
        printf("2. Get Min, Max, and Avg\n");
        printf("3. Stop\n");
        printf("Enter (1/2/3): ");
        scanf(" %s", input);

        if (strcmp(input, "1") == 0) {
            char studentID[20];
            printf("Enter student ID: ");
            scanf(" %s", studentID);
            char request[30];
            sprintf(request, "GET_GRADE %s", studentID);
            sendRequest(clientSocket, request);
            receiveResponse(clientSocket);
        } else if (strcmp(input, "2") == 0) {
            sendRequest(clientSocket, "GET_MIN");
            receiveResponse(clientSocket);
            sendRequest(clientSocket, "GET_MAX");
            receiveResponse(clientSocket);
            sendRequest(clientSocket, "GET_AVG");
            receiveResponse(clientSocket);
        } else if (strcmp(input, "3") == 0) {
            printf("Stopping...\n");
            sendRequest(clientSocket, "STOP");
            break;
        } else {
            printf("Invalid option. Please try again.\n");
        }
    }
    // Close the socket
    close(clientSocket);
    return 0;
}

