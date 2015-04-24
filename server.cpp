#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

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
	// Socket binding success, now we enter the listen loop.
	listen(sock,10); // Conga line!

	// We now have a message.
	client_len = sizeof(cli_addr);
	newsock = accept(sock, (struct sockaddr *) &cli_addr, &client_len);
	if (newsock < 0) {
		cout << "Could not accecpt." << endl;
		exit(1);
	}
	msg_len = read(newsock,msg,255);
	if (msg_len < 0)
	{
		cout << "Could not read from socket." << endl;
		exit(1);
	}
	// Enter read loop.
	while (strcmp(msg,"QUIT\n") != 0) {
		// Reply to the message
		write(newsock, "I got your message, brah", 24);

		// Print out message for debugging purposes.
		cout << msg;
		memset(&msg[0], 0, sizeof(msg)); // Need to init to 0 so msg plays nice with strcmp

		// Wait for and read next message.
		listen(sock,10);
		client_len = sizeof(cli_addr);
		newsock = accept(sock, (struct sockaddr *) &cli_addr, &client_len);
		if (newsock < 0) {
			cout << "Could not accecpt." << endl;
			exit(1);
		}
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
