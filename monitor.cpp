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

using namespace std;

const int serverPort = 26250;
const char* ipIaddress = "127.0.0.1";

int main(int argc, char* argv[])
{
    string message = "";

    for (int i = 1; i < argc; i++) {
        message += argv[i];
        if (i < argc - 1) { 
            message += " ";
        }
    }

    cout << "Monitor wrote.\n";
    cout << message;
    cout << "\n";
}
