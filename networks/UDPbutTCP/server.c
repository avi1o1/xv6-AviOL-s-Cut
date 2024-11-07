#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#define PORT 8080
#define MAX_DATA_SIZE 16
#define MAX_MESSAGE_SIZE 1024

typedef struct Packet {
    int seqNum;
    char data[MAX_DATA_SIZE];
    int infoPack;
} Packet;

int setupServer(struct sockaddr_in *server_addr, struct sockaddr_in *client_addr, socklen_t *addr_len) {
    int sockfd;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(sockfd, (const struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    *addr_len = sizeof(*client_addr);
    return sockfd;
}

void send_ack(int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len, int ack_num) {
    sendto(sockfd, &ack_num, sizeof(ack_num), 0, (struct sockaddr *)client_addr, addr_len);
}

int main() {
    // Setup server
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    sockfd = setupServer(&server_addr, &client_addr, &addr_len);
    printf("UDP server is listening on port %d\n", PORT);

    char buffer[MAX_DATA_SIZE];
    Packet packet;
    int expectedSEQ = 0;
    
    // Initialize message buffer
    char message[MAX_MESSAGE_SIZE] = {0};
    int total_packets = 0;
    int received_packets = 0;

    // Receive packets
    while (1) {
        int n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n > 0 && packet.seqNum == expectedSEQ) {
            if (packet.infoPack == 1) {
                // This is the info packet
                total_packets = atoi(packet.data);
                printf("\n\033[1;34m Expecting %d packets for this message\033[0m\n", total_packets);
            }
            else {
                // This is a data packet
                strncat(message, packet.data, MAX_DATA_SIZE);
                received_packets++;
                printf("Received packet %d: %s\n", packet.seqNum, packet.data);
            }

            expectedSEQ++;

            // Send ACK for the received packet
            send_ack(sockfd, &client_addr, addr_len, packet.seqNum);

            // Check if we have received all packets for the current message
            if (received_packets == total_packets) {
                printf("\033[1;32m Complete message received: %s\033[0m\n", message);
                // Reset for the next message
                memset(message, 0, MAX_MESSAGE_SIZE);
                total_packets = 0;
                received_packets = 0;
            }
        }
        else {
            printf("Out of order packet or no packet received, expecting packet %d\n", expectedSEQ);
            continue;
        }

        // Stop if we receive a quit command
        if (strncmp(packet.data, "quit", 4) == 0) {
            break;
        }
    }

    printf("\n\033[1;31m Connection Closed! Server shutting down!\033[0m\n");
    close(sockfd);
    return 0;
}
