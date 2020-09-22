//Name: Myung Jin Song
//Description: cs344 assignment 4
//recieves texts from otp_dec.c and returns decrypted plaintext
	//decryption of text done here

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[999999];
	//char buffer1[300000];
	struct sockaddr_in serverAddress, clientAddress;
	pid_t spawnPid = -5;
    int childExitStatus = -5;

if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

// Set up the address struct for this process (the server)
memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
serverAddress.sin_family = AF_INET; // Create a network-capable socket
serverAddress.sin_port = htons(portNumber); // Store the port number
serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

// Set up the socket
listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
if (listenSocketFD < 0) error("ERROR opening socket");

// Enable the socket to begin listening
if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
	error("ERROR on binding");
listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

//fork off different processes and allow multiple connections 
while(1){
	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0) error("ERROR on accept");

    spawnPid = fork();
    switch(spawnPid){
   		case -1: { perror("Hull Breach!\n"); exit(1); break; }
      	case 0: {
	      
			// Get cipherText from otp_enc
			memset(buffer, '\0', 999999);

			int bufLen; // Holds the buffer length.
			int bufSum = 0; // The number of chars we have writen to our buffer
			do {
			charsRead = recv(establishedConnectionFD, &buffer[bufSum], sizeof(buffer) - 1, 0); // Read the client's message from the socket
			bufSum += charsRead; // Get the number of chars read and add it to our sum.
			bufLen = strlen(buffer); // Get the current length of the string in the buffer.
			//printf("passing by! 1\n");
			if (charsRead < 0) error("ERROR reading from socket1345");
			} while (strstr(buffer,"@@") == NULL);
			//while (buffer[bufLen] != '\0'); // Keep reading until we find the @ terminating char.

			int plz;
			for (plz=0; plz<strlen(buffer);plz++){
				//change all spaces to "[" for sscanf 
				if((int)buffer[plz] == 32){
					buffer[plz] = '[';
					// printf("space hit buffer \n");
				}
				//printf("im the buffer..%c\n",buffer[plz]);
			}

			char cipherText[300000];
			memset(cipherText, '\0', 300000);

			char keyText[300000];
			memset(keyText, '\0', 300000);

			char type[10];

			int numOfLetters = 0;
			int numOfKeys = 0;

			//sscanf(buffer,"%[^<]<%s", cipherText, keyText);
			sscanf(buffer,"%[^<]<%[^<]<%s", cipherText, keyText,type);

			numOfLetters = (int) strlen(cipherText);
			numOfKeys = (int) strlen(keyText);


			//check to make sure otp_enc is connecting and not otp_dec
			if(strcmp(type, "otp_dec@@") != 0){
				//printf("plaintext size11 %i\n", numOfLetters);
				//printf("key size11 %i\n", numOfKeys);
				fprintf(stderr,"Error: otp daemons do not match!\n");
				exit(2);

			}

			//START ENCRYPTION!
			if (numOfKeys < numOfLetters){
				printf("Error: key '%s' is too short\n", argv[2]);
				exit(1);
			}

			char plainText[300000];
			int i;
			for (i=0; i < numOfLetters; i++){


				cipherText[i] = cipherText[i] - 65;
				keyText[i] = keyText[i] - 65;
				plainText[i] = cipherText[i] - keyText[i];
				if((int)plainText[i] < 0){
					plainText[i] = plainText[i] + 27;
				}

				plainText[i] = plainText[i] + 65;
				if((int)plainText[i] == 91){
					plainText[i] = ' ';
				}
				plainText[numOfLetters] = '\0';
			}

			// Send ciphertext to otp_enc
			charsRead = send(establishedConnectionFD, plainText, strlen(plainText), 0); // Send success back	
			if (charsRead < 0) error("ERROR writing to socket");

			int checkSend = -5;  // Holds amount of bytes remaining in send buffer
			do
			{
			ioctl(establishedConnectionFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
			//printf("checkSend3: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
			}
			while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
			if (checkSend < 0) error("ioctl error");  // Check if we actually stopped the loop because of an error

			close(establishedConnectionFD); // Close the existing socket which is connected to the client
			close(listenSocketFD); // Close the listening socket
			exit(0); break;
		}
		default: {
			//pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
        	//printf("PARENT(%d): Child(%d) terminated, Exiting!\n", getpid(), actualPid);
        	//exit(1); break;
        	close (establishedConnectionFD);
		}
	}
}
close(listenSocketFD);
return 0; 
}
