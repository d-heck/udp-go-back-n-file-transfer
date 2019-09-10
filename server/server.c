// @Author: David Heck (dheck@udel.edu)
// Server Side Implementation Of UPD File Transfer with Go-Back-N pipeline

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
#include <math.h>
#include <time.h>
#include <mhash.h>

// Constants
#define MAX 512 // Maximum buffer size
#define SA struct sockaddr

// Function called after connection is established, transfers specified files from server to client until server gets an exit message. Uses Go-Back-N Pipeline protocol.
void gbnFileTransfer(int sockfd, struct sockaddr_in clientAddress, int windowSize, float errorProbability){ 	
	// Init Variables
	char messageBuffer[MAX]; // buffer used to transmit messages between client and server
	char fileName[MAX]; // Used to store a local copy of the file name
	int length; // Length of UDP message
	int n; // Used for index identification
	clock_t begin, end; // Used to measure transfer time.
	double runTime; // Used to determine transfer time.
	
	// Infinite loop for communication between client and
	while(1) { 
		// Reallocate buffer with MAX buffer size
		bzero(messageBuffer, MAX); 
		
		length = sizeof(clientAddress);

		// Read the message sent from CLIENT, copy to local buffer.
		n = recvfrom(sockfd, messageBuffer, sizeof(messageBuffer), 0, (struct sockaddr*) &clientAddress, &length);
		messageBuffer[n] = '\0';
		// print buffer which contains the client contents 
		printf("CLIENT: Requesting %s \n" , messageBuffer);
		
		// Terminate connection if client requests to exit.
		if ((strncmp(messageBuffer, "exit", 4)) == 0) {
			// Send exit message backt to client
			sendto(sockfd, "exit", MAX, 0, (const struct sockaddr*) &clientAddress, sizeof(clientAddress));
			printf("SERVER: Exiting...\n"); 
			break; 
		}

		// Attempt to open specified file
		memcpy(fileName, messageBuffer, sizeof(messageBuffer)); // Keep local copy of (potential) filename
		FILE* serverFile = fopen(fileName, "r"); // Open file in read mode, store as serverFile

		// If the file exists on the server
		if(serverFile != NULL){
			// Inform client file exists
			sendto(sockfd, "OK", sizeof("OK"), 0, (const struct sockaddr*) &clientAddress, sizeof(clientAddress));

			// Reset messageBuffer, wait for client to acknowledge the OK message
			bzero(messageBuffer, sizeof(messageBuffer));
			n = recvfrom(sockfd, messageBuffer, sizeof(messageBuffer), 0, (struct sockaddr*) &clientAddress, &length);
			messageBuffer[n] = '\0';

			// If read message is OK send file
			if ((strncmp(messageBuffer, "OK", 2)) == 0) {

				// Init Variables Server Side
				int transferFlag = 1; // Flag used to break out of while loop
				int packetSize; // Size of each packet being sent
				int lastAck = -1; // Int of last received acknowledge. Set to -1 for no acks.
				int lastSegment; // Int of last sent segnment number.
				int j = 0; // Value used to "Slide" window (Go-Back-N)
				clock_t timeoutStart, timeoutEnd; // Used for timeout timing (Go-Back-N)
				double timeOut; // Used to determine transfer time.
				unsigned int crc32 = 0; // To store HEX CRC32 val
				MHASH td; // MHASH - Used for Checking Bit Errors

				begin = clock(); // Start File Transfer Clock
				
				// Reset messageBuffer
				bzero(messageBuffer, sizeof(messageBuffer)); 
				printf("SERVER: Sending file %s to Client... \n", fileName);

				// Continuously pipeline file contents following Go-Back-N protocol until EOF
				while(transferFlag == 1){

					// Send up to N Unacked Packets
					for(int i = 0; i < windowSize; i++){

						// Send Current Segment's Number
						bzero(messageBuffer, sizeof(messageBuffer));
						sprintf(messageBuffer, "%d", i + j); // i + j = current possition
						sendto(sockfd, messageBuffer, sizeof(messageBuffer), 0, (const struct sockaddr*) &clientAddress, sizeof(clientAddress));

						// Update Last Segement Number
						lastSegment = atoi(messageBuffer); 
						printf("SERVER: Sending segment %s \n", messageBuffer);

						// Set file read head to the bit location of the last ACK'd packet. (Maintain window)
						fseek(serverFile, (sizeof(char) * (MAX - 8) * (i + j)), SEEK_SET); // Offset max by 8 bits for size of checksum and '\0'

						// Reset Message Buffer and read in packet from server file
						bzero(messageBuffer, MAX);
						packetSize = fread(messageBuffer, sizeof(char), sizeof(messageBuffer) - 8, serverFile); // Save 8 bits for CRC32 append

						// Compute Checksum CRC32 value
						td = mhash_init(MHASH_CRC32);
       					mhash(td, messageBuffer, packetSize);
						mhash_deinit(td, &crc32);

						// Scramble the Checksum Value based on errorProbability
						float p = fabs(((float)rand())/RAND_MAX);
						if (errorProbability > p){
							// Randomly alter some bits
							for(int i = 0; i < 5; i++){
								crc32 += (rand() % 8 + 0) - 100;
							}
						}

						// Append CRC32 Checksum Value to end of Message (payload)
						sprintf(&messageBuffer[packetSize], "%x", crc32); // Will require 8 bits
						packetSize += 8; // Update packetsize

						// Send File
						if(sendto(sockfd, messageBuffer, packetSize, 0, (struct sockaddr*) &clientAddress, length) < 0){
							printf("SERVER: ERROR Failed to send file %s. Closing connection. \n", fileName);
							exit(0);
						}

						// Receive latest acknowledge from Client.
						bzero(messageBuffer, sizeof(messageBuffer));
						n = recvfrom(sockfd, messageBuffer, sizeof(messageBuffer), 0, (struct sockaddr*) &clientAddress, &length);
						messageBuffer[n] = '\0';

						// Update Last ACK value if client does not report bit errors. If not, ignore ACK.
						if (lastAck == atoi(messageBuffer) - 1){
							lastAck = atoi(messageBuffer);
						}

						printf("SERVER: Receiving acknowledge %d \n", lastAck);

						// Testing to make sure the num of unacked packets never exceeds windowsize
						//printf("SERVER: Unacked packets in flight %d \n", (i + j) - lastAck);

						// Break out of while loop if the final packet has been received and acknowledged. (Stop sending packets)
						if (packetSize == 0 || packetSize < MAX - 8){
							if(lastAck == lastSegment){
								transferFlag = 0;
								break;
							}
						}
					}

					// Slide Window to the past the last known ACK number, aka only trasnmit segments that haven't been ack'd last. 
					j = lastAck + 1;
				}

				// End timer and store value in runTime.
				end = clock(); 
				runTime = (double)(end - begin) / CLOCKS_PER_SEC;

				// Display success message
				printf("SERVER: File successfully sent to client in %f seconds! \n", runTime);
			}
		}
		else{
			// Else if file not on server -- print error, send NULL back to client. Wait for next filename from client.
			printf("SERVER: ERROR file %s not found on server. \n", fileName);
			sendto(sockfd, "NULL", sizeof("NULL"), 0, (struct sockaddr*) &clientAddress, length);
		}
	}
	
	// Close connection
	close(sockfd);
}

