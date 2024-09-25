#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char serverMessage[100];
    int clientSocket, portNumber;
    socklen_t addressLength;
    struct sockaddr_in serverAddress;
    int diceRoll;
    int32_t networkDiceValue;
    int dataSize = sizeof(networkDiceValue);
    long int seed = 0;

    // Validate input arguments
    if (argc != 3) {
        printf("Usage: %s <Server IP> <Port Number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create the socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[-] Error: Unable to create socket");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "[+] Socket successfully created\n");
    }

    // Set up the server address structure
    memset(&serverAddress, 0, sizeof(serverAddress)); // Clear structure
    serverAddress.sin_family = AF_INET;
    sscanf(argv[2], "%d", &portNumber); // Parse port number from argument
    serverAddress.sin_port = htons((uint16_t)portNumber); // Convert to network byte order

    // Convert IP address from text to binary
    if (inet_pton(AF_INET, argv[1], &serverAddress.sin_addr) <= 0) {
        perror("Error: Invalid IP address format or inet_pton() failed");
        exit(EXIT_FAILURE);
    }

    // Attempt to connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error: Connection to the server failed");
        exit(EXIT_FAILURE);
    }

    // Main loop to interact with the server
    while (1) {
        // Read the message from the server
        if (read(clientSocket, serverMessage, sizeof(serverMessage)) < 0) {
            perror("Error: Failed to read from the server");
            exit(EXIT_FAILURE);
        }

        // Display the server message
        printf("%s\n", serverMessage);

        // Check if the server allows us to play
        if (strcmp(serverMessage, "You can now play") == 0) {
            diceRoll = (int)time(&seed) % 6 + 1; // Generate dice roll
            printf("********** You rolled: %d **********\n", diceRoll);
            networkDiceValue = htonl(diceRoll);  // Convert to network byte order
            write(clientSocket, &networkDiceValue, dataSize); // Send to server
        }
    }

    // Close socket (although never reached due to infinite loop)
    close(clientSocket);
    return 0;
}
