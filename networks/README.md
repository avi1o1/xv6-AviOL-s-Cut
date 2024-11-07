# Networks

## 1. XOXO
This is an implementation of a simple multiplayer Tic-Tac-Toe game using networking concepts. The game will have a server to manage the game state and two clients (players) who will play against each other. The server will be responsible for managing the game state, validating moves, and sending updates to the clients. The clients will be responsible for sending their moves to the server and displaying the game state to the players.

The game will be implemented using both [TCP](./XOXO/TCP/) and [UDP](./XOXO/UDP/) protocols to demonstrate the differences between the two protocols and their use cases. The TCP implementation will provide reliable communication between the server and clients, while the UDP implementation will provide faster communication with less overhead.

Each of the (linked) directories contains the following files:
- `server.c` : The server-side implementation of the game.
- `clientA.c` : The client-side implementation of the game.
- `clientB.c` : The client-side implementation of the game.


## 2. Fake it till you make it
This is an implementation of the TCP protocol using UDP-based communication. The goal is to mimic the reliable communication provided by TCP using features such as sequencing, acknowledgments, and retransmissions.

The implementation will consist of a client and a server that communicate using UDP packets. The client will send data packets to the server, which will be acknowledged by the server. In case of packet loss (indicated by lack of acknowledgemtn), the client will retransmit the lost packets until they are successfully received and acknowledged by the server. Once the server receives all the packets, it will display the received data to the user.

The directory contains the following files:
- `client.c`: The client-side implementation of the UDP-based reliable communication protocol.
- `server.c`: The server-side implementation of the UDP-based reliable communication protocol.

Note: This implementation is capable of only one-way communication from the client to the server. The reverse communication (from server to client) is not implemented in this version. (TODO: Implement reverse communication)