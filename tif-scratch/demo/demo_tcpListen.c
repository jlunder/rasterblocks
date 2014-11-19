#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()

#define MSG_MAX_LEN 1024
#define PORT		22110
#define MAXPENDING  5 

static void dieError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

static void handleMessage(int clntSock){
	// Buffer to hold packet data:
	char message[MSG_MAX_LEN];
	FILE *fp;

	fp = fopen("config.json", "w");
	int bytesRx = recv(clntSock, message, MSG_MAX_LEN, 0);
	message[bytesRx] = 0;
	printf("initial message received: %s \n", message);
	if (bytesRx < 0) {
		dieError("failed to receive initial bytes");
	}
	while(bytesRx > 0) {
		fwrite(message, sizeof(char), bytesRx, fp);
		/* Send back received data */
		if (send(clntSock, message, bytesRx, 0) != bytesRx) {
			dieError("Failed to send bytes to client");
		}
		/* Check for more data */
		if ((bytesRx = recv(clntSock, message, MSG_MAX_LEN, 0)) < 0) {
		    dieError("Failed to receive additional bytes from client");
		}
	}
	fclose(fp);
	close(clntSock);
}

int main()
{
	printf("Net Listen Test on TCP port %d:\n", PORT);

	// Address
	struct sockaddr_in sin;
	struct sockaddr_in clientsin;
	memset(&sin, 0, sizeof(sin));
	memset(&clientsin, 0, sizeof(clientsin));
	sin.sin_family = AF_INET;                   // Connection may be from network
	sin.sin_addr.s_addr = htonl(INADDR_ANY);    // Host to Network long
	sin.sin_port = htons(PORT);                 // Host to Network short
	
	// Create the socket for TCP
	int socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);
	int clntSock;

	// Bind the socket to the port (PORT) that we specify
	if(bind (socketDescriptor, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
        dieError("bind() failed");
	}

	// Listen for incoming connections
	if (listen(socketDescriptor, MAXPENDING) < 0) {
        dieError("listen() failed");
	}

	while (1) {
		// Get the data (blocking)
		// Will change sin (the address) to be the address of the client.
		// Note: sin passes information in and out of call!
		//unsigned int sin_len = sizeof(sin);
		unsigned int client_len = sizeof(clientsin);

		if ((clntSock = accept(socketDescriptor, (struct sockaddr *) &clientsin, &client_len)) < 0) {
			dieError("accept() failed");
		}
		fprintf(stdout, "Client connected."); // inet_ntoa(clientsin.sin_addr));

		handleMessage(clntSock);

		// Make it null terminated (so string functions work):
		// NOTE: Unsafe in some cases; why?
		/*message[bytesRx] = 0;
		printf("Message received (%d bytes): \n\n'%s'\n", bytesRx, message);
		
		// Extract the value from the message:
		// (Process the message any way your app requires).
		int incMe = atoi(message);

		// Compose the reply message (re-using the same buffer here):
		// (NOTE: watch for buffer overflows!).
		sprintf(message, "Math: %d + 1 = %d\n", incMe, incMe + 1);

		// Transmit a reply:
		sin_len = sizeof(sin);
		sendto( socketDescriptor,
				message, strlen(message),
				0,
				(struct sockaddr *) &sin, sin_len);*/
	}

	// Close
	close(socketDescriptor);
	
	return 0;
}
