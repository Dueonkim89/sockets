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
#include <unordered_map>
#include <random>
#include <algorithm>
#include <cstdio>
#include <fstream>


using namespace std;

const int clientPort = 25250;
const int monitorPort = 26250;
const int UDPPort = 24250;
const int serverAPort = 21250;
const int serverBPort = 22250;
const int serverCPort = 23250;
const char* ipAddress = "127.0.0.1";
const string lowerAlphabet = "abcdefghijklmnopqrstuvwxyz";
const string upperAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const string digits = "0123456789";
const unordered_map<int, int> serverMap = {{0, 21250}, {1, 22250}, {2, 23250}};
int lastTx = -100;

int calculateBalance(vector<string>& userTx) {
    /* Loop through bector and get balance of user.*/
    int balance = 1000;

    for (const auto& tx : userTx) {
        try
        {
            balance += stoi(tx);
        }
        // in case value is "False"
        catch(const exception& e)
        {
            continue;
        }
    }
    return balance;
}

vector<string> convertStringtoVectorFormat(string data) {
    /*Helper method to convert a string into a vector*/

    istringstream iss(data);
    string response;
    vector<string> resData;
    
    while (iss >> response) {
        resData.push_back(response);
    }

    return resData;
}

vector<string> encryptString(vector<string>& operation) {
    /* String will be encrypted by offsetting by 3.
        Use global consts and ascii code.
    */
    vector<string> encryptedData = vector<string>();

    // loop through each string in operation
    for (size_t i = 0; i < operation.size(); i++) {
        string currData = operation[i];
        string encrypted = "";
        for (size_t j = 0; j<currData.length(); j++) {    
            // if alphabet
            if (isalpha(currData[j])) {
                // offset upper case alphabet by 3
                if (isupper(currData[j])) {
                    int currCharIdx = int(currData[j]) - int('A');
                    int offset = (currCharIdx + 3) % 26;
                    encrypted += upperAlphabet[offset];
                // offset lower case alphabet by 3
                } else {
                    int currCharIdx = int(currData[j]) - int('a');
                    int offset = (currCharIdx + 3) % 26;
                    encrypted += lowerAlphabet[offset];
                }
            // offset digit by 3
            } else if (isdigit(currData[j])) {
                int currCharIdx = int(currData[j]) - int('0');
                int offset = (currCharIdx + 3) % 10;
                encrypted += digits[offset];
            } else {
                encrypted += currData[j];
            }   
        }
        encryptedData.push_back(encrypted);
        // cout << encrypted << "\n";
    }
    return encryptedData;
}

vector<string> decryptString(vector<string>& operation) {
    /* String will be decrypted by offsetting by 3.
        Use global consts and ascii code.
    */
    vector<string> decryptedData = vector<string>();

    // loop through each string in operation
    for (size_t i = 0; i < operation.size(); i++) {
        string currData = operation[i];

        // edge case for "False" string
        if (currData == "False") {
            decryptedData.push_back(currData);
            continue;
        }

        string decrypted = "";
        for (size_t j = 0; j<currData.length(); j++) {
            // if alphabet
            if (isalpha(currData[j])) {
                // offset upper case alphabet by 3
                if (isupper(currData[j])) {
                    int currCharIdx = int(currData[j]) - int('A');
                    int offset = (currCharIdx - 3 + 26) % 26;
                    decrypted += upperAlphabet[offset];
                // offset lower case alphabet by 3
                } else {
                    int currCharIdx = int(currData[j]) - int('a');
                    int offset = (currCharIdx - 3 + 26) % 26;
                    decrypted += lowerAlphabet[offset];
                }
            // if digit offset by 3
            } else if (isdigit(currData[j])) {
                int currCharIdx = int(currData[j]) - int('0');
                int offset = (currCharIdx - 3 + 10) % 10;
                decrypted += digits[offset];
            } else {
                decrypted += currData[j];
            }   
        }
        decryptedData.push_back(decrypted);
        // cout << decrypted << "\n";
    }
    return decryptedData;
}

