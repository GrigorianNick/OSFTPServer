/*

Nicholas Grigorian

This file is responsible for handling data communication. (LIST STOR * RETR).

Functions:
 - ls(int client_sock)					Lists current directory
 - ls(int client_sock, string msg)		Lists directory specified by msg
 - parse_msg(string msg)				Extras the argument from a raw ftp
 										command
 - store(int client_sock,string msg)	Stores info coming over data connection
 										in file designated by msg
 - retrieve(int client_sock,string msg) Sends the file specified by msg over
 										the data connection.

Changelog:
 - April 29
 	Created with retrieve, ls, parse_msg, and store functions.
 - April 30
 	Fixed some minor bugs.
 	Added ls(int client_sock, string msg)
 */

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

string parse_msg(string msg) // Pulls the arguments out of a raw ftp message
{
	int i = 0;
	// Fast forward to the argument
	while (i < msg.length() && msg[i] != ' ')
	{
		i++;
	}
	i++; // Advance past the space
	string arg = "";
	for (; i < msg.length() - 2; i++)
	{
		arg += msg[i];
	}
	return arg;
}

void ls(int client_sock) // Open local directory, list all the entries
{
	DIR *dir_ptr;
	dirent * dir_ent;
	dir_ptr = opendir(".");
	while ((dir_ent = readdir(dir_ptr)) != NULL)
	{
		send(client_sock, dir_ent->d_name, strlen(dir_ent->d_name), 0);
		send(client_sock, "\r\n", sizeof("\r\n") - 1, 0);
	}
	closedir(dir_ptr);
}

void ls(int client_sock, string msg) // Opens DIR specified by msg
{
	string arg = parse_msg(msg);
	cout << "ls arg: " << arg << endl;
	DIR *dir_ptr;
	dirent * dir_ent;
	dir_ptr = opendir(arg.c_str());
	while ((dir_ent = readdir(dir_ptr)) != NULL)
	{
		send(client_sock, dir_ent->d_name, strlen(dir_ent->d_name), 0);
		send(client_sock, "\r\n", sizeof("\r\n") - 1, 0);
	}
	closedir(dir_ptr);
}

void store(int client_sock, string msg) // Stores a file on the server by looping "recv a byte, write a byte"
{
	string arg = parse_msg(msg);
	FILE * file_ptr;
	file_ptr = fopen(arg.c_str(), "wb");
	uint8_t byte;
	while (recv(client_sock, &byte, sizeof(byte), 0) != 0)
	{
		fwrite(&byte, sizeof(uint8_t), sizeof(byte), file_ptr);// << endl;
	}
	fclose(file_ptr); // Uber important to flush everything to disk
}

void retrieve(int client_sock, string msg) // Opens up server file then loops "read a byte send a byte" until the file's done
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
