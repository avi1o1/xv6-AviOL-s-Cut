#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BOARD_SIZE 3

char board[BOARD_SIZE * BOARD_SIZE];
const char* invalidMove = "\033[1;31m Oopsie Woopsie : Invalid Move! \033[0m";
const char* invalidIdx = "\033[1;31m Oopsie Woopsie : Invalid Index! \033[0m";
const char* disconnect = " ";
const char* playAgain = " ";

// Function to get the server's IP address
void printServerIP() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;

    // Retrieve the hostname
    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1) {
        printf("\033[1;31m Oopsie Woopsie : gethostname error!\033[0m\n");
        exit(EXIT_FAILURE);
    }

    // Retrieve host information
    if ((host_entry = gethostbyname(hostbuffer)) == NULL) {
        printf("\033[1;31m Oopsie Woopsie : gethostbyname error!\033[0m\n");
        exit(EXIT_FAILURE);
    }

    // Convert the host's internet address to a string
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    printf("\033[1;32m Server started on IP Address: %s\033[0m\n", IPbuffer);
}

// Creating and setting up the server
int setupServer() {
    int serverFD;
    struct sockaddr_in address;
    int opt = 1;

    // Create socket file descriptor
    if ((serverFD = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        printf("\033[1;31m Oopsie Woopsie : Socket creation error!\033[0m\n");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(serverFD, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("\033[1;31m Oopsie Woopsie : Binding failed!\033[0m\n");
        exit(EXIT_FAILURE);
    }

    printServerIP();
    return serverFD;
}

// Send the current board state to both players
void sendCurrBoardState(int serverFD, struct sockaddr_in* player1Addr, struct sockaddr_in* player2Addr, char* currBoardState) {
    int N = 2 * BOARD_SIZE;
    char displayBoardString[N * BOARD_SIZE + 1];
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            displayBoardString[i*N + j*2] = currBoardState[i * BOARD_SIZE + j];
            displayBoardString[i*N + j*2 + 1] = ' ';
        }
        displayBoardString[i * N + N - 1] = '\n';
    }
    displayBoardString[N * BOARD_SIZE] = '\0';

    usleep(100000);
    sendto(serverFD, displayBoardString, strlen(displayBoardString), 0, (struct sockaddr*)player1Addr, sizeof(*player1Addr));
    sendto(serverFD, displayBoardString, strlen(displayBoardString), 0, (struct sockaddr*)player2Addr, sizeof(*player2Addr));
}

// Check if the game is over [0: Not Over, 1: Over, 2: Draw]
int isGameOver(char* board) {
    // Check rows
    for (int i = 0; i < BOARD_SIZE; i++) {
        int rowStart = i * BOARD_SIZE;
        int j;
        for (j = 1; j < BOARD_SIZE; j++) {
            if (board[rowStart] != board[rowStart + j] || board[rowStart] == '_') {
                break;
            }
        }
        if (j == BOARD_SIZE) {
            return 1;
        }
    }

    // Check columns
    for (int i = 0; i < BOARD_SIZE; i++) {
        int j;
        for (j = 1; j < BOARD_SIZE; j++) {
            if (board[i] != board[i + j * BOARD_SIZE] || board[i] == '_') {
                break;
            }
        }
        if (j == BOARD_SIZE) {
            return 1;
        }
    }

    // Check main diagonal
    int j;
    for (j = 1; j < BOARD_SIZE; j++) {
        if (board[0] != board[j * (BOARD_SIZE + 1)] || board[0] == '_') {
            break;
        }
    }
    if (j == BOARD_SIZE) {
        return 1;
    }

    // Check anti-diagonal
    for (j = 1; j < BOARD_SIZE; j++) {
        if (board[BOARD_SIZE - 1] != board[(j + 1) * (BOARD_SIZE - 1)] || board[BOARD_SIZE - 1] == '_') {
            break;
        }
    }
    if (j == BOARD_SIZE) {
        return 1;
    }

    // Check for draw
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++) {
        if (board[i] == '_') {
            return 0;
        }
    }

    return 2;
}