// Main Function w/ Argument Fields -- Establishes TCP Connection to Specified IP + Port
int main(int argc, char* argv[]){
	// Init Variables
	char* port; // Listening Port
	int portVal; // Used to convert PORT to an int
	int serverSocket; // Socket for server
	int connection; // Used in client acceptance
	struct sockaddr_in serverAddress; // Socket address in for server
	struct sockaddr_in clientAddress; // Socket address in for client
	int windowSize; // Used for Go-Back-N
	float errorProbability; // Used to determine the probability of bit errors

	// Random Seed
	srand(time(NULL));
	
	// Set port based command line argument, sets to 8080 if no port specified. 
	if (argv[1] == NULL){
		port = "8080";
	}
	else if (argv[2] == NULL){
		windowSize = 1;
	}
	else if (argv[3] == NULL){
		errorProbability = 0.0;
	}
	else{
		port = argv[1];
		windowSize = atoi(argv[2]);
		errorProbability = atof(argv[3]);		
	}

	// Ensure Probability is between 0 and 1
	if (errorProbability > 1){
		errorProbability = 1;
	}
	else if (errorProbability < 0){
		errorProbability = 0;
	}
	
	// Convert PORT string to int value.
	portVal = atoi(port);
	
	// Create the server's UDP socket, and check for successful creation
	serverSocket = socket(AF_INET, SOCK_DGRAM, 0); 
	if (serverSocket == -1) { 
		printf("SERVER: Socket creation failed. \n"); 
		exit(0); 
	} 
	else{
		printf("SERVER: Socket creation successful. \n");
	}
	
	// Allocate space for server address.
	bzero(&serverAddress, sizeof(serverAddress)); 
	
	// Assign the IP and Port Number for the Server address.
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Take any IP as in address (accept all clients)
	serverAddress.sin_port = htons(portVal); // Set port (user specified)
	
	// Bind server socket with server address.
	if ((bind(serverSocket, (SA*)&serverAddress, sizeof(serverAddress))) != 0) { 
		printf("SERVER: Socket binding failed. \n"); 
		exit(0); 
	} 
	else{
		printf("SERVER: Socket binding successful. \n"); 
	}
	
	// Initiate file transfer program over UDP.
	gbnFileTransfer(serverSocket, clientAddress, windowSize, errorProbability); 
	
	// Close Server Socket (end program)
	close(serverSocket);
	return 0; // End Main
}