// Name: Lav Jharwal
// Roll: 20CS30031
// include the necessary header files
// Assignment 4
// GET http://127.0.0.1/a.txt:20000
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <poll.h>
#include "time.h"
#include <fcntl.h>

#define DEFAULT_PORT 80
#define TIMEOUT 3000

// Client program

// FUNCTION TO OPEN A FILE AND RETURN THE FILE POINTER
FILE *open_file(char *file_name)
{
	FILE *fp = fopen(file_name, "rb");
	if (fp == NULL)
	{
		printf("Error opening file");
		exit(1);
	}
	return fp;
}

// FUNCTION TO CREATE A FILE AND WRITE THE CONTENT IN IT
void create_file(char *file_name, char *content)
{
	FILE *fp = fopen(file_name, "wb");
	if (fp == NULL)
	{
		printf("Error opening file");
		exit(1);
	}
	fprintf(fp, "%s", content);
	fclose(fp);
}

// FUNCTION TO GET FILE SIZE IN BYTES GIVEN THE FILE NAME
int get_file_size(char *file_name)
{
	FILE *fp = open_file(file_name);
	fseek(fp, 0L, SEEK_END); // seek to the end of the file
	int size = ftell(fp);	 // get the current file pointer
	fclose(fp);
	return size;
}

// FUNCTION TO RECEIVE THE DATA FROM THE SERVER
void Recvstr(int sockfd, char *buf)
{
	int i = 0;
	char temp[11];									 // to store the next 10 characters of the expression
	char *buffer = (char *)calloc(10, sizeof(char)); // allocating memory for buf
	int RecvSize = recv(sockfd, buffer, 10, 0);
	while (buffer[RecvSize - 1]) // if the end of expression is not reached then receive the next 10 characters
	{
		for (int i = 0; i < 11; i++)
			temp[i] = '\0';
		RecvSize += recv(sockfd, temp, 10, 0); // if the end of expression is not reached then receive the next 10 characters
		i += 10;
		// reallocate the memory for buf by size of 10
		buffer = (char *)realloc(buffer, (i + 11) * sizeof(char));
		strcat(buffer, temp); // concatenate the received characters to buf
	}
	strcpy(buf, buffer);
}







// FUNCTION TO GET CURRENT DATE IN GMT FORMAT
char *getGMT()
{
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = gmtime(&rawtime);
	char *GMT = (char *)calloc(100, sizeof(char));
	strftime(GMT, 100, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);
	return GMT;
}

// FUNCTION TO GET DATE 2 DAYS BEFORE IN GMT FORMAT
char *getGMT2DaysBefore()
{
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	putenv("TZ=GMT");
	timeinfo = gmtime(&rawtime);
	timeinfo->tm_mday -= 2;
	mktime(timeinfo);
	char *GMT = (char *)calloc(100, sizeof(char));
	strftime(GMT, 100, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);
	return GMT;
}

// FUNCTION TO EXTRACT THE HOST IP FROM THE URL
char *get_host_ip(char *url)
{
	char *p1, *p2;

	p1 = strstr(url, "//");
	if (p1)
	{
		p1 += 2;
		p2 = strchr(p1, '/');
		if (!p2)
		{
			p2 = strchr(p1, ':');
		}
		if (p2)
		{
			*p2 = '\0';
		}
		return p1;
	}
	return NULL;
}

// FUNCTION TO EXTRACT THE PORT FROM THE URL , IF NOT SPECIFIED THEN DEFAULT PORT 80 IS ASSUMED
int Extract_Port(const char *url)
{
	int port;
	int result = sscanf(url, "http://%*[^:]:%d", &port);
	if (result == 1)
	{
		// Port is specified, extract it
		return port;
	}
	else
	{
		// Port is not specified, assume default value 80
		return DEFAULT_PORT;
	}
}

