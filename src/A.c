#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdarg.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define MSG_SIZE 4096

#define SA struct sockaddr

int readXBytes(int sockfd, long unsigned x, char *buffer)
{
    int bytesRead = 0;
    int res;
    while (bytesRead < (int) x)
    {
        res = read(sockfd, buffer + bytesRead, x - bytesRead);
        if (res < 0)
        {
            printf("Error reading checkpoint\n");
            perror("read");
            return -1;
        }
        bytesRead += res;
    }
    return 0;
}

int main()
{	
	int sockfd, connfd = 0;
	struct sockaddr_in servaddr, cli;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1){
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("socket created\n");

    int flag = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(8080); //define port
	if (bind(sockfd, (SA*)&servaddr, sizeof(servaddr))!=0){
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("socket binded...\n");
	
	if ((listen(sockfd, 5))!=0){
		printf("listen failed...\n");
		exit(0);
	}
	else
		printf("server listening...\n");
	
    socklen_t lenAddr = sizeof(cli);

	connfd = accept(sockfd, (struct sockaddr *)&cli, &lenAddr);
	if(connfd < 0){
		printf("conn failed...\n");
		exit(0);
	}
	else
    {
		printf("Connexion OK.\n");
	}

    char *buff = (char *)malloc(MSG_SIZE);
	
	while(1)
	{
		if (readXBytes(connfd, MSG_SIZE, buff) < 0)
		{
			printf("readXBytes failed\n");
			exit(0);
		}
	}
	return 0;
}
