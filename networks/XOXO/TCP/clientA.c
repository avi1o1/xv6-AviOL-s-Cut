#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return -1;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\033[1;31m Oopsie Woopsie : Socket Creation Error!\033[0m\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("\033[1;31m Oopsie Woopsie : Invalid Address!\033[0m\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\033[1;31m Oopsie Woopsie : Connection Failed!\033[0m\n");
        return -1;
    }
    
    // Waiting for other player to connect
    printf("\033[1;34m Waiting for the other player...\033[0m\n");

    // Game Start Message
    read(sock, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);
    memset(buffer, 0, BUFFER_SIZE);
    

    // Main game loop
    char input[1000];
    while (1) {
        // Current board state (or any error message)
        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);

        // Get response from the server
        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        // printf("--> %s\n", buffer);

        // This player's turn
        if (strncmp(buffer, "myTurn", 6) == 0) {
            printf("\033[1;33m Your move : \033[0m");
            fgets(input, sizeof(input), stdin);
            
            size_t len = strlen(input);
            if (len > 0 && input[len-1] == '\n') {
                input[len-1] = '\0';
            }

            usleep(100000);
            send(sock, input, strlen(input), 0);
        }

        // Other player's turn
        else if (strncmp(buffer, "otherTurn", 9) == 0) {
            printf("\033[1;33m Waiting for the other player to make a move...\033[0m\n");
        }

        // Someone disconnected
        else if (strncmp(buffer, "exit", 4) == 0) {
            if (strncmp(input, "exit", 4) == 0) {
                printf("\033[1;31m Disconnected. Thank you for playing!\033[0m\n");
            }
            else {
                printf("\033[1;31m Oopsie Woopie : The other player disconnected!\033[0m\n");
            }
            break;
        }

        // Won
        else if (strncmp(buffer, "won", 3) == 0) {
            printf("\033[1;35m Woo Hoo! You won! Party Wen?\033[0m\n");
            // break;
        }

        // Lost
        else if (strncmp(buffer, "lost", 4) == 0) {
            printf("\033[1;35m Nuh Uh : You lost! Git Gud\033[0m\n");
            // break;
        }

        // Draw
        else if (strncmp(buffer, "draw", 4) == 0) {
            printf("\033[1;35m Ah, it's a draw! Good game!\033[0m\n");
            // break;
        }

        // Rematch
        else if (strncmp(buffer, "rematch", 7) == 0) {
            printf("\033[1;34m Would you wish to play again? [Y/N] \033[0m");
            fgets(input, sizeof(input), stdin);
            
            size_t len = strlen(input);
            if (len > 0 && input[len-1] == '\n') {
                input[len-1] = '\0';
            }

            usleep(100000);
            send(sock, input, strlen(input), 0);
        }

        // Quit
        else if (strncmp(buffer, "gameOver", 8) == 0) {
            if (strncmp(input, "N", 1) == 0) {
                printf("\033[1;31m Disconnected. Thank you for playing!\033[0m\n");
            }
            else {
                printf("\033[1;31m Oopsie Woopsie : The other player quit!\033[0m\n");
            }
            break;
        } 

        // Invalid response
        else {
            printf("\033[1;31m Oopsie Woopsie : Invalid Response from the Server!\n[%s]\033[0m\n", buffer);
            break;
        }
    }

    // Close socket
    close(sock);
    return 0;
}
