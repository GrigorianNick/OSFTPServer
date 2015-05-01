/*

Nicholas Grigorian - ngg3vm

Under no circumstances should this be run as a serious ftp server. There is no
security beyond the restrictions place on the user that started the server.

This is the backbone of the server. This .cpp is responsible for establishing
and closing all connections to the client machine. The dataflow is really
simple: after the inital connection is established it enters a loop where
messages are read. The first couple of characters are examined to figure out
what the client wants, then the command is either executed or rejected.

Commands that work:
 - USER
 - PASS
 - LIST
 - PORT
 - CWD
 - CDUP
 - CDW
 - DELE
 - QUIT
Commands that sorta work:
 - SYST

Changelog:

 - April 24:
 	Got skeleton up and running. Msg loop works, client can connect. USER and
 	LIST aren't working.
 - April 25
 	USER now works. PASS doesn't. Use `ftp -n <host> <port>` to connect
 - April 26
 	PWD and CD family working. LIST is non-functional.
 - April 27
 	Fixed QUIT, LIST a little less broken. Added a makefile.
 - April 28
 	Fixed LIST .
 - April 29
 	Added RETR and STOR functionality, created functions.h. Added TYPE I
 	requirements as well as swapping back to TYPE A for LIST
 - April 30
 	Fixed some minor bugs.
 	Added ls(int client_sock, string msg)
 */
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "functions.h"

using namespace std;

