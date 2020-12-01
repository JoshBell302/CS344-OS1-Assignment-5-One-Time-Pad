#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_LENGTH 140000
#define ASCII_APLHABET 65
#define APLHAPBET_VALUE 26
#define AVAILABLE_VALUE 27

// Error function used for reporting issues
void error(const char* msg) {
	perror(msg);
	exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
	int portNumber) {

	// Clear out the address struct
	memset((char*)address, '\0', sizeof(*address));

	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);
	// Allow a client at any address to connect to this server
	address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char* argv[]) {
	// Setting Variables
	int connectionSocket, charsRead, keyStart, textCharsInt, keyCharsInt, encCharsInt;
	char buffer[BUFFER_LENGTH];
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);
	char* masterSent = calloc((BUFFER_LENGTH)+1, sizeof(char));
	int masterLength = 0;
	char* textSent = calloc((BUFFER_LENGTH)+1, sizeof(char));
	int textLength = 0;
	char* keySent = calloc((BUFFER_LENGTH)+1, sizeof(char));
	char* encrypt = calloc((BUFFER_LENGTH)+1, sizeof(char));
	int encLength = 0;
	int a = 0;
	int left = 0;

	// Setting Fork Variables
	pid_t spawnpid = -5;
	int childPid = 0;
	int childStatus = 0;

	// Forking
	spawnpid = fork();
	switch (spawnpid) {
	case -1:
		perror("fork() failed!");
		exit(1);
		break;
	case 0:
		// Check usage & args
		if (argc < 2) {
			fprintf(stderr, "USAGE: %s port\n", argv[0]);
			exit(1);
		}

		// Create the socket that will listen for connections
		int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (listenSocket < 0) {
			error("ERROR opening socket");
		}

		// Set up the address struct for the server socket
		setupAddressStruct(&serverAddress, atoi(argv[1]));

		// Associate the socket to the port
		if (bind(listenSocket,
			(struct sockaddr*)&serverAddress,
			sizeof(serverAddress)) < 0) {
			error("ERROR on binding");
		}

		// Start listening for connetions. Allow up to 5 connections to queue up
		listen(listenSocket, 5);

		// Accept a connection, blocking if one is not available until one connects
		while (1) {
			// Accept the connection request which creates a connection socket
			connectionSocket = accept(listenSocket,
				(struct sockaddr*)&clientAddress,
				&sizeOfClientInfo);
			if (connectionSocket < 0) {
				error("ERROR on accept");
			}

			// Get the message from the client and display it
			memset(masterSent, '\0', BUFFER_LENGTH);
			// Read the client's message from the socket
			charsRead = recv(connectionSocket, masterSent, BUFFER_LENGTH - 1, 0);
			if (charsRead < 0) {
				error("ERROR reading from socket");
			}

			// ----------------------------------------------------------------------------------------
			// Get length of the master string
			masterLength = strlen(masterSent);

			// Spliting the master string into text string and key string
			// Create text string
			for (int i = 0; i < masterLength; i++) {
				// Checks to see if there is @@
				if (masterSent[i] == '@' && masterSent[i + 1] == '@') {
					// Splits the master to the text and key
					for (int j = 0; j < i; j++) {
						textSent[j] = masterSent[j];
						textLength++;
					}
					keyStart = i + 2;
				}
			}

			// Create key string
			for (int k = 0; k < (strlen(masterSent)) - keyStart; k++)
				keySent[k] = masterSent[keyStart + k];

			//printf("The text is |%s|\n", textSent);
			//fflush(stdout);
			//printf("The key is |%s|\n", keySent);
			//fflush(stdout);

			// Create the encryption
			for (int i = 0; i < textLength; i++) {
				// Covert chars to their int value
				textCharsInt = abs(textSent[i] - ASCII_APLHABET);
				keyCharsInt = abs(keySent[i] - ASCII_APLHABET);

				// Check for space
				if (textCharsInt > 26)
					textCharsInt = 26;

				// Add the values together then mod to get encrypted char
				encCharsInt = textCharsInt + keyCharsInt;
				encCharsInt = encCharsInt % AVAILABLE_VALUE;

				// Fill the char into the encrypt string
				encrypt[i] = encCharsInt + ASCII_APLHABET;
			}

			//printf("The encryption is |%s|\n", encrypt);
			//fflush(stdout);

			// ----------------------------------------------------------------------------------------
			// Set values to use for sending
			encLength = strlen(encrypt);

			// Send a Success message back to the client
			while (send(connectionSocket, encrypt, strlen(encrypt), 0) != encLength) {
				if (charsRead < 0) {
					error("ERROR writing to socket");
				}
			}

			// Close the connection socket for this client
			close(connectionSocket);
		}
		// Close the listening socket
		close(listenSocket);
	default:
		childPid = wait(&childStatus);
		break;
	}
	return 0;
}