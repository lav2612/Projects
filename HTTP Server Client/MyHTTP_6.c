#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "time.h"
#include <sys/stat.h>
#define DEFAULT_PORT 80
// int PORT = 20001;
FILE *open_file(char *file_name)
{
	FILE *fp = fopen(file_name, "rb");
	if (fp == NULL)
	{
		printf("Error opening file");
		// exit(1);
		return NULL;
	}
	return fp;
}

// Function to get the last modified date of a file
char *get_last_modified_date(char *filename)
{
	// Get information about the file
	struct stat sb;
	if (stat(filename, &sb) == -1)
	{
		printf("Error: Unable to get information about file %s\n", filename);
		return NULL;
	}
	// Get the last modified date of the file in GMT format
	struct tm *tm = gmtime(&sb.st_mtime);
	char *date = (char *)calloc(35, sizeof(char));
	strftime(date, 35, "%a, %d %b %Y %H:%M:%S GMT", tm);
	return date;
}

// function to get date in GMT format
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

char *getGMTExpiry()
{
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	putenv("TZ=GMT");
	timeinfo = gmtime(&rawtime);
	timeinfo->tm_mday += 3;
	mktime(timeinfo);
	char *GMT = (char *)calloc(100, sizeof(char));
	strftime(GMT, 100, "%a, %d %b %Y %H:%M:%S %Z", timeinfo);
	return GMT;
}
void Updatedate(int *dd, int *mm, int *yy, char *GMT)
{
	char day[3], month[4], year[5];
	for (int i = 4; i < 7; i++)
	{
		day[i - 4] = GMT[i];
	}
	day[3] = '\0';
	for (int i = 8; i < 11; i++)
	{
		month[i - 8] = GMT[i];
	}
	month[3] = '\0';
	for (int i = 12; i < 16; i++)
	{
		year[i - 12] = GMT[i];
	}
	year[4] = '\0';
	// int dd, mm, yyyy;
	*dd = atoi(day);
	if (strcmp(month, "Jan") == 0)
		*mm = 1;
	else if (strcmp(month, "Feb") == 0)
		*mm = 2;
	else if (strcmp(month, "Mar") == 0)
		*mm = 3;
	else if (strcmp(month, "Apr") == 0)
		*mm = 4;
	else if (strcmp(month, "May") == 0)
		*mm = 5;
	else if (strcmp(month, "Jun") == 0)
		*mm = 6;
	else if (strcmp(month, "Jul") == 0)
		*mm = 7;
	else if (strcmp(month, "Aug") == 0)
		*mm = 8;
	else if (strcmp(month, "Sep") == 0)
		*mm = 9;
	else if (strcmp(month, "Oct") == 0)
		*mm = 10;
	else if (strcmp(month, "Nov") == 0)
		*mm = 11;
	else if (strcmp(month, "Dec") == 0)
		*mm = 12;
	*yy = atoi(year);
}

// function to find the number of days between two dates in dd mm yyyy format
int calcdays(int dd1, int mm1, int yy1, int dd2, int mm2, int yy2)
{
	int days1 = 0, days2 = 0, days = 0;

	// Calculate the total number of days for the first date
	days1 = dd1 + 30 * (mm1 - 1) + 365 * (yy1 - 1);
	if (mm1 > 2)
	{
		days1 += (yy1 % 4 == 0) ? 1 : 0;
	}

	// Calculate the total number of days for the second date
	days2 = dd2 + 30 * (mm2 - 1) + 365 * (yy2 - 1);
	if (mm2 > 2)
	{
		days2 += (yy2 % 4 == 0) ? 1 : 0;
	}

	// Calculate the difference between the two dates
	days = days2 - days1;

	return days;
}