string generateClientMsg(int msgCode, vector<string>& operation) {
    /* This is the helper method to generate the appropriate 
    on screen message for client requests.*/

    string msg = "";
    string userName = "\"" + operation[0] + "\"";
    string port = to_string(clientPort);

    if (msgCode == 1) {
        msg = "The main server received input=" + userName;
        msg += " from the client using TCP over port " + port;
    } else {
        string receiver = "\"" + operation[1] + "\"";
        string amount = "\"" + operation[2] + "\"";
        msg = "The main server received from " + userName;
        msg += " to transfer " + amount + " coins to " + receiver;
        msg += " using TCP over " + port;
    }
    return msg;
}

string generateMonitorMsg() {
    string msg = "";
    return msg;
}

vector<string> getAllTransactions(int serverNumber) {
    /* Makes a request to all 3 servers in the backend 
    and returns encrypted transactions. */
    
    vector<string> allTransactions;
    int serverPort = serverMap.at(serverNumber);

    // make UDP client socket
    char buffer[1024];
    struct sockaddr_in clientAddr, servaddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    vector<string> reqData;

    if (sockfd < 0) {
        perror("UDP client socket creation failed"); 
        exit(EXIT_FAILURE); 
    }

    // clear garbage values and fill client information
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(ipAddress);
    clientAddr.sin_port = htons(UDPPort);

    // to allow option to reuse the same port
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0) {
        perror("Failed to bind UDP client socket");
        exit(EXIT_FAILURE);
    }

    // clear garbage values and fill server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ipAddress);
    servaddr.sin_port = htons(serverPort);

    string req = "GETTXLIST";
    const char *data = req.c_str();
    socklen_t len = sizeof(servaddr); 
    sendto(sockfd, (const char *)data, strlen(data),MSG_CONFIRM, (const struct sockaddr *) &servaddr,sizeof(servaddr));

    while (true) {
        int n = recvfrom(sockfd, (char *)buffer, sizeof(buffer),MSG_WAITALL, (struct sockaddr *) &servaddr,&len);

        if (n < 0) {
            string errMsg = "Failed to receive a line of data transaction from server " + string(1, upperAlphabet[serverNumber]);
            perror(errMsg.c_str());
        }

        buffer[n] = '\0';
        if (strcmp(buffer, "DONE") == 0) {
            break;
        }
        // append to vector
        string currLine = string(buffer);
        allTransactions.push_back(currLine);
    }
    
    return allTransactions;
}

bool recordFundTransfer(vector<string>& encryptedText) {
    /* This method will record the transfer from send to receiver in a randomly selected server.*/
    bool successfulTransfer = true;

    // get lastTx number.
    string nextTxNumber = to_string(lastTx + 1);

    // pick random number from 0 to 2.  **** Copied directly from chatgpt. ****
    random_device rd;  // Non-deterministic seed
    mt19937 gen(rd()); // Mersenne Twister RNG
    uniform_int_distribution<> dist(0, 2); // Range: 0 to 2
    int randomServerNum = dist(gen);
    int serverPort = serverMap.at(randomServerNum);

    // make UDP client socket
    char buffer[1024];
    struct sockaddr_in clientAddr, servaddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    vector<string> reqData;

    if (sockfd < 0) {
        perror("UDP client socket creation failed"); 
        exit(EXIT_FAILURE); 
    }

    // clear garbage values and fill client information
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(ipAddress);
    clientAddr.sin_port = htons(UDPPort);

    // to allow option to reuse the same port
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0) {
        perror("Failed to bind UDP client socket");
        exit(EXIT_FAILURE);
    }

    // clear garbage values and fill server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ipAddress);
    servaddr.sin_port = htons(serverPort);

    // msg format is: <nextTxNumber> <encrypted_sender_username> <encrypted_receiver_username> <encrypted_transfer_amount>
    string txLog = nextTxNumber + " " + encryptedText[0] + " " + encryptedText[1] + " " + encryptedText[2];
    string reqMsg = "The main server sent a request to server " + string(1, upperAlphabet[randomServerNum]);
    cout << reqMsg << "\n";

    string req = "3 " + txLog;
    const char *data = req.c_str();
    socklen_t len = sizeof(servaddr); 
    sendto(sockfd, (const char *)data, strlen(data),MSG_CONFIRM, (const struct sockaddr *) &servaddr,sizeof(servaddr));

    int n = recvfrom(sockfd, (char *)buffer, sizeof(buffer),MSG_WAITALL, (struct sockaddr *) &servaddr,&len);

    string dataReceived = "The main server received the feedback from server " + string(1, upperAlphabet[randomServerNum]);
    dataReceived += " using UDP over port " + to_string(serverPort);
    cout << dataReceived << "\n";

    // delimit string based on white space
    buffer[n] = '\0';
    string backEndResponse = string(buffer);
    istringstream iss(backEndResponse);
    string response;
    
    while (iss >> response) {
        reqData.push_back(response);
    }

    // back end server sends back confirmation once log is recorded.
    // if confirmation is "True" return true
    if (reqData[0] != "True") {
        successfulTransfer = false;
    }

    return successfulTransfer;
}


