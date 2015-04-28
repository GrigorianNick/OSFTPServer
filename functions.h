#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;

void ls(int client_sock)
{
	DIR *dir_ptr;
	dirent * dir_ent;
	cout << "Starting to send directory!" << endl;
	dir_ptr = opendir(".");
	while ((dir_ent = readdir(dir_ptr)) != NULL)
	{
		send(client_sock, dir_ent->d_name, strlen(dir_ent->d_name), 0);
		send(client_sock, "\r\n", sizeof("\r\n") - 1, 0);
	}
	closedir(dir_ptr);
	cout << "Done sending directory!" << endl;
}