int datediff(char *date1, char *date2)
{
	int dd, mm, yyyy;
	int ddExpiry, mmExpiry, yyyyExpiry;
	Updatedate(&dd, &mm, &yyyy, date1);
	Updatedate(&ddExpiry, &mmExpiry, &yyyyExpiry, date2);
	// printf("dd: %d, mm: %d, yyyy: %d\n", dd, mm, yyyy);
	// printf("ddExpiry: %d, mmExpiry: %d, yyyyExpiry: %d\n", ddExpiry, mmExpiry, yyyyExpiry);
	int days = calcdays(dd, mm, yyyy, ddExpiry, mmExpiry, yyyyExpiry);
	if (days < 0)
		days = -(days);
	return days;
}

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
	free(buffer);
}

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

/*
PUT /docs HTTP/1.1
Host: 10.98.78.2:80
Connection: close
Date: Sat, 11 Feb 2023 07:04:04 GMT
Content-Language: en-Us
Content-Length: 100
Content-Type: text/html

<pre>
 <code>
 x = 5;
 y = 6;
 z = x + y;
 </code>
 </pre>
*/

// function to parse the above request and extracting the content
char *PUT(char *buf, char *file_name)
{

	// int i = 0;
	// while (buf[i] != '\r' || buf[i + 1] != '\n' || buf[i + 2] != '\r' || buf[i + 3] != '\n')
	// 	i++;
	// i += 4;

	// char *content = (char *)calloc(1000,sizeof(char));
	// int j = 0;
	// while (buf[i] != '\0')
	// {
	// 	content[j] = buf[i];
	// 	i++;
	// 	j++;
	// }

	// finding the Content-Type: from the request
	char *type = (char *)calloc(20, sizeof(char));
	int i = 0;
	while (1)
	{
		if (buf[i] == 'C' && buf[i + 1] == 'o' && buf[i + 2] == 'n' && buf[i + 3] == 't' && buf[i + 4] == 'e' && buf[i + 5] == 'n' && buf[i + 6] == 't' && buf[i + 7] == '-' && buf[i + 8] == 'T' && buf[i + 9] == 'y' && buf[i + 10] == 'p' && buf[i + 11] == 'e' && buf[i + 12] == ':')
		{
			i += 14;
			int j = 0;
			while (buf[i] != '\n')
			{
				type[j] = buf[i];
				i++;
				j++;
			}
			type[j] = '\0';
			break;
		}
		i++;
	}
	// storing file name in another variable
	char *file_name1 = (char *)calloc(50, sizeof(char));
	strcpy(file_name1, file_name);

	// reallocating the file name

	if (strcmp(type, "text/html\r") == 0)
	{
		strcat(file_name1, ".html");
	}
	else if (strcmp(type, "application/pdf\r") == 0)
	{
		strcat(file_name1, ".pdf");
	}
	else if (strcmp(type, "image/jpeg\r") == 0)
	{
		strcat(file_name1, ".jpeg");
	}
	else if (strcmp(type, "text/*\r") == 0)
	{
		strcat(file_name1, ".txt");
	}
	// free(type);
	// creating the file and writing the content in it

	int content_length = strlen(buf);

	// create_file(file_name1, content);
	free(file_name1);
	// int content_length = strlen(content);
	char *response = (char *)calloc(1000, sizeof(char));
	strcat(response, "HTTP/1.1 200 OK\r\n");
	strcat(response, "Content-Type: ");
	strcat(response, type);
	strcat(response, "\r\n");
	strcat(response, "Content-Language: en-us\r\n");
	strcat(response, "Content-Length: ");
	char *content_length1 = (char *)malloc(10 * sizeof(char));
	sprintf(content_length1, "%d", content_length);
	strcat(response, content_length1);
	strcat(response, "\r\n");
	strcat(response, "Connection: close\r\n");
	// close(newsockfd);
	return response;
}