int getUserBalance(int startBalance, int transferAmount) {
    return startBalance - transferAmount;
}

int createTCPSockets(int portNumber) {
    // This method was influenced from https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp
    // creating sockets with TCP connection
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // need to allow lingering connections per chatgpt
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  // 

    // failed to be created
    if (sock < 0) {
        perror("Server M could not create a socket!");
        exit(EXIT_FAILURE);
    }

    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = inet_addr(ipAddress);

    // binding the socket.
    if (bind(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Server M could not bind the socket!");
        exit(EXIT_FAILURE);      
    }

    // listening to the socket
    if (listen(sock, 5) < 0) {
        perror("Server M could not listen to the socket!");
        exit(EXIT_FAILURE);  
    }

    return sock;
}

bool recordAllTransactions(vector<string>& decryptedText, vector<string>& masterList) {
    /* Record all transactions into file */
    const char* filename = "txchain.txt";

    // delete file if it already exists
    FILE* file = fopen(filename, "r");

    if (file) { 
        fclose(file);
        if (remove(filename) != 0) {
            perror("Error deleting txchain.txt file");
            return false;
        }  
    }

    string txFile = string(filename);
    ofstream log(txFile, ios::app);

    for (size_t i = 0; i<decryptedText.size(); i++ ) {
        vector<string> currMasterListText = convertStringtoVectorFormat(masterList[i]);
        vector<string> currDecryptedText = convertStringtoVectorFormat(decryptedText[i]);
        string lineToBeLogged = currMasterListText[0] + " " + currDecryptedText[1] + " " + currDecryptedText[2] + " " + currDecryptedText[3];
        if (log.is_open()) {
            log << lineToBeLogged << endl;
        }
    }
    log.close();
    return true;
}

vector<string> makeRequestToBackEndServers(int reqCode, int serverPortNumber, string user, char serverName) {
    /* reqCode will be either 1 or 2. 
       1 = CHECK WALLET, 
       2 = TXCOINS,
       3 = 
       serverPortNumber 21250 = A, 22250 = B, 23250 = C

       This method will create a UDP socket and send a datagram to the serverNumber provided.
       Socket creation and sending copied from 
       https://www.geeksforgeeks.org/udp-server-client-implementation-c/
    */
    char buffer[1024];
    struct sockaddr_in clientAddr, servaddr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    vector<string> reqData;

    if (sockfd < 0) {
        perror("UDP client socket creation failed"); 
        exit(EXIT_FAILURE); 
    }

    // clear garbage values and fill client information
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = inet_addr(ipAddress);
    clientAddr.sin_port = htons(UDPPort);

    // to allow option to reuse the same port
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0) {
        perror("Failed to bind UDP client socket");
        exit(EXIT_FAILURE);
    }

    // clear garbage values and fill server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ipAddress);
    servaddr.sin_port = htons(serverPortNumber);

    // CHECK WALLET Transaction
    if (reqCode == 1) {
        string reqMsg = "The main server sent a request to server " + string(1, serverName);
        cout << reqMsg << "\n";

        string req = "1 " + user;
        const char *data = req.c_str();
        socklen_t len = sizeof(servaddr); 
        sendto(sockfd, (const char *)data, strlen(data),MSG_CONFIRM, (const struct sockaddr *) &servaddr,sizeof(servaddr));

        int n = recvfrom(sockfd, (char *)buffer, sizeof(buffer),MSG_WAITALL, (struct sockaddr *) &servaddr,&len);

        /*
        if (n < 0) {
            string dataFailedToArrive = "Failed to receive datagram from backend server with port " + '"' + to_string(serverPortNumber) + '"';
            perror(dataFailedToArrive.c_str());
            exit(EXIT_FAILURE);
        }*/

        string dataReceived = "The main server received transactions from Server " + string(1, serverName);
        dataReceived += " using UDP over port " + to_string(serverPortNumber);
        cout << dataReceived << "\n";

        // delimit string based on white space
        // if 0th index is "False", then it means user doesnt exist.
        buffer[n] = '\0';
        string backEndResponse = string(buffer);
        istringstream iss(backEndResponse);
        string response;
        
        while (iss >> response) {
            reqData.push_back(response);
        }
    }
    // TXCOINS Transaction
    else if (reqCode == 2) {
        // need to return similar to reqCode 1 but also the last serial number.
        string reqMsg = "The main server sent a request to server " + string(1, serverName);
        cout << reqMsg << "\n";
        
        string req = "2 " + user;
        const char *data = req.c_str();
        socklen_t len = sizeof(servaddr); 
        sendto(sockfd, (const char *)data, strlen(data),MSG_CONFIRM, (const struct sockaddr *) &servaddr,sizeof(servaddr));        

        int n = recvfrom(sockfd, (char *)buffer, sizeof(buffer),MSG_WAITALL, (struct sockaddr *) &servaddr,&len);

        string dataReceived = "The main server received the feedback from server " + string(1, serverName);
        dataReceived += " using UDP over port " + to_string(serverPortNumber);
        cout << dataReceived << "\n";

        // delimit string based on white space
        // if 0th index is "False", then it means user doesnt exist.
        buffer[n] = '\0';
        string backEndResponse = string(buffer);
        istringstream iss(backEndResponse);
        string response;
        
        while (iss >> response) {
            // if last index of reqData is "LastSerialNumber:"
            if (!reqData.empty() && reqData[reqData.size() - 1] == "LastSerialNumber:") {
                // update lastTx number
                int currTx = stoi(response);
                if (currTx > lastTx) {
                    lastTx = currTx;
                }
                break;
            }
            reqData.push_back(response);
        }
        // remove last index of vector, dont want "LastSerialNumber: "
        reqData.pop_back();
        // cout << buffer << "\n";
    }

    // close and tear down connection
    close(sockfd);
    shutdown(sockfd, SHUT_RDWR);

    return reqData;
}

