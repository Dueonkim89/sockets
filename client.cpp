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

const int serverPort = 25250;
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
    cout << "The client is up and running.\n";

    // copied from https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1) {
        perror("Client socket creation failed");
        exit(EXIT_FAILURE); 
    }

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    serverAddress.sin_addr.s_addr = inet_addr(ipAddress);

    // sending connection request
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Failed to connect to client socket!");
        close(clientSocket);
        exit(EXIT_FAILURE); 
    }

    // loop thru argv and concat
    string message = "";

    // skip ./client part of terminal input
    for (int i = 1; i < argc; ++i) {
        message += argv[i];
        if (i < argc - 1) { 
            message += " ";
        }
    }

    // need to pass pointer to send()
    const char* messagePointer = message.c_str();

    // sending data
    ssize_t byteSize = send(clientSocket, messagePointer, strlen(messagePointer), 0);
    string userName = "\"" + string(argv[1]) + "\"";

    if (argc == 2) {
        string terminalMsg = userName + " sent a balance enquiry request to the main server.";
        cout << terminalMsg << "\n";
    } else {
        string receiver = "\"" + string(argv[2]) + "\"";
        string amount = "\"" + string(argv[3]) + "\"";
        string terminalMsg = userName + " has requested to transfer " + amount + " txcoins to " + receiver;
        cout << terminalMsg << "\n";
    }

    if (byteSize == -1) {
        perror("Client failed to send message to server M!");
        exit(EXIT_FAILURE);
    }

    // get back response from server M
    char buffer[1024] = { 0 };
    ssize_t messageBytes = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (messageBytes <= 0) {
        perror("Unable to get back response from server M.");
        return EXIT_FAILURE;
    } else {
        // if balance inquiry
        if (argc == 2) {
            // serialize data into vector string
            buffer[messageBytes] = '\0';
            vector<string> resData = parseServerResponse(buffer);
            
            // User not part of network
            if (resData[0] == "False") {
                string notAuthorized = userName + " is not a part of the network.\n";
                cout << notAuthorized;
            }
            else {
                // show balance
                string authorized = "The current balance of " + userName + " is : " + resData[0] + " txcoins.\n";
                cout << authorized;
            }            
        } else {
            // successful transaction
            buffer[messageBytes] = '\0';
            vector<string> resData = parseServerResponse(buffer);

            string sender = argv[1];
            string receiver = argv[2];
            string txAmt = argv[3];
            
            if (resData[0] == "False") {
                // Sender or receiver not part of network
                string notAuthorized = "";
                if (resData.size() == 1) {
                    notAuthorized = "Unable to proceed with the transaction as " + sender + "/" + receiver + " is not part of the network.\n";
                    cout << notAuthorized;
                // both are not on the network
                } else {
                    notAuthorized = "Unable to proceed with the transaction as " + sender + " and " + receiver + " are not part of the network.\n";
                    cout << notAuthorized;
                }
            }
            // insufficient balance
            else if (resData[0] == "NSF") {
                string balance = resData[1];
                string part1 = sender + " was unable to transfer " + txAmt + "  txcoins to " + receiver + " because of insufficient balance.\n";
                part1 += "The current balance of " + sender + " is : " + balance + " txcoins.\n";
                cout << part1;
            }
            // success
            else if (resData[0] == "True") {
                string balance = resData[1];
                string part1 = sender + " successfully transferred " + txAmt + " txcoins to " + receiver + ".\n";
                part1 += "The current balance of " + sender + " is : " + balance + " txcoins.\n";
                cout << part1;
            }
            // server error, transaction could not be processed
            else {
                cout << "Unable to process transaction due to a server error. Please try again!\n";
            }           
        }
    }    
    
    // closing socket
    close(clientSocket);
    return 0;
}