#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>

using namespace std;

void ls(int client_sock)
{
	DIR *dir_ptr;
	dirent * dir_ent;
	//cout << "Starting to send directory!" << endl;
	dir_ptr = opendir(".");
	while ((dir_ent = readdir(dir_ptr)) != NULL)
	{
		send(client_sock, dir_ent->d_name, strlen(dir_ent->d_name), 0);
		send(client_sock, "\r\n", sizeof("\r\n") - 1, 0);
	}
	closedir(dir_ptr);
	//cout << "Done sending directory!" << endl;
}

string parse_msg(string msg)
{
	int i = 0;
	// Fast forward to the argument
	while (i < msg.length() && msg[i] != ' ')
	{
		i++;
	}
	i++;
	string arg = "";
	for (; i < msg.length() - 2; i++)
	{
		arg += msg[i];
	}
	return arg;
}

// Write store and retrieve functions
void store(int client_sock, string msg)
{
	string arg = parse_msg(msg);
	FILE * file_ptr;
	//file_ptr = fopen(arg.c_str(), "w");
	file_ptr = fopen(arg.c_str(), "wb");
	uint8_t byte;
	while (recv(client_sock, &byte, sizeof(byte), 0) != 0)
	{
		fwrite(&byte, sizeof(uint8_t), sizeof(byte), file_ptr);// << endl;
	}
	fclose(file_ptr); // Uber important to flush everything to disk
}

void retrieve(int client_sock, string msg)
{
	string arg = parse_msg(msg);
	FILE * file_ptr;
	file_ptr = fopen(arg.c_str(), "rb");
	uint8_t byte;
	while( fread(&byte, sizeof(uint8_t), sizeof(byte), file_ptr) != 0)
	{
		send(client_sock, &byte, sizeof(byte), 0);
	}
	//send(client_sock, "\r\n", sizeof("\r\n"), 0);
	fclose(file_ptr);
}
