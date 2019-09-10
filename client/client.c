// @Author: David Heck (dheck@udel.edu)
// Client Side Implementation Of UPD File Transfer with Go-Back-N pipeline
// Includes
#include <stdlib.h> 
#include <stdio.h>
#include <string.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/types.h>
#include <unistd.h> 
#include <arpa/inet.h>
#include <time.h>
#include <mhash.h> 

// Constants
#define MAX 512 // Maximum buffer size
#define SA struct sockaddr

// Function called after connection is established, transfers specified files from server to client until server gets an exit message. Uses Go-Back-N Pipeline protocol.
void gbnFileTransfer(int sockfd, struct sockaddr_in serverAddress){ 
	// Init Variables
	char currWindow[MAX]; // Buffer used to store each packet in the window received from server. Is later used to write data to file.
	char messageBuffer[MAX]; // buffer used to transmit messages between client and server
	char fileName[MAX]; // local storage for filename
	int length; // Length of UDP message
	int n; // Used for index identification for adding endline character from received messages
	clock_t begin, end; // Used to measure transfer time.
	double runTime; // Used to determine transfer time.
	
	// Infinite loop for communication between client and server
	while (1){
		// Reset messageBuffer
		bzero(messageBuffer, sizeof(messageBuffer));
		
		// Get user input for file name, send input to server
		printf("CLIENT: Enter the filename you wish to download, or type 'exit' to close connection: "); 
		scanf("%s", messageBuffer);	
		memcpy(fileName, messageBuffer, sizeof(messageBuffer)); // Keep local copy of (potential) filename
		
		// Send filename to server
		sendto(sockfd, messageBuffer, MAX, 0, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));

		// If message sent is "exit" wait for server to respond with exit message.
		if ((strncmp(messageBuffer, "exit", 4)) == 0) {
			// Reset messageBuffer, read returned exit message from server
			bzero(messageBuffer, sizeof(messageBuffer)); 
			recvfrom(sockfd, messageBuffer, sizeof(messageBuffer), 0, (struct sockaddr *)&serverAddress, &length); // Read message returned from server
			
			// If server message is "exit", close connection
			if ((strncmp(messageBuffer, "exit", 4)) == 0) {
				printf("SERVER: Closing connection. \n");
				printf("CLIENT: Exiting...\n"); 
				break;
			}
			else{
				printf("CLIENT: ERROR Server did not acknowledge exit. Force closing connection... \n");
				exit(0);
			}
		}
		
		// Reset messageBuffer, wait for OK message to confirm file existance.
		bzero(messageBuffer, sizeof(messageBuffer));
		n = recvfrom(sockfd, messageBuffer, MAX, 0, (struct sockaddr *)&serverAddress, &length);
		messageBuffer[n] = '\0';

		// If server finds file start recieveing it.
		if ((strncmp(messageBuffer, "OK", 2)) == 0) {

			// Print OK message
			printf("SERVER: %s\n", messageBuffer);

			// Send ok message back to client to begin data transmission
			bzero(messageBuffer, sizeof(messageBuffer));
			sendto(sockfd, "OK", sizeof("OK"), 0, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
			
			// Receive File from Server
			printf("CLIENT: Receiveing %s from Server and saving it. \n", fileName);
			
			// Create file in write mode
			FILE *clientFile = fopen(fileName, "w"); 
			
			// If the file is null something went wrong, else download file from server
			if(clientFile == NULL){
				printf("CLIENT: ERROR File %s cannot be opened. \n", fileName);
			}
			else{
				// Reset Buffer, init client variables
				bzero(messageBuffer, MAX);
				int transferFlag = 1; // Flag used to continuously receive data 
				int packetSize = 0; // Size of each packet received from server
				int lastAck = -1; // Int of last received acknowledge. Set to -1 for no acks.
				int lastSegment; // Int of last sent segnment number.
				char localCRC32[8]; // Will store the locally computed CRC32 value, to be compared to expectedCRC32.
				char expectedCRC32[8]; // Will store the received (expected) CRC32 value.
				unsigned int crc32 = 0; // To store HEX CRC32 val
				MHASH td; // MHASH - Used for Checking Bit Errors

				begin = clock(); // Start Clock

				// Transfer File From Server To Client (Go-Back-N)
				while(transferFlag == 1){
					// Reset File Buffer
					bzero(currWindow, sizeof(currWindow));

					// Receive the Segment Number from server
					bzero(messageBuffer, sizeof(messageBuffer));
					n = recvfrom(sockfd, messageBuffer, MAX, 0, (struct sockaddr *)&serverAddress, &length);
					messageBuffer[n] = '\0';
					printf("CLIENT: Receiveing segment %s \n", messageBuffer);

					// Set last received segment number
					lastSegment = atoi(messageBuffer);

					// Receive Packet, store in currWindow.
					packetSize = recvfrom(sockfd, currWindow, sizeof(currWindow), 0, (struct sockaddr *)&serverAddress, &length);

					// Extract (unpack) Appened CRC32 Value
					memcpy(expectedCRC32, &currWindow[packetSize - 8], 8);
					expectedCRC32[8] = '\0';

					// Compute Checksum CRC32 value of input data.
					td = mhash_init(MHASH_CRC32);
					mhash(td, currWindow, packetSize - 8);
					mhash_deinit(td, &crc32);

					// Convert CRC32 Value to String and Store in localCRC32
					sprintf(localCRC32, "%x", crc32);
				
					// If there are no gaps in acknowledgements.
					if(lastAck == lastSegment - 1){
						// Check for bit errors. Make sure local CRC32 value equals expected CRC32 value, if not bit error detected.
						if(strcmp(localCRC32, expectedCRC32) == 0){
							lastAck = lastSegment; // Update acknowledgement number if no bit error detected

							// Only write the currently acknowledged data to local file. (Ignore unacknowledged packets and appended checksum)
							fwrite(currWindow, sizeof(char), packetSize - 8, clientFile);
						}
						else{
							printf("CLIENT: Bit error detected - Expected CRC32 Value of %s but got %s instead! Ignoring packets..\n", expectedCRC32, localCRC32);
						}
					}
					
					// Send acknowledgement for last successfully received segment.
					bzero(messageBuffer, sizeof(messageBuffer));
					sprintf(messageBuffer, "%d", lastAck); 
					sendto(sockfd, messageBuffer, sizeof(messageBuffer), 0, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));
					printf("CLIENT: Sending acknowledge %s \n", messageBuffer);

					// Break out of while loop if last packet has been received and ack'd.
					if (packetSize == 0 || packetSize < MAX){
						if(lastAck == lastSegment){
							transferFlag = 0;
							break;
						}
					}
				}
			}
			// End timer and store value in runTime.
			end = clock(); 
			runTime = (double)(end - begin) / CLOCKS_PER_SEC;

			// Display success message and close File
			printf("CLIENT: File received from server in %f seconds! \n", runTime);
			fclose(clientFile); // Close File
		}
		else {
			// Else if no OK is received file is not on server.
			printf("SERVER: File %s not found on server, please try another file. \n", fileName);
		}
	}
	// Close Connection
	close(sockfd);
}

