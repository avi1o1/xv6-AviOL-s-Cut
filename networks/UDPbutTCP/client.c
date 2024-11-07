#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_DATA_SIZE 16
#define TIMEOUT 3

typedef struct Packet {
    int seqNum;
    char data[MAX_DATA_SIZE];
    int infoPack;
} Packet;

int setupClient(struct sockaddr_in *server_addr) {
    int sockfd;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\033[1;31m Oopsie Woopsie : Socket creation failed!\033[0m\n");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = INADDR_ANY;

    return sockfd;
}

int sendPacketAndWaitForAck(int sockfd, struct sockaddr_in *server_addr, socklen_t addr_len, Packet *packet) {
    int attempts = 0;
    int ack;
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    // Send packet and wait for ACK
    while (attempts < 5) {
        printf("\033[1;34m Sending packet %d: %s\033[0m\n", packet->seqNum, packet->data);
        sendto(sockfd, packet, sizeof(*packet), 0, (struct sockaddr *)server_addr, addr_len);

        // Test for ACK
        // if (packet->seqNum < 3) {
        //     sendto(sockfd, packet, sizeof(*packet), 0, (struct sockaddr *)server_addr, addr_len);
        // }

        // Set socket timeout for recvfrom
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        int n = recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL);

        if (n >= 0 && ack == packet->seqNum) {
            printf("\033[1;35m Received ACK for packet %d\033[0m\n", packet->seqNum);
            return 0;
        }
        else {
            printf("\033[1;33m Timeout or incorrect ACK, retransmitting packet %d\033[0m\n", packet->seqNum);
            attempts++;
        }
    }

    return -1;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    sockfd = setupClient(&server_addr);

    int seqNum = 0;
    while (1) {
        int error = 0;
        char *data = NULL;
        size_t len = 0;
        printf("\nEnter message to send (type 'quit' to exit): ");
        getline(&data, &len, stdin);
        data[strcspn(data, "\n")] = 0;

        // Calculate total packets
        int total_packets = (strlen(data) + MAX_DATA_SIZE - 1) / MAX_DATA_SIZE;
        
        // Send info packet
        Packet info_packet = {seqNum, "", 1};
        snprintf(info_packet.data, MAX_DATA_SIZE, "%d", total_packets);
        if (sendPacketAndWaitForAck(sockfd, &server_addr, addr_len, &info_packet) < 0) {
            printf("\033[1;31m Oopsie Woopsie : Failed to send info packet. Try again.\033[0m\n");
            free(data);
            continue;
        }
        seqNum++;

        // Send data packets
        for (int i = 0; i < total_packets; i++) {
            Packet packet = {seqNum, "", 0};
            int start_index = i * MAX_DATA_SIZE;
            int data_length = strlen(data + start_index) < MAX_DATA_SIZE ? strlen(data + start_index) : MAX_DATA_SIZE;
            strncpy(packet.data, &data[start_index], data_length);
            packet.data[data_length] = '\0';

            if (sendPacketAndWaitForAck(sockfd, &server_addr, addr_len, &packet) < 0) {
                printf("\033[1;31m Oopsie Woopsie : Failed to send packet %d. Try again.\033[0m\n", seqNum);
                error = 1;
                break;
            }
            seqNum++;
        }

        // Check for client exit
        if (strncmp(data, "quit", 4) == 0) {
            free(data);
            break;
        }

        if (!error) {
            printf("\033[1;32m Message sent and acknowledged. You can now send a new message.\033[0m\n");
        }
        free(data);
    }

    printf("\n\033[1;31m Closing the connection\033[0m\n");
    close(sockfd);
    return 0;
}