char *create_response(long int s, char *file_name, char *if_modified_since)
{
	// get the GMT date
	char *GMT = getGMT();
	// send the response message
	char *response = (char *)calloc(1000, sizeof(char));
	strcat(response, "HTTP/1.1 200 OK\r\n");
	strcat(response, "Date: ");
	strcat(response, GMT);
	strcat(response, "\r\n");
	strcat(response, "Expires: ");
	strcat(response, getGMTExpiry());
	strcat(response, "\r\n");
	strcat(response, "Content-Language: en-us\r\n");
	strcat(response, "Content-Length: ");
	// get the content length and convert it to string
	char *content_length = (char *)calloc(10, sizeof(char));
	sprintf(content_length, "%ld", s);
	strcat(response, content_length);
	strcat(response, "\r\n");
	free(content_length);
	strcat(response, "Content-Type: ");
	// check the file extension
	char *file_ext = (char *)calloc(10, sizeof(char));
	sscanf(file_name, "%*[^.].%s", file_ext);
	if (strcmp(file_ext, "html") == 0)
	{
		strcat(response, "text/html");
	}
	else if (strcmp(file_ext, "jpg") == 0)
	{
		strcat(response, "image/jpeg");
	}
	else if (strcmp(file_ext, "pdf") == 0)
	{
		strcat(response, "application/pdf");
	}
	else
	{
		strcat(response, "text/*");
	}
	free(file_ext);
	strcat(response, "\r\n");
	strcat(response, "Last-Modified: ");
	strcat(response, get_last_modified_date(file_name));
	strcat(response, "\r\n");
	strcat(response, "\r\n");
	// strcat(response, content);

	if (datediff(if_modified_since, get_last_modified_date(file_name)) >= 0)
	{
		return response;
	}
	else
	{
		return "HTTP/1.1 400 Bad Request\r\n";
	}
}

