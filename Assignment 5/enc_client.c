#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define BUFFER_LENGTH 140000

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char* msg) {
	perror(msg);
	exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
	int portNumber,
	char* hostname) {

	// Clear out the address struct
	memset((char*)address, '\0', sizeof(*address));

	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);

	// Get the DNS entry for this host name
	struct hostent* hostInfo = gethostbyname(hostname);
	if (hostInfo == NULL) {
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	// Copy the first IP address from the DNS entry to sin_addr.s_addr
	memcpy((char*)&address->sin_addr.s_addr,
		hostInfo->h_addr_list[0],
		hostInfo->h_length);
}

int main(int argc, char* argv[]) {
	//---------------------------------------------------------------------------------------------
			// Setting Variables
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	size_t len = 0;
	char* textBuffer = calloc((BUFFER_LENGTH)+1, sizeof(char));
	int textLength = 0;
	char* keyBuffer = calloc((BUFFER_LENGTH)+1, sizeof(char));
	int keyLength = 0;
	char* master = calloc((BUFFER_LENGTH)+1, sizeof(char));
	int masterLength = 0;
	char* recived = calloc((BUFFER_LENGTH)+1, sizeof(char));
	memset(recived, '\0', BUFFER_LENGTH);
	char* fullRecived = calloc((BUFFER_LENGTH)+1, sizeof(char));

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
		if (argc < 4) {
			fprintf(stderr, "Must provide: Text, Key, and Port\n");
			exit(0);
		}

		// Get information from the text file
		FILE* textFile = fopen(argv[1], "r");
		getline(&textBuffer, &len, textFile);
		textLength = strlen(textBuffer);

		// Checks to make sure text has valid characters
		for (int i = 0; i < textLength; i++) {
			if (textBuffer[i] != ' ' && (textBuffer[i] < 'A' || textBuffer[i] > 'Z')) {
				if (textBuffer[i] != '\n') {
					fprintf(stderr, "The text file has an invalid char at 'textBuffer[%d] = %c'\n", i, textBuffer[i]);
					exit(1);
				}
			}
		}

		// Gets rid of the new line to be replaced later
		textBuffer[textLength - 1] = '+';

		// Get information from the key file
		len = 0;
		FILE* keyFile = fopen(argv[2], "r");
		getline(&keyBuffer, &len, keyFile);
		keyLength = strlen(keyBuffer);

		if (keyLength < textLength) {
			fprintf(stderr, "The Key file is too small for this text file\n");
			exit(1);
		}

		// Create master string to send to server with '@@' inbetween to show begining of text and key
		strcpy(master, textBuffer);

		// Create the buffer 
		master[textLength - 1] = '@';
		master[textLength] = '@';

		// Add the keyBuffer to the master 
		strcat(master, keyBuffer);

		// Get rid of new line at end of master
		//masterLength = strlen(master);
		//master[masterLength - 10] = '\0';
		//---------------------------------------------------------------------------------------------

		// The information has gone through the test that means that the text and key are valid to be sent
		// Create a socket
		socketFD = socket(AF_INET, SOCK_STREAM, 0);
		if (socketFD < 0) {
			error("CLIENT: ERROR opening socket");
		}

		// Set up the server address struct
		setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

		// Connect to server
		if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			error("CLIENT: ERROR connecting");
		}

		//Send message to server
		charsWritten = send(socketFD, master, strlen(master), 0);
		if (charsWritten < 0) {
			error("CLIENT: ERROR writing to socket");
		}
		if (charsWritten < strlen(master)) {
			printf("CLIENT: WARNING: Not all data written to socket!\n");
		}

		//---------------------------------------------------------------------------------------------
		// Get return message from server
		// Read data from the socket, leaving \0 at end
		while (strlen(fullRecived) <= textLength - 1) {
			charsRead = recv(socketFD, recived, 1, 0);
			strcat(fullRecived, recived);
			if (charsRead < 0) {
				error("CLIENT: ERROR reading from socket");
			}
		}
		// Prints full message from the Server
		printf("%s", fullRecived);
		fflush(stdout);

		// Close the socket
		close(socketFD);
	default:
		childPid = wait(&childStatus);
		break;
	}
	return 0;
}