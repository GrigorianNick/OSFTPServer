#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

/*void ReadXBytes(int socket, unsigned int x, void* buffer)
{
	int bytesRead = 0;
	int result;
	while (bytesRead < x)
	{
		result = read(socket, buffer + bytesRead, x - bytesRead);
		if (result < 1 )
		{
			// Throw your error.
		}

		bytesRead += result;
	}
}*/

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cout << "No port provided." << endl;
		exit(1);
	}

	int sock, newsock, port, msg_len;
	char msg[256];
	memset(&msg[0], 0, sizeof(msg));
	sock = socket(AF_INET, SOCK_STREAM, 0); // Open a socket
	if (sock < 0)
	{
		cout << "Could not open socket." << endl;
		exit(1);
	}

	port = atoi(argv[1]); // Set up initial port

	// Set up server socket options
	socklen_t client_len;
	struct sockaddr_in serv_addr, cli_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	// Voodoo lives here
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	// Try to bind the socket
	if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		cout << "Could not bind socket." << endl;
		exit(1);
	}
	cout << "starting to listen..." << endl;
	// Socket binding success, now we enter the listen loop.
	listen(sock,10); // Conga line!
	cout << "finished listening!" << endl;

	// We now have a message.
	client_len = sizeof(cli_addr);
	cout << "Accepcting socket." << endl;
	newsock = accept(sock, (struct sockaddr *) &cli_addr, &client_len);
	if (newsock < 0)
	{
		cout << "Could not accecpt." << endl;
		exit(1);
	}
	cout << "Accecpted socket." << endl;

	printf("accepted a connection from client IP %s port %d \n",inet_ntoa(cli_addr.sin_addr),ntohs(serv_addr.sin_port));

	send(newsock,"\n",strlen("\n"),0); // Let the client know we're listening

	cout << "starting to read." << endl;
	msg_len = read(newsock,msg,255); // This should be a USER request
	cout << "Done reading." << endl;
	send(newsock,"230\n\n", strlen("230\n\n"), 0); // Accecpt all USER logins.
	if (msg_len < 0)
	{
		cout << "Could not read from socket." << endl;
		exit(1);
	}
	// Enter read loop.
	while (strncmp(msg,"QUIT", 4) != 0) {
		
		// Print out message for debugging purposes.
		cout << strncmp(msg,"QUIT", 4) << ": " << "msg:" <<  msg;

		// Wait for and read next message.
		client_len = sizeof(cli_addr);
		if (newsock < 0) {
			cout << "Could not accecpt." << endl;
			exit(1);
		}
		memset(&msg[0], 0, sizeof(msg)); // Need to init to 0 so msg plays nice with strcmp
		msg_len = read(newsock,msg,255);
		if (msg_len < 0)
		{
			cout << "Could not read from socket." << endl;
			exit(1);
		}
	}
	close(sock);
	close(newsock);
}
