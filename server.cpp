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

using namespace std;

int main(int argc, char *argv[])
{
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

	int return_status;
	return_status = 220;
	return_status = htons(return_status);
	send(newsock, "\n", sizeof("\n"), 0); // Magic.


	// Enter read loop.
	do {
		// Wait for a new message
		/*cout << "Listening..." << endl;
		listen(newsock, 5);
		cout << "Heard something!" << endl;*/
		// We have a new message
		cout << "reading message" << endl;
		memset(&msg[0], 0, sizeof(msg)); // Need to init to 0 so msg plays nice with strcmp
		msg_len = recv(newsock,msg,255,0);
		cout << "Finished reading message." << endl;
		cout << "msg: " << msg;
		if (msg_len < 0)
		{
			cout << "Could not read from socket." << endl;
			exit(1);
		}
		
		if (strncmp(msg, "USER", 4) == 0) // Doesn't work..
		{
			return_status = 220; // Accecpt all USER logins
			return_status = htons(return_status);
			//send(newsock, "FTP 331\n", sizeof("FTP 331\n"), 0);
			send(newsock, (char *)&return_status, sizeof(return_status), 0);
			send(newsock, "\n", sizeof("\n"), 0);
			//send(newsock, "Hai", sizeof("Hai"), 0);
			cout << "Sent 331" << endl;
			// Send welcome message
			send(newsock, "Welcome to Elysium!", sizeof("Welcome to Elysium!"), 0);

		}
		else if (strncmp(msg, "SYST", 4) == 0)
		{
			cout << "Returning system type." << endl;
			return_status = 215;
			return_status = htons(return_status);
			send(newsock, (char *)&return_status, sizeof(return_status), 0);
			send(newsock, "Custom\n", sizeof("Custom\n"), 0);
		}
		else if (strncmp(msg, "CDUP", 4) == 0)
		{
			if (chdir("..") == 0) send(newsock, "250 CDUP Successful\n", sizeof("250 CDUP Successful\n"), 0);
			else send(newsock, "550 Failed CDUP\n", sizeof("550 Failed CDUP\n"), 0);
		}
		else if (strncmp(msg, "CWD", 3) == 0)
		{
			string arg = "";
			for (int i = 4; i < strlen(msg) - 2; i++)
			{
				arg += msg[i];
			}
			cout << "CWD: " << arg << endl;
			if (chdir(arg.c_str()) == 0) send(newsock, "250 CD Successful\n", sizeof("250 CD Successful\n"), 0);
			else send(newsock, "550 Failed to change directory.\n", sizeof("550 Failed to change directory.\n"), 0);
		}
		else if (strncmp(msg, "PWD", 3) == 0)
		{
			char cwd[1024];
			bzero(cwd, 1024);
			getcwd(cwd, 1024);
			send(newsock, cwd, sizeof(cwd), 0);
			send(newsock, "\n", sizeof("\n"), 0);
		}
		else if (strncmp(msg, "PORT", 4) == 0)
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
			cout << data_port << endl;

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
			cout << "Connected" << endl;

			send(newsock, "200 PORT Successful\n", sizeof("200 PORT Successful\n"), 0);

		}
		else if (strncmp(msg, "LIST", 4) == 0)
		{
			send(newsock, "150 Sending info\n", sizeof("150 Sending info\n"), 0);
			send(client_sock, "Testing1\r\n", sizeof("Testing1\r\n"), 0);
			send(client_sock, "Testing2\r\n", sizeof("Testing2\r\n"), 0);
			send(client_sock, "Testing3\n\n", sizeof("Testing3\n\n"), 0);
			send(newsock, "226 LIST done\n", sizeof("226 LIST done\n"), 0);
		}
		else if (strncmp(msg, "DELE", 4) == 0)
		{
			string arg = "";
			for (int i = 5; i < strlen(msg) - 2; i++) {
				arg += msg[i];
			}
			if (remove(arg.c_str()) == 0) send(newsock, "250 File deleted\n", sizeof("250 File deleted\n"), 0);
			else send(newsock, "553 could not delete file\n", sizeof("553 could not delete file\n"), 0);
		}
		else if (strncmp(msg, "QUIT", 4) == 0)
		{
			break;
		}
		else
		{
			cout << "Unsupported" << endl;
			send(newsock, "202", strlen("202"), 0); // Don't support
		}
	} while (strncmp(msg,"QUIT", 4) != 0);
	close(sock);
	close(newsock);
}
