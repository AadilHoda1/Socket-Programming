// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;

	//creating buffer and  hello message in it
	char hello[1024] = "Hello from client";
	char buffer[1024] = {0};

	//creating socket for client process... error if failed
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));

	// Convert IPv4 and IPv6 addresses from text to binary form
	//error in case of wrong format of ip address
	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

  // connecting to the server socket
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	//if connection succesfully established then send hello message to server
	send(sock , hello , strlen(hello) , 0 );
	bzero(hello, 1024);
	strcpy(hello, "Hello message sent\n");
	printf("%sHello from server\nConnection established\n", hello);
	while(1){
		// read messages sent by the server
		read( sock , buffer, 1024);
		printf("%s",buffer );
		// If there is an Authentication Error 
		if(strcmp(buffer, "Invalid username or password\n") == 0){
			bzero(buffer, 1024);
			read(sock, buffer, 1024);
		}
		//end connection if 'END' is received
		if(strcmp(buffer,"END") == 0)
			break;
		bzero(buffer,1024);
		scanf("%s", buffer);
		//send reply to server
		send(sock , buffer , strlen(buffer) , 0);
		bzero(buffer,1024);
	}
	//close client socket
	close(sock);
	return 0;
}