// Handle Player 1's response
int handlePlayer1Response(int serverFD, struct sockaddr_in* player1Addr, struct sockaddr_in* player2Addr) {
    char buffer[BUFFER_SIZE] = {0};
    socklen_t addr_len = sizeof(*player1Addr);

    // Get response from player 1
    recvfrom(serverFD, buffer, BUFFER_SIZE, 0, (struct sockaddr*)player1Addr, &addr_len);
    char* p1Response = strdup(buffer);
    memset(buffer, 0, BUFFER_SIZE);

    // Check if anyone disconnected
    if (strncmp(p1Response, "exit", 4) == 0) {
        printf("\033[1;33m Player 1 disconnected!\033[0m\n");
        free(p1Response);

        usleep(100000);
        sendto(serverFD, disconnect, strlen(disconnect), 0, (struct sockaddr*)player1Addr, addr_len);
        sendto(serverFD, disconnect, strlen(disconnect), 0, (struct sockaddr*)player2Addr, addr_len);

        usleep(100000);
        sendto(serverFD, "exit", strlen("exit"), 0, (struct sockaddr*)player1Addr, addr_len);
        sendto(serverFD, "exit", strlen("exit"), 0, (struct sockaddr*)player2Addr, addr_len);
        exit(EXIT_SUCCESS);
    }

    // Extracting Player 1's move
    int r, c;
    if (sscanf(p1Response, "%d %d", &r, &c) == 2) {
        if (r >= BOARD_SIZE || c >= BOARD_SIZE || r < 0 || c < 0 || board[r * BOARD_SIZE + c] != '_') {
            printf("Player 1: %s\n", p1Response);
            printf("\033[1;33m Invalid Index by Player 1!\033[0m\n");
            free(p1Response);

            usleep(100000);
            sendto(serverFD, invalidIdx, strlen(invalidIdx), 0, (struct sockaddr*)player1Addr, addr_len);
            usleep(100000);
            sendto(serverFD, "myTurn", strlen("myTurn"), 0, (struct sockaddr*)player1Addr, addr_len);
            handlePlayer1Response(serverFD, player1Addr, player2Addr);
            return 0;
        }
    }
    else {
        printf("Player 1: %s\n", p1Response);
        printf("\033[1;33m Invalid Move by Player 1!\033[0m\n");
        free(p1Response);

        usleep(100000);
        sendto(serverFD, invalidMove, strlen(invalidMove), 0, (struct sockaddr*)player1Addr, addr_len);
        usleep(100000);
        sendto(serverFD, "myTurn", strlen("myTurn"), 0, (struct sockaddr*)player1Addr, addr_len);
        handlePlayer1Response(serverFD, player1Addr, player2Addr);
        return 0;
    }

    // Update the board
    printf("Player 1: r = %d, c = %d\n", r, c);
    board[r * BOARD_SIZE + c] = 'X';

    if (isGameOver(board) == 1) {
        // Player 1 wins
        printf("\033[1;35m Player 1 wins!\033[0m\n");
        usleep(100000);
        sendCurrBoardState(serverFD, player1Addr, player2Addr, board);

        // Update the players
        usleep(100000);
        sendto(serverFD, "won", strlen("won"), 0, (struct sockaddr*)player1Addr, addr_len);
        sendto(serverFD, "lost", strlen("lost"), 0, (struct sockaddr*)player2Addr, addr_len);
        free(p1Response);
        return 1;
    }
    else if (isGameOver(board) == 2) {
        // It's a draw
        printf("\033[1;35m It's a draw!\033[0m\n");
        usleep(100000);
        sendCurrBoardState(serverFD, player1Addr, player2Addr, board);

        usleep(100000);
        sendto(serverFD, "draw", strlen("draw"), 0, (struct sockaddr*)player1Addr, addr_len);
        sendto(serverFD, "draw", strlen("draw"), 0, (struct sockaddr*)player2Addr, addr_len);
        free(p1Response);
        return 1;
    }

    free(p1Response);
    return 0;
}