// Main Function w/ Argument Fields -- Establishes TCP Connection to Specified IP + Port
int main(int argc, char* argv[]){
	// Init Variables
	char* ipAddress; // Destination IP Address
	char* port; // Destination Port
	int portVal; // Used to convert PORT to an int
	int clientSocket; // Socket for the client
	struct sockaddr_in serverAddress; // Socket address for server
	
	// Set defaults based on how many arguments are supplied. (e.g. set port to 8080 if only IP is passed in)
	if (argv[1] == NULL){
		ipAddress = "127.0.0.1";
		port = "8080";
	}
	else if(argv[2] == NULL){
		ipAddress = argv[1];
		port = "8080";
	}
	else{
		ipAddress = argv[1];
		port = argv[2]; 
	}
	
	// Convert PORT string to int value.
	portVal = atoi(port);
	
	// Create the client's socket, and check for successful creation
	clientSocket = socket(AF_INET, SOCK_DGRAM, 0); 
	if (clientSocket == -1) { 
		printf("CLIENT: Socket creation failed. \n"); 
		exit(0); 
	} 
	else{
		printf("CLIENT: Socket creation successful. \n"); 
	}
	
	// Allocate space for server address.
	bzero(&serverAddress, sizeof(serverAddress)); 
	
	// Assign the IP and Port Number for the destination address.
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_addr.s_addr = inet_addr(ipAddress); 
	serverAddress.sin_port = htons(portVal); 

	// Initiate file transfer over UDP.
	gbnFileTransfer(clientSocket, serverAddress); 
	
	// Close Client Socket (end program).
	close(clientSocket); 
	return 0; // End Main
} 