int main(int argc, char *argv[])
{
	bool binary_format = false;
	if (argc < 2)
	{
		cout << "No port provided." << endl;
		exit(1);
	}

	int sock, newsock, client_sock, port, msg_len;
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
	struct sockaddr_in serv_addr, cli_addr, data_addr;
	struct hostent *client;
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
	//cout << "starting to listen..." << endl;
	// Socket binding success, now we enter the listen loop.
	listen(sock,10); // Conga line!
	//cout << "finished listening!" << endl;

	// We now have a message.
	client_len = sizeof(cli_addr);
	newsock = accept(sock, (struct sockaddr *) &cli_addr, &client_len);
	if (newsock < 0)
	{
		cout << "Could not accecpt." << endl;
		exit(1);
	}

	printf("accepted a connection from client IP %s port %d \n",inet_ntoa(cli_addr.sin_addr),ntohs(serv_addr.sin_port));

	int return_status;
	return_status = 220;
	return_status = htons(return_status);
	send(newsock, "\n", sizeof("\n"), 0); // Magic.


	// Enter read loop.
	do {
		memset(&msg[0], 0, sizeof(msg)); // Need to init to 0 so msg plays nice with strcmp
		msg_len = recv(newsock,msg,255,0);
		if (msg_len < 0)
		{
			cout << "Could not read from socket." << endl;
			exit(1);
		}

		// We have a new message
		cout << "msg: " << msg;
		if (!binary_format && !strncmp(msg, "TYPE I", 6)) // Switching to binary mode
		{
			binary_format = true;
			send(newsock, "200 binary mode engage!\r\n", sizeof("200 binary mode engage!\r\n"), 0);
			continue;
		}
		else if (!strncmp(msg, "TYPE A", 6)) // Switching back to ASCII. Used for LIST
		{
			binary_format = false;
			send(newsock, "200 binary mode disengaged!\r\n", sizeof("200 binary mode disengaged!\r\n") - 1, 0);
			continue;
		}
		
		if (strncmp(msg, "USER", 4) == 0) // They're trying to log in.
		{
			send(newsock, "331 Need password (any will do)\r\n", sizeof("331 Need password (any will do)\r\n"), 0);
		}
		else if (strncmp(msg, "PASS", 4) == 0) // They're sending us a password.
		{
			// We take all passwords because security lol
			send(newsock, "230 Welcome to Elysium!\r\n", sizeof("230 Welcome to Elysium!\r\n"), 0);
		}
		else if (strncmp(msg, "SYST", 4) == 0) // The client's creeping on us
		{
			send(newsock, "215 Custom\r\n", sizeof("215 Custom\r\n"), 0);
		}
		else if (strncmp(msg, "CDUP", 4) == 0) // Move up a directory by following ..
		{
			if (chdir("..") == 0) send(newsock, "250 CDUP Successful\n", sizeof("250 CDUP Successful\n"), 0);
			else send(newsock, "550 Failed CDUP\n", sizeof("550 Failed CDUP\n"), 0);
		}
		else if (strncmp(msg, "CWD", 3) == 0) // Move around the directory structure
		{
			string arg = "";
			for (int i = 4; i < strlen(msg) - 2; i++)
			{
				arg += msg[i];
			}
			//cout << "CWD: " << arg << endl;
			if (chdir(arg.c_str()) == 0) send(newsock, "250 CD Successful\n", sizeof("250 CD Successful\n"), 0);
			else send(newsock, "550 Failed to change directory.\n", sizeof("550 Failed to change directory.\n"), 0);
		}
		else if (strncmp(msg, "PWD", 3) == 0) // Give them the current working directory
		{
			char cwd[1024];
			bzero(cwd, 1024);
			getcwd(cwd, 1024);
			send(newsock, cwd, sizeof(cwd), 0);
			send(newsock, "\n", sizeof("\n"), 0);
		}
		else if (strncmp(msg, "PORT", 4) == 0) // They're trying to initiate a data connection. Handled here because it's just easier.
		{
			// Interpret the ip and port
			string arg = "",
				   ip_str = "",
				   port_str = "";
			int data_port;
			for (int i = 5; i < strlen(msg); i++)
			{
				arg += msg[i];
			}
			int commas = 0, j = 0;
			
			while (commas < 4)
			{
				if (arg[j] == ','){
					commas++;
					if (commas > 3) {
						j++;
						break;
					}
					ip_str += ".";
				}
				else
				{
					ip_str += arg[j];
				}
				j++;
			}
			while (arg[j] != ',')
			{
				port_str += arg[j];
				j++;
			}
			j++;
			data_port = 256 * atoi(port_str.c_str());
			port_str = "";
			while (j < arg.length()) {
				port_str += arg[j];
				j++;
			}
			data_port += atoi(port_str.c_str());

			// Establish data connection
			client_sock = socket(AF_INET, SOCK_STREAM, 0);
			if (client_sock < 0)
			{
				cout << "Could not open client socket." << endl;
				exit(1);
			}
			client = gethostbyname(ip_str.c_str());
			if (client == NULL)
			{
				cout << "Client doesn't exist. Somehow." << endl;
				exit(1);
			}
			bzero((char *) &data_addr, sizeof(data_addr));
			data_addr.sin_family = AF_INET;
			bcopy((char *)client->h_addr,
			      (char *)&data_addr.sin_addr.s_addr,
			      client->h_length);
			data_addr.sin_port = htons(data_port);
			if (connect(client_sock, (struct sockaddr *) &data_addr, sizeof(data_addr)) <0)
			{
				cout << "Couldn't connect to the client." << endl;
			}

			send(newsock, "200 PORT Successful\r\n", sizeof("200 PORT Successful\n"), 0);

		}
		else if (strncmp(msg, "LIST", 4) == 0) // List current directory
		{
			string arg = "";
			bool is_arg = 5 < strlen(msg) - 2;
			send(newsock, "150 Sending info\r\n", sizeof("150 Sending info\r\n"), 0);
			if (is_arg) ls(client_sock, msg);
			else ls(client_sock);
			//ls(client_sock);
			close(client_sock);
			send(newsock, "226 LIST done\r\n", sizeof("226 LIST done\r\n"), 0);
		}
		else if (strncmp(msg, "STOR", 4) == 0) // Put a file on the server. Needs to be in binary mode to work.
		{
			if (binary_format)
			{
				send(newsock, "150 Ready to receive file\r\n", sizeof("150 Ready to receive file\r\n"), 0);
				store(client_sock, msg);
				close(client_sock);
				send(newsock, "226 File received\r\n", sizeof("226 File received\r\n"), 0);
			}
			else
			{
				send(newsock, "451 Not in Type I (image) mode!\r\n", sizeof("451 Not in Type I (image) mode!\r\n") - 1, 0);
				close(client_sock); // Need to close socket so the client's data connection doesn't hang.
			}
		}
		else if (strncmp(msg, "RETR", 4) == 0) // Put a file on the client. Needs to be in binary mode to work.
		{
			if (binary_format)
			{
				send(newsock, "150 Sending file\r\n", sizeof("150 Sending file\r\n"), 0);
				retrieve(client_sock, msg);
				close(client_sock);
				send(newsock, "226 File sent\r\n", sizeof("226 File sent\r\n"), 0);
			}
			else
			{
				send(newsock, "451 Not in Type I (image) mode!\r\n", sizeof("451 Not in Type I (image) mode!\r\n") - 1, 0);
				close(client_sock); // Need to close socket so the client's data connection doesn't hang.
			}
		}
		else if (strncmp(msg, "DELE", 4) == 0) // Delete a file on the server. No extra permissions have been implemented for this.
		{
			string arg = "";
			for (int i = 5; i < strlen(msg) - 2; i++) {
				arg += msg[i];
			}
			if (remove(arg.c_str()) == 0) send(newsock, "250 File deleted\n", sizeof("250 File deleted\n"), 0);
			else send(newsock, "553 could not delete file\r\n", sizeof("553 could not delete file\r\n"), 0);
		}
		else if (strncmp(msg, "QUIT", 4) == 0) // We're quitting out. Exit the loop so the sockets can be closed.
		{
			close(newsock);
			// Socket binding success, now we enter the listen loop.
			listen(sock,10); // Conga line!
			//cout << "finished listening!" << endl;

			// We now have a message.
			client_len = sizeof(cli_addr);
			newsock = accept(sock, (struct sockaddr *) &cli_addr, &client_len);
			if (newsock < 0)
			{
				cout << "Could not accecpt." << endl;
				exit(1);
			}

			printf("accepted a connection from client IP %s port %d \n",inet_ntoa(cli_addr.sin_addr),ntohs(serv_addr.sin_port));


			send(newsock, "\n", sizeof("\n"), 0); // Magic.
		}
		else // They're trying to do something I don't support.
		{
			send(newsock, "502 Command not implemented\r\n", strlen("502 Command not implemented\r\n") - 1, 0); // Don't support
		}
	} while (1);
	// Clean up after ourselves.
	close(sock);
	close(newsock);
	exit(0);
}