// Handle Player 2's response
int handlePlayer2Response(int serverFD, struct sockaddr_in* player1Addr, struct sockaddr_in* player2Addr) {
    char buffer[BUFFER_SIZE] = {0};
    socklen_t addr_len = sizeof(*player2Addr);

    // Get response from player 2
    recvfrom(serverFD, buffer, BUFFER_SIZE, 0, (struct sockaddr*)player2Addr, &addr_len);
    char* p2Response = strdup(buffer);
    memset(buffer, 0, BUFFER_SIZE);

    // Check if anyone disconnected
    if (strncmp(p2Response, "exit", 4) == 0) {
        printf("\033[1;33m Player 2 disconnected!\033[0m\n");
        free(p2Response);

        usleep(100000);
        sendto(serverFD, disconnect, strlen(disconnect), 0, (struct sockaddr*)player1Addr, addr_len);
        sendto(serverFD, disconnect, strlen(disconnect), 0, (struct sockaddr*)player2Addr, addr_len);

        usleep(100000);
        sendto(serverFD, "exit", strlen("exit"), 0, (struct sockaddr*)player1Addr, addr_len);
        sendto(serverFD, "exit", strlen("exit"), 0, (struct sockaddr*)player2Addr, addr_len);
        exit(EXIT_SUCCESS);
    }

    // Extracting Player 2's move
    int r, c;
    if (sscanf(p2Response, "%d %d", &r, &c) == 2) {
        if (r >= BOARD_SIZE || c >= BOARD_SIZE || r < 0 || c < 0 || board[r * BOARD_SIZE + c] != '_') {
            printf("Player 2: %s\n", p2Response);
            printf("\033[1;33m Invalid Index by Player 2!\033[0m\n");
            free(p2Response);

            usleep(100000);
            sendto(serverFD, invalidIdx, strlen(invalidIdx), 0, (struct sockaddr*)player2Addr, addr_len);
            usleep(100000);
            sendto(serverFD, "myTurn", strlen("myTurn"), 0, (struct sockaddr*)player2Addr, addr_len);
            handlePlayer2Response(serverFD, player1Addr, player2Addr);
            return 0;
        }
    }
    else {
        printf("Player 2: %s\n", p2Response);
        printf("\033[1;33m Invalid Move by Player 2!\033[0m\n");
        free(p2Response);

        usleep(100000);
        sendto(serverFD, invalidMove, strlen(invalidMove), 0, (struct sockaddr*)player2Addr, addr_len);
        usleep(100000);
        sendto(serverFD, "myTurn", strlen("myTurn"), 0, (struct sockaddr*)player2Addr, addr_len);
        handlePlayer2Response(serverFD, player1Addr, player2Addr);
        return 0;
    }

    // Update the board
    printf("Player 2: r = %d, c = %d\n", r, c);
    board[r * BOARD_SIZE + c] = 'O';

    if (isGameOver(board) == 1) {
        // Player 2 wins
        printf("\033[1;35m Player 2 wins!\033[0m\n");
        usleep(100000);
        sendCurrBoardState(serverFD, player1Addr, player2Addr, board);

        // Update the players
        usleep(100000);
        sendto(serverFD, "won", strlen("won"), 0, (struct sockaddr*)player2Addr, addr_len);
        sendto(serverFD, "lost", strlen("lost"), 0, (struct sockaddr*)player1Addr, addr_len);
        free(p2Response);
        return 1;
    }
    else if (isGameOver(board) == 2) {
        // It's a draw
        printf("\033[1;35m It's a draw!\033[0m\n");
        usleep(100000);
        sendCurrBoardState(serverFD, player1Addr, player2Addr, board);
        
        usleep(100000);
        sendto(serverFD, "draw", strlen("draw"), 0, (struct sockaddr*)player1Addr, addr_len);
        sendto(serverFD, "draw", strlen("draw"), 0, (struct sockaddr*)player2Addr, addr_len);
        free(p2Response);
        return 1;
    }

    free(p2Response);
    return 0;
}

// Main Game Loop
void mainGameLoop(int serverFD, struct sockaddr_in* player1Addr, struct sockaddr_in* player2Addr) {
    while (1) {
        // Get Player 1 Turn
        usleep(100000);
        sendto(serverFD, "myTurn", strlen("myTurn"), 0, (struct sockaddr*)player1Addr, sizeof(*player1Addr));
        sendto(serverFD, "otherTurn", strlen("otherTurn"), 0, (struct sockaddr*)player2Addr, sizeof(*player2Addr));
        int x = handlePlayer1Response(serverFD, player1Addr, player2Addr);
        if (x == 1) {
            break;
        }

        // Send the current board state to both players
        sendCurrBoardState(serverFD, player1Addr, player2Addr, board);

        // Get Player 2 Turn
        usleep(100000);
        sendto(serverFD, "myTurn", strlen("myTurn"), 0, (struct sockaddr*)player2Addr, sizeof(*player2Addr));
        sendto(serverFD, "otherTurn", strlen("otherTurn"), 0, (struct sockaddr*)player1Addr, sizeof(*player1Addr));
        int y = handlePlayer2Response(serverFD, player1Addr, player2Addr);
        if (y == 1) {
            break;
        }

        // Send the current board state to both players
        sendCurrBoardState(serverFD, player1Addr, player2Addr, board);
    }
}

