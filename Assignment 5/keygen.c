#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char* argv[]) {
	// Check to see if the user provided a length for the keygen
	if (argc < 2) {
		printf("You must provide the length for the keygen\n");
		return EXIT_FAILURE;
	}

	// Set up some variables
	int length = atoi(argv[1]);
	char* key = calloc((length)+1, sizeof(char));
	int randValue = 0;
	char numberChar;
	time_t t;

	// Set Random
	srand((unsigned)time(&t));

	// Creates a key with random chars from A-Z and space
	for (int i = 0; i < length; i++) {
		randValue = rand() % 27;

		// To make the space value 
		if (randValue == 26)
			key[i] = ' ';
		// To make the char for key with ASCII
		else {
			randValue = randValue + 65;
			numberChar = randValue;
			key[i] = numberChar;
		}
	}

	key[length] = '\n';

	// Std out the key
	printf("%s", key);
	fflush(stdout);

	return EXIT_SUCCESS;
}