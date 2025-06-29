#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <fstream>

using namespace std;

const int serverBPort = 22250;
const char* ipAddress = "127.0.0.1";
const int clientPort = 24250;
const string blockFile = "block2.txt";
int lastTx = -100;

vector<string> getUserBalance(string user) {
    /*  Get all user transactions. Data will be an array of length 2.
        0th index is receive and 1st index is sent.
        If user not in network, receive and sent amount will be 0
    */
    vector<string> userBalance;

    // open blockFile and update userBalance
    ifstream file(blockFile);
    if (!file) {
        cerr << "Error: Cannot open file.\n";
        return {"0"};
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string txData;
        vector<string> column;

        // delimit around white space
        while (iss >> txData) {
            column.push_back(txData);
        }

        // get highest tx number from log files
        int currTx = stoi(column[0]);
        if (currTx > lastTx) {
            lastTx = currTx;
        }

        // 1st index is sender
        if (column[1] == user) {
            userBalance.push_back('-' + column[3]);
        // 2nd index in receiver
        } else if (column[2] == user) {
            userBalance.push_back('+' + column[3]);
        }
    }

    // if userBalance is empty, send False
    if (userBalance.empty()) {
        userBalance = {"False"};
    }

    return userBalance;
}

string serializeData(vector<string>& data) {
    /*This method will use white space as a delimiter to serialize a vector of strings*/
    string serialized = "";

    for (const auto& tx : data) {
        serialized += tx + " ";  
    }

    return serialized;
}

bool recordTransaction(vector<string>& data) {
    /* Open CSV file and record the transaction. */
    bool success = true;
    string dataToBeLogged = data[1] + " " + data[2] + " " + data[3] + " " + data[4];

    ofstream log(blockFile, ios::app);

    if (log.is_open()) {
        log << dataToBeLogged << endl;
    } else {
        success = false;
    }

    log.close();
    return success;
}

bool openFileAndSendTransactions(int sockfd, const sockaddr_in& cliaddr, socklen_t len) {
    /* Open block file and send all transactions to server M*/
    bool success = true;
    ifstream file(blockFile);

    if (!file) {
        cerr << "Failed to open file\n";
        return false;
    }

    string line;
    while (getline(file, line)) { 
        ssize_t bytesSent =  sendto(sockfd, line.c_str(), line.length(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        if (bytesSent < 0) {
            success = false;
            perror("The ServerA failed to send the response to the Main Server.");
        }
        usleep(10000);
    }
    const char* ack = "DONE";
    sendto(sockfd, ack, strlen(ack), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
    return success;
}

int startServer() {
    // start up server code copied from https://www.geeksforgeeks.org/udp-server-client-implementation-c/
    char buffer[1024]; 
    struct sockaddr_in servaddr, cliaddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Creating socket file descriptor 
    if (sockfd < 0) { 
        perror("Server B socket creation failed!"); 
        exit(EXIT_FAILURE); 
    }

    // clear garbage values
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 

    // Filling server information 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ipAddress);
    servaddr.sin_port = htons(serverBPort);

    // Bind the socket with the server address 
    if (bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) { 
        perror("Server B could not bind the socket!"); 
        exit(EXIT_FAILURE); 
    }

    string portInStrFormat = "\"" + to_string(serverBPort) + "\"";
    string msg = "The Server B is up and running using UDP on port " + portInStrFormat;
    cout << msg << "\n";

    while (true) {
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);

        if (n < 0) {
            perror("Failed to receive datagram from server M.");
            continue;
        }
        int clientPort = ntohs(cliaddr.sin_port);
        if (clientPort == 24250) {
            cout << "The ServerB received a request from the Main Server.\n";

            // delimit the string based on white space
            buffer[n] = '\0';
            string serverMReq = string(buffer);
            istringstream iss(serverMReq);
            vector<string> requestVector;
            string req;
            
            while (iss >> req) {
                requestVector.push_back(req);
            }

            // if code is 1 - Get user balance by checking blockFile
            // If user is not in network, user will have 0, 0 for receive and sent.
            if (requestVector[0] == "1") {
                vector<string> userBalance = getUserBalance(requestVector[1]);
                string serialize = serializeData(userBalance);
                ssize_t bytesSent =  sendto(sockfd, serialize.c_str(), serialize.size(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);

                if (bytesSent < 0) {
                    perror("The ServerB failed to send the response to the Main Server.");
                } else {
                    cout << "The ServerB finished sending the response to the Main Server.\n";
                }
            }
            // if code is 2 - Get sender and receiver balance by checking blockFile
            // just reuse code "1" for both sender and receiver. serverM will make 2 requests.
            else if (requestVector[0] == "2") {
                vector<string> userBalance = getUserBalance(requestVector[1]);
                string serialize = serializeData(userBalance);
                string serialNumber = "LastSerialNumber: " + to_string(lastTx);
                serialize += serialNumber;
                ssize_t bytesSent =  sendto(sockfd, serialize.c_str(), serialize.size(), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);

                if (bytesSent < 0) {
                    perror("The ServerB failed to send the response to the Main Server.");
                } else {
                    cout << "The ServerB finished sending the response to the Main Server.\n";
                }

            }
            // record transaction into the log
            else if (requestVector[0] == "3") {
                if (recordTransaction(requestVector)) {
                    const char* ack = "True";
                    ssize_t bytesSent =  sendto(sockfd, ack, strlen(ack), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if (bytesSent < 0) {
                        perror("The ServerB failed to send the response to the Main Server.");
                    } else {
                        cout << "The ServerB finished sending the response to the Main Server.\n";
                    }       
                } else {
                    // send confirmation to main server that tx was logged.
                    perror("The ServerB failed to record the transaction into the log.");
                    const char* ack = "False";
                    ssize_t bytesSent =  sendto(sockfd, ack, strlen(ack), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
                    if (bytesSent < 0) {
                        perror("The ServerB failed to send the response to the Main Server.");
                    } else {
                        cout << "The ServerB finished sending the response to the Main Server.\n";
                    }
                }
            }
            // Get all transactions from txt file
            else {
                if (openFileAndSendTransactions(sockfd, cliaddr, len)) {
                    cout << "The ServerA finished sending the response to the Main Server.\n";
                } else {
                    perror("The ServerA failed to send the response to the Main Server.");
                }
            }
        }
    }

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    return 0; 
}

int main(int argc, char* argv[])
{
    startServer();
}