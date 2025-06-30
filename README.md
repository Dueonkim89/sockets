Socket programming in C++ using a combination of TCP & UDP protocols.

1. Du Kim
2. 5751-2512-50
3. I have done phases 1, 2, 3. Phase 4 was not done.
4. client.cpp - this file will generate the client executable that allows a user to run the operations 
CHECK WALLET and TXCOINS. client will make a TCP connection to serverM.

monitor.cpp - this file will generate the monitor executable that allows a user to see a txt file of all the
transactions in sorted ascending order. monitor will make a TCP connection to serverM.

serverM.cpp - this file will accept operations from monitor and client via TCP connection. this file will also
make requests to serverA, serverB, serverC VIA UDP for user information related to checking wallet, transferring
coins and creating a sorted list of all the decrypted transactions stored in the blockfile.

serverA.cpp - back end server that takes UDP connections from a client, only has access to block1.txt file

serverB.cpp - back end server that takes UDP connections from a client, only has access to block2.txt file

serverC.cpp - back end server that takes UDP connections from a client, only has access to block3.txt file

5. Messages printed to the terminal for each file were from tables 4 - 9.

6. Project may fail when creating a sorted transaction list, if the buffer were to hang. This is possible
if the block files are several thousand lines long.

7. For setting up client and server TCP and UDP sockets, I copied code from 
https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/
https://www.geeksforgeeks.org/udp-server-client-implementation-c/

This would be in 
- main() in client.cpp
- main() in monitor.cpp
- startServer() in serverM.cpp, serverA.cpp, serverB.cpp, serverC.cpp

I used chatgpt for
For handling multiple server TCP sockets.  Inside startServer() in serverM.cpp
For sorting a vector of strings in ascending order.  Inside startServer() in serverM.cpp
For picking a random int between 0 and 2. Inside recordFundTransfer() in serverM.cpp
For allowing reuse of address ports. Inside createTCPSockets() in serverM.cpp
For extracting a leading integer in a string. Inside extractLeadingInt() in serverM.cpp