// Main function
int main() {
    struct sockaddr_in player1Addr, player2Addr;
    char buffer[BUFFER_SIZE] = {0};
    socklen_t addr_len = sizeof(player1Addr);

    // Setup the server
    int serverFD = setupServer();

    // Wait for player 1 to connect
    printf("\033[1;34m Waiting for Player 1...\033[0m\n");
    recvfrom(serverFD, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&player1Addr, &addr_len);
    printf("\033[1;32m Player 1 connected!\033[0m\n");

    // Wait for player 2 to connect
    printf("\033[1;34m Waiting for Player 2...\033[0m\n");
    recvfrom(serverFD, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&player2Addr, &addr_len);
    printf("\033[1;32m Player 2 connected!\033[0m\n");

    // Starting the Game
    printf("\033[1;34m Game starting!\033[0m\n");
    usleep(100000);

    char p1Response[1024] = "Hi, P1! You are 'X'.";
    sendto(serverFD, p1Response, strlen(p1Response), 0, (struct sockaddr*)&player1Addr, addr_len);

    char p2Response[1024] = "Hi, P2! You are 'O'.";
    sendto(serverFD, p2Response, strlen(p2Response), 0, (struct sockaddr*)&player2Addr, addr_len);

    // Initialize the board and send it to both players
    memset(board, '_', sizeof(board));
    sendCurrBoardState(serverFD, &player1Addr, &player2Addr, board);

    // Main game loop
    while (1) {
        // Main game loop
        mainGameLoop(serverFD, &player1Addr, &player2Addr);

        // Ask for a rematch
        usleep(100000);
        sendto(serverFD, playAgain, strlen(playAgain), 0, (struct sockaddr*)&player1Addr, addr_len);
        sendto(serverFD, playAgain, strlen(playAgain), 0, (struct sockaddr*)&player2Addr, addr_len);

        usleep(100000);
        sendto(serverFD, "rematch", strlen("rematch"), 0, (struct sockaddr*)&player1Addr, addr_len);
        sendto(serverFD, "rematch", strlen("rematch"), 0, (struct sockaddr*)&player2Addr, addr_len);

        // Get response from the players
        recvfrom(serverFD, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&player1Addr, &addr_len);
        char* p1Response = strdup(buffer);
        recvfrom(serverFD, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&player2Addr, &addr_len);
        char* p2Response = strdup(buffer);

        printf("Player 1: %s\n", p1Response);
        printf("Player 2: %s\n", p2Response);
        
        // Check if anyone wants to quit
        if (strncmp(p1Response, "N", 1) == 0 || strncmp(p2Response, "N", 1) == 0 || strncmp(p1Response, "n", 1) == 0 || strncmp(p2Response, "n", 1) == 0) {
            printf("\033[1;33m Game Over!\033[0m\n");
            free(p1Response);
            free(p2Response);

            usleep(100000);
            sendto(serverFD, disconnect, strlen(disconnect), 0, (struct sockaddr*)&player1Addr, addr_len);
            sendto(serverFD, disconnect, strlen(disconnect), 0, (struct sockaddr*)&player2Addr, addr_len);

            usleep(100000);
            sendto(serverFD, "gameOver", strlen("gameOver"), 0, (struct sockaddr*)&player1Addr, addr_len);
            sendto(serverFD, "gameOver", strlen("gameOver"), 0, (struct sockaddr*)&player2Addr, addr_len);
            break;
        }
        else {
            // usleep(100000);
            // send(player1Socket, newGame, strlen(newGame), 0);
            // send(player2Socket, newGame, strlen(newGame), 0);
            printf("\033[1;32m New Game Starting...\033[0m\n");

            // Reset the board
            memset(board, '_', sizeof(board));
            sendCurrBoardState(serverFD, &player1Addr, &player2Addr, board);
        }
    }

    // Close the server socket
    close(serverFD);
    return 0;
}