int main(int argc, char const *argv[])
{
	int PORT;
	if (argc != 2)
	{
		printf("Enter the port number as command line argument\n");
		exit(1);
	}
	else
	{
		PORT = atoi(argv[1]);
	}

	// int PORT = 20001;
	int sockfd, newsockfd;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	memset((char *)&serv_addr, '\0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("Error opening socket");
		exit(1);
	}

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Error on binding");
		exit(1);
	}

	listen(sockfd, 5);

	while (1)
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
		{
			perror("Error on accept");
			exit(1);
		}

		if (fork() == 0)
		{
			close(sockfd);
			char buffer[1024];
			memset(buffer, '\0', 1024);

			// Recvstr(newsockfd,buffer);
			// recv(newsockfd, buffer, 1024, 0);
			Recvstr(newsockfd, buffer);
			// printf("Request: %s\n", buffer);
			// if first 3 characters are GET
			if (strncmp(buffer, "GET", 3) == 0)
			{

				/*
				GET /a.html HTTP/1.1
				Host: 10.147.129.252:20000
				Connection: close
				Date: Mon, 13 Feb 2023 10:05:23 GMT
				Accept: text/html
				Accept-Language: en-Us
				If-Modified-Since: Sat, 11 Feb 2023 10:05:23 GMT
				*/

				// get the If-Modified-Since: date from the request message as given in the above example
				char *if_modified_since = NULL;
				char *token1 = strtok(buffer, "\r\n");
				while (token1 != NULL)
				{
					if (strstr(token1, "If-Modified-Since: ") != NULL)
					{
						if_modified_since = strstr(token1, ": ") + 2;
						break;
					}
					token1 = strtok(NULL, "\r\n");
				}

				// get the file name from the request message until the first space
				char *file_name = (char *)calloc(100, sizeof(char));
				sscanf(buffer, "GET %s", file_name);
				// remove the first character which is '/'
				file_name = file_name + 1;
				// printf("\n%s\n",file_name);
				// check if the file exists
				FILE *fp = open_file(file_name);
				// FILE *fp = fopen(file_name, "r");
				// if the file exists
				if (fp != NULL)
				{
					// printf("%s file is being requested and exists\n", file_name);
					// printf("File opened successfully\n");
					char ch;
					long int i = 0;
					// char *content = (char *)calloc(1000, sizeof(char));
					while ((ch = fgetc(fp)) != EOF)
					{
						i = i + 2;
						// content[i]=ch;
						// i++;
						// content = (char *)realloc(content, (i+1)*sizeof(char));
					}
					// content[i] = '\0';
					// printf("%s\n",content);
					char *response = create_response(i, file_name, if_modified_since);
					// printf("%s\n", response);
					// send the response message to the client by adding null character at the end
					// response = (char *)realloc(response,(strlen(response)+1)*sizeof(char));
					// response[strlen(response)] = '\0';
					send(newsockfd, response, strlen(response) + 1, 0);
					char buf[20];
					recv(newsockfd, buf, 17, 0);
					// printf("Response: %s\n", buf);
					memset(buf, '\0', 1024);

					char *extension = (char *)calloc(10, sizeof(char));
					sscanf(file_name, "%*[^.].%s", extension);

					// if the file is a jpg or pdf
					if (strcmp(extension, "jpg") == 0 || strcmp(extension, "pdf") == 0)
					{
						int *data = (int *)calloc(10, sizeof(int));

						FILE *fp2 = open_file(file_name);
						// int a;

						// while((a = fgetc(fp2)) != EOF){
						// 	send(newsockfd, &a, sizeof(a), 0);
						// }
						// send(newsockfd,"\0",1 , 0);
						// send the file contents to the client in chunks of 10 bytes until the end of the file in binary format
						int n;
						while ((n = fread(data, sizeof(int), 10, fp2)) > 0)
						{
							send(newsockfd, data, n * sizeof(int), 0);
							// memset(data, 0, 10);
						}

						// send(newsockfd, 0, 1, 0);
						fclose(fp2);
					}
					else
					{
						char *data = (char *)calloc(10, sizeof(char));

						// open file using "r" mode
						FILE *fp2 = fopen(file_name, "r");
						int n;
						while ((n = fread(data, sizeof(char), 10, fp2)) > 0)
						{
							send(newsockfd, data, n * sizeof(char), 0);
							// memset(data, 0, 10);
						}

						// send(newsockfd, 0, 1, 0);
						fclose(fp2);
					}
					printf("\n%s", response);

					printf("\n-------------------RESPONSE GENERATED----------------\n");
					/*
					maintains an Access Log (AccessLog.txt) which records every client accesses. Format of
					every record/line of the AccessLog.txt: <Date(ddmmyy)>:<Time(hhmmss)>:<Client IP>:<Client
					Port>:<GET/PUT>:<URL>
					*/
					// get the GMT date
					char *GMT = getGMT();
					// get the client IP address
					char *client_IP = (char *)calloc(100, sizeof(char));
					client_IP = inet_ntoa(cli_addr.sin_addr);
					// get the client port number
					int client_port = ntohs(cli_addr.sin_port);
					// get the URL
					char *URL = (char *)calloc(100, sizeof(char));
					sscanf(buffer, "GET %s", URL);
					// Since we are in the if condition, it is a GET request
					char *request_type = "GET";
					// get the date using time.h
					time_t t = time(NULL);
					struct tm tm = *localtime(&t);
					char *date = (char *)calloc(100, sizeof(char));
					sprintf(date, "%d/%d/%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

					// get the time using time.h
					char *time = (char *)calloc(100, sizeof(char));
					sprintf(time, "%d:%d:%d", tm.tm_hour, tm.tm_min, tm.tm_sec);

					// write the log to the AccessLog.txt
					FILE *fp1 = fopen("AccessLog.txt", "a");
					fprintf(fp1, "<%s>:<%s>:<%s>:<%d>:<%s>:</%s>\n", date, time, client_IP, client_port, request_type, file_name);
					printf("Access log written to AccessLog.txt for GET request\n");

					fclose(fp1);
					close(newsockfd);
				}
				// if the file does not exist
				else
				{
					// get the GMT date
					char *GMT = getGMT();
					// send the response message
					char *response = (char *)calloc(1000, sizeof(char));
					printf("\nRESPONSE: 404 Not Found");
					sprintf(response, "\n HTTP/1.1 404 Not Found\r\n Date: %s\r\n Content-Length: 0\r\n Content-Type: text/html\r\n\r\n", GMT);
					printf("\n%s\n", response);
					send(newsockfd, response, strlen(response) + 1, 0);
					// free the allocated memory
					free(response);
					free(GMT);
				}
			}

			if (strncmp(buffer, "PUT", 3) == 0)
			{
				// char *file_name = "test";
				// extract the file name from the buffer and store it in file_name
				char *file_name = (char *)calloc(100, sizeof(char));
				sscanf(buffer, "PUT %s", file_name);
				// remove the first character which is '/'
				file_name = file_name + 1;

				// remove extension from file name
				char *file_name_without_ext = (char *)calloc(100, sizeof(char));
				sscanf(file_name, "%[^.]", file_name_without_ext);

				char *response = PUT(buffer, file_name_without_ext);
				// printf("response is %s\n", response);

				send(newsockfd, "RECEIVED", 10, 0);

				// printf("done till here\n");
				// finding the extension of the file
				char *extension = (char *)calloc(10, sizeof(char));
				sscanf(file_name, "%*[^.].%s", extension);
				// if the extension is jpg or pdf

				if (strcmp(extension, "jpg") == 0 || strcmp(extension, "pdf") == 0)
				{
					// open the file in binary mode
					FILE *fp1 = fopen(file_name, "wb");
					// receive the file contents from the client in chunks of 10 bytes until the end of the file in binary format
					int n;
					int data[10];
					memset(data, 0, sizeof(data));
					while ((n = recv(newsockfd, data, 10, 0)) > 0)
					{
						// for(int i = 0; i < n; i++){
						// 	printf("%d ", data[i]);
						// }
						// printf("\n");
						fwrite(data, 1, n, fp1);
						// printf("\nReceived string: %d\n", n);
						memset(data, 0, sizeof(data));
					}
					fclose(fp1);
				}
				else
				{
					// open file using "w" mode
					FILE *fp1 = fopen(file_name, "w");
					// receive the file contents from the client in chunks of 10 bytes until the end of the file in binary format
					int n;
					char data[10];
					memset(data, '\0', sizeof(data));
					while ((n = recv(newsockfd, data, 10, 0)) > 0)
					{
						// for(int i = 0; i < n; i++){
						// 	printf("%d ", data[i]);
						// }
						// printf("\n");
						fwrite(data, 1, n, fp1);
						// printf("\nReceived string: %d\n", n);
						memset(data, '\0', sizeof(data));
					}
					fclose(fp1);
				}

				// printf("File received\n");

				printf("\n%s", response);
				printf("\n-------------------RESPONSE GENERATED----------------\n");

				// send(newsockfd, response, strlen(response) + 1, 0);

				// printf("Response sent\n");

				// get the GMT date
				char *GMT = getGMT();
				// get the client IP address
				char *client_IP = (char *)calloc(100, sizeof(char));
				client_IP = inet_ntoa(cli_addr.sin_addr);
				// get the client port number
				int client_port = ntohs(cli_addr.sin_port);
				// get the URL
				char *URL = (char *)calloc(100, sizeof(char));
				sscanf(buffer, "PUT %s", URL);
				// Since we are in the if condition, it is a GET request
				char *request_type = "PUT";
				// get the date using time.h
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				char *date = (char *)calloc(100, sizeof(char));
				sprintf(date, "%d/%d/%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

				// get the time using time.h
				char *time = (char *)calloc(100, sizeof(char));
				sprintf(time, "%d:%d:%d", tm.tm_hour, tm.tm_min, tm.tm_sec);

				// write the log to the AccessLog.txt
				FILE *fp = fopen("AccessLog.txt", "a");
				fprintf(fp, "<%s>:<%s>:<%s>:<%d>:<%s>:<%s>\n", date, time, client_IP, client_port, request_type, URL);
				printf("Access log written to AccessLog.txt for PUT request\n");
				fclose(fp);
				free(response);
				close(newsockfd);
			}
			close(newsockfd);
			exit(0);
		}
		close(newsockfd);
	}
}