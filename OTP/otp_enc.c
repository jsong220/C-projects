//Name: Myung Jin Song
//Description: cs344 assignment 4
//Processes plaintext and key text files to send message for encryption
	//for message relay, no encryption is done here

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[999999];
//char buffer1[70000];

if (argc < 4) { fprintf(stderr,"USAGE: %s myplaintext mykey port\n", argv[0]); exit(0); } // Check usage & args

//Pulling text information from plaintext files to send to otp_enc_d
int numOfLetters=0;
char text[100000]; //storing plaintext in here
int index = 0;
int myChar;
FILE* fPtr = fopen(argv[1],"r");

//error if file did not open properly
if(fPtr == NULL ){
	printf("error opening file \n");
}
//printf("file opened \n");

//works cited - https://cboard.cprogramming.com/c-programming/70039-strcpy-how-copy-single-character-string.html
while((myChar = fgetc(fPtr)) != EOF){
	text[index++] = myChar;
	numOfLetters = numOfLetters + 1;
		text[index] = '\0';   //to terminate string
	}

text[strcspn(text, "\n")] = '\0'; //get rid of newline added by fgetc

//check for invalid messages
int c;
for (c=0; c < numOfLetters-1; c++){
	if( (int)text[c] > 91 || ((int)text[c] < 65 && (int)text[c] != 32)){
		fprintf(stderr, "Error: %s contains invalid chars!\n", argv[1] );
		exit(1);
	}
}

int numOfKeys=0;
char keyText[100000];
int keyIndex = 0;
int keyChar;

FILE* keyPtr = fopen(argv[2],"r");

if(keyPtr == NULL ){
	printf("error opening file \n");
}

while((myChar = fgetc(keyPtr)) != EOF){
	keyText[keyIndex++] = myChar;
	numOfKeys = numOfKeys + 1;
	keyText[keyIndex] = '\0';   //to terminate string
}
keyText[strcspn(keyText, "\n")] = '\0'; //get rid of newline added by fgetc

//if key is too short, exit
if (numOfKeys < numOfLetters){
	fprintf(stderr,"Error: key '%s' is too short\n", argv[2]);
	exit(1);
}

//combining both text and key for easier send management
char fullMsg[300000];
char type[10] = "otp_enc@@";
memset(fullMsg, '\0', sizeof(fullMsg)); // Clear out the address struct
sprintf(fullMsg, "%s<%s<%s", text, keyText,type); //the "<" used to seperate each text

//close files
fclose(fPtr);
fclose(keyPtr);

// Set up the server address struct
memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
serverAddress.sin_family = AF_INET; // Create a network-capable socket
serverAddress.sin_port = htons(portNumber); // Store the port number
serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

// Set up the socket
socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
if (socketFD < 0) error("CLIENT: ERROR opening socket");

// Connect to server
if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
	error("CLIENT: ERROR connecting");

charsWritten = send(socketFD, fullMsg, strlen(fullMsg), 0);	
if (charsWritten < 0) error("ERROR writing to socket");

int checkSend = -5;  // Holds amount of bytes remaining in send buffer
do
{
ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
//printf("checkSend1: %d\n", checkSend);  // Out of curiosity, check how many remaining bytes there are
}
while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
if (checkSend < 0) error("ioctl error");  // Check if we actually stopped the loop because of an error

int bufLen; // Holds the buffer length.
int bufSum = 0; // The number of chars we have writen to our buffer
do {
	charsRead = recv(socketFD, &buffer[bufSum], sizeof(buffer) - 1, 0); // Read the client's message from the socket
	bufSum += charsRead; // Get the number of chars read and add it to our sum.
	bufLen = strlen(buffer); // Get the current length of the string in the buffer.
	if (charsRead < 0) error("ERROR reading from socket13456");
} while (buffer[bufLen] != '\0'); // Keep reading until we find the terminating char.


//printf("testing!! %i\n", testme);
int k;
for (k=0; k < numOfLetters-1; k++){
	printf("%c", buffer[k]);
}
printf("\n");

close(socketFD); // Close the socket
return 0;
}