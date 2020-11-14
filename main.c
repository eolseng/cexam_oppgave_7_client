#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "./include/mop.h"
#include "./include/input.h"
#include "./include/simplelogger.h"

int VerifyAndSetArgs(int iArgC, char **apszArgV, int *iServerPort);

int EstablishConnection(int fdSocket);

void PrintMenu();

int SendMessageToServer(int fdSocket);

int main(int iArgC, char *apszArgV[]) {
    LogInfo("Starting 'Network Client' program");
    LogInfo("Log level set to %d", LOG_LEVEL);
    int iStatus = 0;

    int iServerPort;
    int fdSocket;
    struct sockaddr_in saAddr = {0};
    int iAdrLen = sizeof(saAddr);

    if (VerifyAndSetArgs(iArgC, apszArgV, &iServerPort) != 0) {
        LogError("Failed to verify args. Terminating program");
        iStatus = 1;
    } else {
        // Create a socket
        fdSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (fdSocket < 0) {
            LogError("Failed to open socket");
            iStatus = 1;
        } else {
            // Setup socket struct
            saAddr.sin_family = AF_INET;
            saAddr.sin_port = htons(iServerPort);
            saAddr.sin_addr.s_addr = htonl(0x7F000001); // 127.0.0.1
            // Connect to server
            if (connect(fdSocket, (struct sockaddr *) &saAddr, iAdrLen) < 0) {
                LogError("Failed to connect to server");
                iStatus = 1;
            } else {
                int iSelection, bActive = 1;
                if (EstablishConnection(fdSocket) != 0) {
                    LogError("Did not connect to server");
                } else {
                    while (bActive) {
                        PrintMenu();
                        // Get user input for menu selection
                        printf("Please select an option [%d-%d]: ", 1, 2);
                        iSelection = GetIntInRange(1, 2);
                        switch (iSelection) {
                            case 1:
                                // Send message to server. Shutdown on failure to connect to server
                                printf("* Send message to server selected *\r\n");
                                if (SendMessageToServer(fdSocket) == 2) bActive = 0;
                                break;
                            case 2:
                                printf("* Exit application selected *\r\n");
                                bActive = 0;
                                break;
                            default:
                                LogError("Invalid selection input: %d", iSelection);
                                bActive = 0;
                        }
                    }
                }
            }
        }
    }

    if (iStatus != 0)
        LogInfo("Program failed with status code %d", iStatus);
    else
        LogInfo("Program finished successfully");
    return iStatus;
}

int VerifyAndSetArgs(int iArgC, char **apszArgV, int *iServerPort) {

    /*
     * Verifies that two arguments is supplied and sets the values for server port
     * Finds "-server" and then tries to convert the next argument into a long, casted to an int
     *
     * Works as long as program is used as intended.
     */

    int i, iStatus = 0;

    if (iArgC != 3) {
        LogError("Wrong amount of arguments supplied. Expected 2, got %d", iArgC - 1);
        printf("ERROR: Wrong amount of arguments provided.\r\n");
        printf("ERROR: Please provide arguments: '-server [SERVER_PORT]'\r\n");
        iStatus = 1;
    } else {
        // Validate -server argument
        int bFoundPort = 0;
        for (i = 1; i < iArgC - 1; i++) {
            if (strcmp(apszArgV[i], "-server") == 0) {
                bFoundPort = 1;
                *iServerPort = (int) strtol(apszArgV[i + 1], NULL, 10);
                if (*iServerPort == 0) {
                    LogError("Failed to convert server port to integer. '-server' argument: '%s'", apszArgV[i + 1]);
                    printf("ERROR: Failed to convert server port to number. '-server' argument': '%s'\r\n",
                           apszArgV[i + 1]);
                    iStatus = 1;
                }
                break;
            }
        }
        if (!bFoundPort) {
            printf("ERROR: Please supply '-server [SERVER_PORT]' argument to program\r\n");
            iStatus = 1;
        }
    }
    return iStatus;
}

int EstablishConnection(int fdSocket) {
    char szBuffer[BUFFER_SIZE];
    DH stHeader = {0};
    if (SendMOPMessage(fdSocket, CONNECT, "") != 0) {
        LogError("Failed to send CONNECT message.");
        return 1;
    } else {
        int iRec = ReceiveMOPMessage(fdSocket, &stHeader, szBuffer, MSG_WAITALL);
        if (iRec != 0) {
            LogError("Error while establishing connection");
            return 1;
        } else if (stHeader.Type == DECLINE) {
            LogError("Failed to establish connection. Msg from server: %s", szBuffer);
            printf("ERROR: Connection to server declined. Message: %s\r\n", szBuffer);
            return 1;
        } else {
            LogInfo("Connection established.");
            while (1) {
                printf("Attempting to connect to server '%s'. Continue (Y/N): ", szBuffer);
                int c = getchar();
                fflush(stdin);
                if (c == 'y' || c == 'Y') return 0;
                else if (c == 'n' || c == 'N') return 1;
            }
        }
    }
}

void PrintMenu() {
    printf("------------------------------------\r\n");
    printf("Available commands:\r\n");
    printf("[1] Message server\r\n");
    printf("[2] Exit application\r\n");
    printf("------------------------------------\r\n");
}

int SendMessageToServer(int fdSocket) {

    char szBuffer[BUFFER_SIZE];
    bzero(szBuffer, BUFFER_SIZE);

    // Get user input
    printf("Enter message: ");
    GetStringWithMaxLength(BUFFER_SIZE, szBuffer);

    // Send message to server
    int iStatus = SendMOPMessage(fdSocket, MESSAGE, szBuffer);
    if (iStatus == 0) {
        printf("* Message sent to server *\r\n");
    } else if (iStatus == 1) {
        printf("* Internal error - please try again *\r\n");
    } else if (iStatus == 2) {
        printf("* Lost connection to server - shutting down *\r\n");
    }
    return iStatus;
}
