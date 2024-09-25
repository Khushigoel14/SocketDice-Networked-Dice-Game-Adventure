#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

// Function declaration for handling players
void handleGameSession(int player1, int player2);

int main(int argc, char *argv[]) {
    int serverSocket, playerSocket1, playerSocket2, portNumber;
    socklen_t addressLength;
    struct sockaddr_in serverAddress;
    int32_t receivedPoints;
    int dataSize = sizeof(receivedPoints);

    // Check if the correct number of arguments is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port Number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create a socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[-] Error: Socket creation failed");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "[+] Socket successfully created\n");
    }

    // Set up the server address structure
    memset(&serverAddress, 0, sizeof(serverAddress)); // Clear structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    sscanf(argv[1], "%d", &portNumber); // Parse the port number from argument
    serverAddress.sin_port = htons((uint16_t)portNumber); // Convert to network byte order

    // Bind the socket to the given IP and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("[-] Error: Binding failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 6) == 0) {
        printf("[+] Server is now listening...\n");
    } else {
        perror("[-] Error: Listening failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Main loop to handle incoming game requests
    while (1) {
        // Accept the first player connection
        playerSocket1 = accept(serverSocket, (struct sockaddr *)NULL, NULL);
        if (playerSocket1 < 0) {
            perror("[-] Error: Accepting Player 1 failed");
            continue;
        }
        write(playerSocket1, "Waiting for Player 2 to join...", 100);

        // Accept the second player connection
        playerSocket2 = accept(serverSocket, (struct sockaddr *)NULL, NULL);
        if (playerSocket2 < 0) {
            perror("[-] Error: Accepting Player 2 failed");
            close(playerSocket1); // Close the first player socket
            continue;
        }

        write(playerSocket1, "Player 2 joined... Game is starting...", 100);
        write(playerSocket2, "Player 1 already joined... Game is starting...", 100);
        printf("[+] A new game session has started\n");

        // Create a child process to handle the game session
        if (fork() == 0) {
            handleGameSession(playerSocket1, playerSocket2); // Handle the game session in child process
            exit(0); // Ensure child process exits after handling
        }
        
        // Parent process continues to listen for more connections
        close(playerSocket1);
        close(playerSocket2);
    }

    // Close the server socket (although never reached in this infinite loop)
    close(serverSocket);
    return 0;
}

// Function to handle the game session between two players
void handleGameSession(int player1, int player2) {
    int player1Score = 0, player2Score = 0;
    int dataSize = sizeof(int32_t);
    int32_t networkPointsPlayer1, networkPointsPlayer2;
    char scoreMessage[100];

    while (1) {
        sleep(1);
        // Player 1's turn
        write(player1, "You can now play", 100);
        if (read(player1, &networkPointsPlayer1, dataSize) < 0) {
            perror("Error reading from Player 1");
            break;
        }
        player1Score += ntohl(networkPointsPlayer1); // Convert score to host byte order

        snprintf(scoreMessage, sizeof(scoreMessage), "Your Score is :: %d \nOpponent Score is :: %d\n\n", player1Score, player2Score);
        write(player1, scoreMessage, sizeof(scoreMessage));

        // Check if Player 1 has won
        if (player1Score >= 100) {
            write(player1, "Game over: You won the game", 100);
            write(player2, "Game over: You lost the game", 100);
            break;
        }

        sleep(2);
        // Player 2's turn
        write(player2, "You can now play", 100);
        if (read(player2, &networkPointsPlayer2, dataSize) < 0) {
            perror("Error reading from Player 2");
            break;
        }
        player2Score += ntohl(networkPointsPlayer2); // Convert score to host byte order

        snprintf(scoreMessage, sizeof(scoreMessage), "Your Score is :: %d \nOpponent Score is :: %d\n\n", player2Score, player1Score);
        write(player2, scoreMessage, sizeof(scoreMessage));

        // Check if Player 2 has won
        if (player2Score >= 100) {
            write(player2, "Game over: You won the game", 100);
            write(player1, "Game over: You lost the game", 100);
            break;
        }
    }

    // Close both player sockets
    close(player1);
    close(player2);
}
