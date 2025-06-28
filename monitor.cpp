#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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
#include <vector>

using namespace std;

const int serverPort = 26250;
const char* ipAddress = "127.0.0.1";

vector<string> parseServerResponse(const char* buffer) {
    /* Parse the server response and use white space as delimiter*/
    string serverMResponse = string(buffer);
    istringstream iss(serverMResponse);
    string response;
    vector<string> resData;
    
    while (iss >> response) {
        resData.push_back(response);
    }

    return resData;
}

int main(int argc, char* argv[])
{
    cout << "The monitor is up and running.\n";

    int monitorSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (monitorSocket == -1) {
        perror("Monitor socket creation failed");
        exit(EXIT_FAILURE); 
    }

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = inet_addr(ipAddress);

   // sending connection request
    if (connect(monitorSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Failed to connect to client socket!");
        close(monitorSocket);
        exit(EXIT_FAILURE); 
    }
    // create request
    string message = "";

    for (int i = 1; i < argc; i++) {
        message += argv[i];
        if (i < argc - 1) { 
            message += " ";
        }
    }

    if (argc == 2) { 
        // need to pass pointer to send()
        const char* messagePointer = message.c_str();

        // sending data and write screen message
        ssize_t byteSize = send(monitorSocket, messagePointer, strlen(messagePointer), 0);
        cout << "Monitor sent a sorted list request to the main server.\n";

        if (byteSize == -1) {
            perror("Monitor failed to send message to server M!");
            exit(EXIT_FAILURE);
        }

        // get back response from server M
        char buffer[1024] = { 0 };
        int messageBytes = recv(monitorSocket, buffer, sizeof(buffer), 0);

        if (messageBytes <= 0) {
            perror("Unable to get back response from server M.");
            exit(EXIT_FAILURE); 
        } else {
            // serialize data into vector string
            buffer[messageBytes] = '\0';
            vector<string> resData = parseServerResponse(buffer);
            
            if (resData[0] == "True") { 
                cout << "Successfully received a sorted list of transactions from the main server.\n";
            }
            else {
                cout << "Failed to receive a sorted list of transactions from the main server.\n";
            }
        }
    }
    // closing socket
    close(monitorSocket);
    return 0;
}