int extractLeadingInt(const string& s) {
    /* Helper function to help sort strings by 0th index.
    Copied from chatgpt */
    istringstream iss(s);
    int value;
    iss >> value;  // read leading integer
    return value;
}

int startServer() {
    // create both TCP sockets
    int clientServerSocket = createTCPSockets(clientPort);
    int monitorServerSocket = createTCPSockets(monitorPort);

    cout << "The main server is up and running.\n";

    // Used chat gpt for handling multiple sockets at once
    fd_set readfds;
    int maxfd = max(clientServerSocket, monitorServerSocket) + 1;
    bool recreateSockets = false;

    while (true) {
        if (recreateSockets) {
            // create both TCP sockets and bind them
            int clientServerSocket = createTCPSockets(clientPort);
            int monitorServerSocket = createTCPSockets(monitorPort);
            maxfd = max(clientServerSocket, monitorServerSocket) + 1;
            recreateSockets = false;
        }

        FD_ZERO(&readfds);
        FD_SET(clientServerSocket, &readfds);
        FD_SET(monitorServerSocket, &readfds);       

        int activity = select(maxfd, &readfds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            perror("Error reading from the main server sockets!");
            continue;
        } 

        // Handle all client requests
        if (FD_ISSET(clientServerSocket, &readfds)) {
            int clientSocket = accept(clientServerSocket, nullptr, nullptr);

            if (clientSocket < 0) {
                perror("Client connection failed.");
                continue;
            }
            // receiving data
            char buffer[1024] = { 0 };
            int messageBytes = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (messageBytes <= 0) {
                perror("Client message was not received.");
                continue;
            } else {
                // delimit client request based on white space into a string vector
                string clientReq = string(buffer);
                istringstream iss(clientReq);
                vector<string> operation;
                string command;
                
                while (iss >> command) {
                    operation.push_back(command);
                }

                // check for type of operation. 
                // First operation: Balance inquiry
                if (operation.size() == 1) {
                    string msg = generateClientMsg(1, operation);
                    cout << msg << "\n";
                    
                    // encrypt data
                    vector<string> encryptedData = encryptString(operation);
                    vector<string> masterList;

                    // make request to all 3 servers (A, B, C) for getting user balance
                    for (int i = 0; i < 3; i++) {
                        if (i == 0) {
                            vector<string> result = makeRequestToBackEndServers(1, serverAPort, encryptedData[0], upperAlphabet[i]);
                            masterList.insert(masterList.end(), result.begin(), result.end());
                        } else if (i == 1) {
                            vector<string> result2 = makeRequestToBackEndServers(1, serverBPort, encryptedData[0], upperAlphabet[i]);
                            masterList.insert(masterList.end(), result2.begin(), result2.end());
                        } else {
                            vector<string> result3 = makeRequestToBackEndServers(1, serverCPort, encryptedData[0], upperAlphabet[i]);
                            masterList.insert(masterList.end(), result3.begin(), result3.end());
                        }
                    }

                    // decrypt the integers
                    vector<string> decryptedTx = decryptString(masterList);

                    if (decryptedTx[0] == "False" && decryptedTx[1] == "False" && decryptedTx[2] == "False") {
                        const char* ack = "False";
                        send(clientSocket, ack, strlen(ack), 0);
                        cout << "The main server sent the current balance to the client.\n";  
                    } else {
                        // calculate user balance, since user is part of network
                        int userBalance = calculateBalance(decryptedTx);
                        string uBString = to_string(userBalance);       
                        const char* ack = uBString.c_str();
                        send(clientSocket, ack, strlen(ack), 0);
                        cout << "The main server sent the current balance to the client.\n";        
                    }    
                // Coin transfer
                } 
                else {
                    string msg = generateClientMsg(2, operation);
                    cout << msg << "\n";

                    // encrypt data and make request to all 3 servers (A - C)
                    int currTxAmt = stoi(operation[2]);
                    vector<string> encryptedData = encryptString(operation);
                    vector<string> senderMasterList;
                    vector<string> receiverMasterList;

                    // make request to all 3 servers (A, B, C) for getting sender and receiver balance
                    for (int i = 0; i < 3; i++) {
                        if (i == 0) {
                            vector<string> result = makeRequestToBackEndServers(2, serverAPort, encryptedData[0], upperAlphabet[i]);
                            senderMasterList.insert(senderMasterList.end(), result.begin(), result.end());

                            vector<string> receiverResult = makeRequestToBackEndServers(2, serverAPort, encryptedData[1], upperAlphabet[i]);
                            receiverMasterList.insert(receiverMasterList.end(), receiverResult.begin(), receiverResult.end());
                        } else if (i == 1) {
                            vector<string> result2 = makeRequestToBackEndServers(2, serverBPort, encryptedData[0], upperAlphabet[i]);
                            senderMasterList.insert(senderMasterList.end(), result2.begin(), result2.end());

                            vector<string> receiverResult2 = makeRequestToBackEndServers(2, serverBPort, encryptedData[1], upperAlphabet[i]);
                            receiverMasterList.insert(receiverMasterList.end(), receiverResult2.begin(), receiverResult2.end());
                        } else {
                            vector<string> result3 = makeRequestToBackEndServers(2, serverCPort, encryptedData[0], upperAlphabet[i]);
                            senderMasterList.insert(senderMasterList.end(), result3.begin(), result3.end());

                            vector<string> receiverResult3 = makeRequestToBackEndServers(2, serverCPort, encryptedData[1], upperAlphabet[i]);
                            receiverMasterList.insert(receiverMasterList.end(), receiverResult3.begin(), receiverResult3.end());
                        }
                    }

                    // decrypt the integers for sender only
                    vector<string> decryptedTx = decryptString(senderMasterList);

                    // string msg33 = "Last tx number is " + to_string(lastTx);
                    // cout << msg33 << "\n";
                    bool senderNotOnNetwork = decryptedTx[0] == "False" && decryptedTx[1] == "False" && decryptedTx[2] == "False";
                    bool receiverNotOnNetwork = receiverMasterList[0] == "False" && receiverMasterList[1] == "False" && receiverMasterList[2] == "False";
        
                    if (senderNotOnNetwork || receiverNotOnNetwork) {
                        // if both not on network, send 2 "False".
                        if (senderNotOnNetwork && receiverNotOnNetwork) {
                            const char* ack = "False False";
                            send(clientSocket, ack, strlen(ack), 0);
                        }
                        // else send 1 "False" to client
                        else {
                            const char* ack = "False";
                            send(clientSocket, ack, strlen(ack), 0);
                        }                        
                        cout << "The main server sent the result of the transaction to the client.\n";  
                    } else {
                        // calculate sender balance
                        int userBalance = calculateBalance(decryptedTx);
                        
                        // insufficient funds, send "NSF" and balance to client
                        if (userBalance < currTxAmt) {
                            string remainingBalance = "NSF " + to_string(userBalance);
                            const char* ack = remainingBalance.c_str();
                            send(clientSocket, ack, strlen(ack), 0);
                            cout << "The main server sent the result of the transaction to the client.\n";
                        }
                        // else, make the transfer
                        else {
                            if (recordFundTransfer(encryptedData)) {
                            // Once confirmation is received, main server gets updated balance of sender and sends back
                            // transaction status with updated balance. "True curr_balance"
                            int remainingBalance = getUserBalance(userBalance, currTxAmt);
                            string data = "True " + to_string(remainingBalance);       
                            const char* ack = data.c_str();
                            send(clientSocket, ack, strlen(ack), 0);
                            cout << "The main server sent the result of the transaction to the client. \n";
                            }
                            else {
                                // failed to record fund transfer
                                string data = "ServerError " + to_string(currTxAmt);
                                const char* ack = data.c_str();
                                // generate error message and send original starting balance
                                send(clientSocket, ack, strlen(ack), 0);
                                cout << "The main server sent the result of the transaction to the client. \n";
                            }
                        }                                   
                    }
                }
            }
            recreateSockets = true;
            close(monitorServerSocket);
            close(clientServerSocket);
        }

        // Handle all monitor requests
        if (FD_ISSET(monitorServerSocket, &readfds)) {
            int monitorSocket = accept(monitorServerSocket, nullptr, nullptr);

            if (monitorSocket < 0) {
                perror("Monitor connection failed.");
                continue;
            }
            // receiving data
            char buffer[1024] = { 0 };
            int messageBytes = recv(monitorSocket, buffer, sizeof(buffer), 0);
            if (messageBytes <= 0) {
                perror("Monitor message was not received.");
            } else {
                string msg = "The main server received a sorted list request from the monitor using TCP over port " + to_string(monitorPort);
                cout << msg << endl;

                // to hold all the logged transactions.
                vector<string> masterTransactionList;

                // get list of encrypted transactions from servers A - B.
                for (int i = 0; i < 3; i++) {
                    // concat all the vectors into master list
                    vector<string> currTransactions = getAllTransactions(i);
                    masterTransactionList.insert(masterTransactionList.end(), currTransactions.begin(), currTransactions.end());
                }
                
                // sort by 0th index, copied from chat gpt
                sort(masterTransactionList.begin(), masterTransactionList.end(), [](const string& a, const string& b) {
                    return extractLeadingInt(a) < extractLeadingInt(b);
                });

                // decrypt the list
                vector<string> decryptedTx = decryptString(masterTransactionList);

                // create log file and write to it.
                if (recordAllTransactions(decryptedTx, masterTransactionList)) {
                    const char* ack = "True";
                    send(monitorSocket, ack, strlen(ack), 0);
                } else {
                    const char* ack = "False";
                    send(monitorSocket, ack, strlen(ack), 0);
                }           
            }

            recreateSockets = true;
            close(monitorServerSocket);
            close(clientServerSocket);
        }
    }
}

int main()
{
    startServer();
}