// FUNCTION TO CREATE THE HTTP REoui[QUEST GIVEN THE URL, COMMAND AND FILE NAME
char *create_http_request(const char *url, char *cmd, char *file_name)
{
	// Extract host and path from the URL
	char host[100];
	char path[100];
	sscanf(url, "http://%[^/]/%s", host, path);
	int port = Extract_Port(url);
	// Construct the HTTP request
	char *request = (char *)calloc(1000, sizeof(char));

	// removing the port from the path
	char *ptr = strstr(path, ":");
	if (ptr != NULL)
	{
		*ptr = '\0';
	}

	if (strcmp(cmd, "GET") == 0)
	{
		strcat(request, "GET /");
		strcat(request, path);
	}
	if (strcmp(cmd, "PUT") == 0)
	{
		strcat(request, "PUT /");
		strcat(request, file_name);
	}

	strcat(request, " HTTP/1.1\r\n");
	strcat(request, "Host: ");
	strcat(request, host);
	// convert port to string
	char port_str[10];
	sprintf(port_str, "%d", port);
	strcat(request, ":");
	strcat(request, port_str);
	strcat(request, "\r\n");
	strcat(request, "Connection: close\r\n");

	char *GMT = getGMT();
	char *GMT2DaysBefore = getGMT2DaysBefore();

	strcat(request, "Date: ");
	strcat(request, GMT);
	strcat(request, "\r\n");

	if (strcmp(cmd, "GET") == 0)
	{

		if (strstr(path, ".html") != NULL)
			strcat(request, "Accept: text/html\r\n");
		else if (strstr(path, ".pdf") != NULL)
			strcat(request, "Accept: application/pdf\r\n");
		else if (strstr(path, ".jpg") != NULL)
			strcat(request, "Accept: image/jpeg\r\n");
		else
			strcat(request, "Accept: text/*\r\n");

		strcat(request, "Accept-Language: en-Us\r\n");
		strcat(request, "If-Modified-Since: ");
		strcat(request, GMT2DaysBefore);
		strcat(request, "\r\n");
	}
	if (strcmp(cmd, "PUT") == 0)
	{
		// opening the file named 'file_name'

		int size = get_file_size(file_name);

		strcat(request, "Content-Language: en-Us\r\n");

		strcat(request, "Content-Length: ");
		char size_str[10];
		sprintf(size_str, "%d", size);
		strcat(request, size_str);
		strcat(request, "\r\n");
		if (strstr(file_name, ".html") != NULL)
			strcat(request, "Content-Type: text/html\r\n");
		else if (strstr(file_name, ".pdf") != NULL)
			strcat(request, "Content-Type: application/pdf\r\n");
		else if (strstr(file_name, ".jpg") != NULL)
			strcat(request, "Content-Type: image/jpeg\r\n");
		else
			strcat(request, "Content-Type: text/*\r\n");

		strcat(request, "\r\n");

	}

	return request;
}

