#include <netinet/in.h> /* sockaddr_in */
#include <netinet/tcp.h> /*TCP_REPAIR_WINDOW, TCP TIMESTAMP */
#include <stdint.h> /* uint32_t */
#include <sys/socket.h> /* sockaddr */
#include <linux/types.h> /* __u64 */
#include <linux/sockios.h> // SIOCOUTQ
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>

#include "../liburing-master/src/include/liburing.h"


#define SIZE_URING_SQE 40
#define MAX_QUEUE_SIZE 65535 << 14
#define MSG_SIZE 4096



char recv_queue[MAX_QUEUE_SIZE] = {0};

int connection_to_client(){
	
	int sockfdClient;
	struct sockaddr_in AAddr;
	int portA = 8080;
	char* ipA = "192.168.15.1";
	
	//Création de la sockeet
    if ((sockfdClient = socket(AF_INET, SOCK_STREAM, 0)) == 1)
    {
        printf("Socket creation failed...\n");
        exit(1);
    }
	
    memset(&AAddr, 0, sizeof(struct sockaddr_in));
    AAddr.sin_family = AF_INET;
    AAddr.sin_addr.s_addr = inet_addr(ipA);
    AAddr.sin_port = htons(portA);
	
	if (connect(sockfdClient, (struct sockaddr *)&AAddr, sizeof(AAddr)) != 0)
    {
        printf("connection with server failed.\n");
        exit(0);
    }
	
	return sockfdClient;
}



int main(int argc, char *argv[]){
	int sock_fd_client;
	char msg_buffer[MSG_SIZE] = {1};
	sock_fd_client = connection_to_client();
	struct io_uring ring;
	
	struct io_uring_sqe *sqe;
	struct io_uring_cqe *cqe;
				
	io_uring_queue_init(SIZE_URING_SQE, &ring, IORING_SETUP_CQE32 | IORING_SETUP_SQE128);
	memset(msg_buffer, 0, MSG_SIZE);
	
	int size_write = write(sock_fd_client, msg_buffer, MSG_SIZE);
	
	int	repaireON = 1, 
		repaireOFF = 0, 
		amount = 0;
	int id_queue = TCP_RECV_QUEUE;
	long int ret, res;
	
	if (setsockopt(sock_fd_client, SOL_TCP, TCP_REPAIR, &repaireON, sizeof(int)) < 0)
	{
		perror("setsockopt repaire on");
		exit(1);
	}
	if (setsockopt(sock_fd_client, SOL_TCP, TCP_REPAIR_QUEUE, &id_queue, sizeof(int)) < 0)
	{
		perror("setsockopt repaire queue");
		exit(1);
	}
	// Getting ingress queue content
	sqe = io_uring_get_sqe(&ring);
	if(!sqe){
		perror("Error retreving SQE");
		exit(1);
	}
	
	int ret_recv = recv(
				sock_fd_client, 
				recv_queue,
				MAX_QUEUE_SIZE, 
				MSG_PEEK | MSG_DONTWAIT);
				
	printf("Result of recv syscall : %d\n", ret_recv);
	
	///////////////////////////
	sqe->user_data = ++amount;
	io_uring_prep_recv(	sqe, 
				sock_fd_client, 
				recv_queue,
				MAX_QUEUE_SIZE, 
				MSG_PEEK | MSG_DONTWAIT);
	sqe->flags |= IOSQE_IO_DRAIN;
	//Simulate other io_uring instructions
	sqe = io_uring_get_sqe(&ring);
	if(!sqe){
		perror("Error retreving SQE");
		exit(1);
	}
	sqe->user_data = ++amount;
	io_uring_prep_nop(sqe);
	printf("Submitting SQE ... \n");
	
	io_uring_submit_and_wait(&ring, amount);
	
	printf("OK ! \n");
	
	//////////////////////////////////////
	
	do{
		ret = io_uring_wait_cqe(&ring, &cqe);
		res = cqe->res;
		printf("CQE %lld : %ld %ld\n", cqe->user_data, ret, res);
		
		io_uring_cqe_seen(&ring, cqe);
		amount--;
	}while(amount > 0);
	
	if (setsockopt(sock_fd_client, SOL_TCP, TCP_REPAIR, &repaireOFF, sizeof(int)) < 0)
	{
		perror("setsockopt repaire off");
		exit(1);
	}

	io_uring_queue_exit(&ring);
	close(sock_fd_client);
	
	return 0;
}