int main(int argc, char *argv[])
{
	// client will take only one argument from the command line consisting of the port number of the load balancer
	// if (argc != 2)
	// {
	// 	printf("Usage: ./client <port_number>\n");
	// 	exit(0);
	// }
	// int PORT = atoi(argv[1]);
	// int PORT = 20001;
	char *ptr = "MyOwnBrowser> ";
	char command[100]; // to store the command entered by the user

	while (1)
	{
		for (int i = 0; i < 100; i++)
			command[i] = '\0';
		printf("%s", ptr);
		// fgets(command, 100, stdin);
		scanf("%[^\n]s", command);
		scanf("%*c");
		// command[strlen(command) - 1] = '\0';

		// URL IS STORED IN 'concatenated'
		// COMMAND IS STORED IN 'path'
		char *concatenated = (char *)calloc(1, sizeof(char));
		char *cmd = strtok(command, " ");
		// storing cmd;
		char *path = (char *)calloc(strlen(cmd) + 1, sizeof(char));
		strcpy(path, cmd);

		while ((cmd = strtok(NULL, " ")) != NULL) // concatenating the path string after the command
		{
			// realloc(concatenated, strlen(concatenated) + strlen(path) + 1);
			concatenated = (char *)realloc(concatenated, strlen(concatenated) + strlen(cmd) + 2);
			strcat(concatenated, cmd);
			strcat(concatenated, " ");
		}
		// remove the last space
		concatenated[strlen(concatenated) - 1] = '\0';

		// token CONTAINS THE FILE NAME FOR PUT COMMAND
		char *token;
		token = strtok(concatenated, " ");
		token = strtok(NULL, " ");

		// STORING concatenated IN another variable
		char *concatenated2 = (char *)calloc(strlen(concatenated), sizeof(char));
		strcpy(concatenated2, concatenated);

		// GETTING THE HOST NAME
		char *host_ip = get_host_ip(concatenated2);

		// GETTING PORT NUMBER
		int port = Extract_Port(concatenated);

		if (strcmp(path, "QUIT") == 0)
			break;

		else if (strcmp(path, "GET") == 0)
		{
			// Opens a TCP connection with the http server.
			int sockfd;		// socket descriptor
			char buf[1000]; // buffer to read and write
			struct sockaddr_in server_addr;
			sockfd = socket(AF_INET, SOCK_STREAM, 0); // create a socket
			if (sockfd < 0)
			{
				perror("Socket not created\n");
				exit(0);
			}
			bzero(&server_addr, sizeof(server_addr)); // initialize server_addr to 0
			server_addr.sin_family = AF_INET;		  // IPv4
			server_addr.sin_port = htons(port);		  // port number
			inet_aton(host_ip, &server_addr.sin_addr);
			if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{ // connect to the load balancer
				perror("Connection failed\n");
				exit(0);
			}

			char *request = create_http_request(concatenated, path, token);
			printf("\n%s\n", request);

			// send the HTTP request to the server
			send(sockfd, request, strlen(request) + 1, 0);

			struct pollfd fds[1];
			int ret;
			fds[0].fd = sockfd;
			fds[0].events = POLLIN;
			ret = poll(fds, 1, TIMEOUT);
			if (ret == 0)
			{
				printf("Timeout\n");
				continue;
			}
			else if (ret < 0)
			{
				perror("Error polling socket");
				exit(1);
			}
			else
			{
				// RECIEVE THE HTTP RESPONSE FROM THE SERVER using Recvstr()
				memset(buf, 0, sizeof(buf));
				Recvstr(sockfd, buf);
				printf("%s\n", buf);
				send(sockfd, "Response Received", 17, 0);
				char *file_name = (char *)calloc(100, sizeof(char));
				sscanf(request, "GET %s", file_name);
				file_name = file_name + 1;

				// // extracting the content from the HTTP response buf
				// char *content = (char *)calloc(1000, sizeof(char));
				// int i = 0;
				// while (buf[i] != '\r' || buf[i + 1] != '\n' || buf[i + 2] != '\r' || buf[i + 3] != '\n')
				// 	i++;
				// i += 4;

				// int j = 0;
				// while (buf[i] != '\0')
				// {
				// 	content[j] = buf[i];
				// 	i++;
				// 	j++;
				// }

				// printf("%s", content);
				// create_file(file_name, content);

				// if file has extension .pdf or .jpg then open it through wb mode
				// else open it through w mode


				char *file_extension = (char *)calloc(10, sizeof(char));
				sscanf(file_name, "%*[^.].%s", file_extension);

				if(strcmp(file_extension, "pdf") == 0 || strcmp(file_extension, "jpg") == 0)
				{
					FILE *fp = fopen(file_name, "wb");
					int n;
					int data[10];
					memset(data, 0, sizeof(data));
					while ((n = recv(sockfd, data, 10, 0)) > 0)
					{
						fwrite(data, 1, n, fp);
						memset(data, 0, sizeof(data));
					}
					fclose(fp);
				}
				else
				{
					FILE *fp = fopen(file_name, "w");
					int n;
					char data[10];
					memset(data, '\0', sizeof(data));
					while ((n = recv(sockfd, data, 10, 0)) > 0)
					{
						fwrite(data, 1, n, fp);
						memset(data, '\0', sizeof(data));
					}
					fclose(fp);
				}

				// FILE *fp = fopen(file_name, "wb");
				// int a;

				// while (recv(sockfd, &a, sizeof(int), 0)>0)
				// {
				// 	fputc(a, fp);

				// 	printf("%d ", a);
				// }

				// Receive the file from the server which is sent in chunks and is in binary format

			}

			close(sockfd);
		}
		else if (strcmp(path, "PUT") == 0)
		{
			// Opens a TCP connection with the http server.
			// {
			// 	printf("Usage: ./server <port>\n");
			// 	exit(1);
			// }

			// int PORT = atoi(argv[1]);
			int sockfd;		// socket descriptor
			char buf[1000]; // buffer to read and write
			struct sockaddr_in server_addr;
			sockfd = socket(AF_INET, SOCK_STREAM, 0); // create a socket
			if (sockfd < 0)
			{
				perror("Socket not created\n");
				exit(0);
			}
			bzero(&server_addr, sizeof(server_addr)); // initialize server_addr to 0
			server_addr.sin_family = AF_INET;		  // IPv4
			server_addr.sin_port = htons(port);		  // port number
			inet_aton(host_ip, &server_addr.sin_addr);
			if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{ // connect to the load balancer
				perror("Connection failed\n");
				exit(0);
			}
			char *request = create_http_request(concatenated, path, token);

			printf("\n%s\n", request);
			// send the command to the HTTP server
			send(sockfd, request, strlen(request) + 1, 0);
			
			char d[10];
			recv(sockfd, d, 10, 0);
			printf("%s\n", d);

			printf("Enter the file name: %s\n", token);

			char *file_extension = (char *)calloc(10, sizeof(char));
			sscanf(token, "%*[^.].%s", file_extension);

			if(file_extension == "pdf" || file_extension == "jpg"){
				FILE *fp = fopen(token, "rb");
				int n;
				int data[10];
				memset(data, 0, sizeof(data));
				while((n = fread(data,sizeof(int), 10, fp)) > 0)
				{
					// for(int i=0;i<n;i++)
					// 	printf("%d ", data[i]);
					send(sockfd, data, n*sizeof(int), 0);
					// fwrite(data, sizeof(int), n, fp2);
					// printf("\nSent %d bytes\n", n);
					memset(data, 0, sizeof(data));
				}
				// send(sockfd,0,1,0);
				fclose(fp);
			}

			else{
				FILE *fp = fopen(token, "r");
				int n;
				char data[10];
				memset(data, '\0', sizeof(data));
				while((n = fread(data,sizeof(char), 10, fp)) > 0)
				{
					// for(int i=0;i<n;i++)
					// 	printf("%d ", data[i]);
					send(sockfd, data, n*sizeof(char), 0);
					// fwrite(data, sizeof(int), n, fp2);
					// printf("\nSent %d bytes\n", n);
					memset(data, '\0', sizeof(data));
				}
				// send(sockfd,0,1,0);
				fclose(fp);
			}

			printf("File sent\n");

			// RECIEVE THE RESPONSE FROM THE SERVER
			// memset(buf, '\0', sizeof(buf));
			// Recvstr(sockfd, buf);
			// // recv(sockfd, buf, 1000, 0);
			// printf("Response from server: ");
			// printf("%s\n", buf);
			close(sockfd);
		}
		else
		{
			printf("Invalid command ");
		}
	}
	return 0;
}