#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

int LISTEN_FLAG = 0;
int gserver_sockfd, gclient_sockfd;
int gc_sockfd;

struct Node{
	int sock;
	struct sockaddr_in address;
	socklen_t len;
};

void closeSockets(void)
{
	close(gc_sockfd);
	close(gclient_sockfd);
	close(gserver_sockfd);
}

void* threadedWrite(void *arg) 
{
    int client_sockfd = (int) arg;
    char msg[60] = " ";
   
    while(1) {
        fgets(msg, 60, stdin);
        if(write(client_sockfd, &msg, strlen(msg)) != -1) 			{ 
        	sleep(2);
            memset(msg, '\0', sizeof(msg));
        }
    }
    pthread_exit(NULL);
}

void* threadedRead(void *arg)
{
    char m[60];
    memset(m, '\0', sizeof(m));
   
    int client_sockfd = (int) arg;
    int result;
    
    while(1) {
    	result = read(client_sockfd, &m, sizeof(m));
      	
		if(result == 0) {
			close(gc_sockfd);
			close(gserver_sockfd);
			close(gclient_sockfd);
			exit(-1);
		}
		else {
    		printf(">%s\n", m);
    		fflush(stdout);
    	}
    	sleep(1);
    	memset(m, '\0', sizeof(m));
    
	}
    pthread_exit(NULL);
}

void* threadedConnect(void *arg)
{
	struct Node* n = (struct Node*) arg;
    int client_sockfd = n->sock;
    struct sockaddr_in client_address = n->address;
    socklen_t client_len = n->len;    
    int rc = -1;
    
    while(rc != 0) {
    
	   	rc = connect(client_sockfd, (struct sockaddr *) &client_address, client_len);
		sleep(2);
		
		if(rc == -1);
			perror("Error connect(): ");
		
		
	}
	pthread_exit(NULL);
}

void* threadedAccept(void* arg)
{
	pthread_t write_t, read_t;
	
	struct Node* n = (struct Node*) arg;
    int server_sockfd = n->sock;
    struct sockaddr_in client_address = n->address;
    socklen_t client_len = n->len;
	while(1) {
	
		if(LISTEN_FLAG){
			gclient_sockfd = accept(server_sockfd, (struct sockaddr*) &client_address, &client_len);
			
		if(gclient_sockfd == -1)
			perror("Error accept(): ");
		
		pthread_create(&read_t, NULL, threadedRead, (void*) gclient_sockfd);
		pthread_create(&write_t, NULL, threadedWrite, (void*) gclient_sockfd);	
		}
	}
	pthread_exit(NULL);
}

void* threadedListen(void *arg)
{
    int server_sockfd = (int) arg;
    
    while(1) {
        if(listen(server_sockfd, 5) == -1)
        	perror("listen ");
        else
        	LISTEN_FLAG = 1;   	
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
	
	if(strncmp(argv[2], "MAS", 3)){
		
		pthread_t listen_t, accept_t;
		
		socklen_t server_len, client_len;
		struct sockaddr_in server_address;
		struct sockaddr_in client_address;
		
		gserver_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = htonl(INADDR_ANY);
		server_address.sin_port = htons(9001);
		server_len = sizeof(server_address);
		
		int on = 1;
		setsockopt (gserver_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
		
		bind(gserver_sockfd, (struct sockaddr *)&server_address, server_len);
		
		client_address.sin_family = AF_INET;
		client_address.sin_addr.s_addr = inet_addr(argv[1]);
		client_address.sin_port = htons(9001);
		client_len = sizeof(client_address);
		
		struct Node *server_node;
		server_node = malloc(sizeof(struct Node));
		server_node->sock = gserver_sockfd;
		server_node->address = client_address;
		server_node->len = client_len;
  		
  		pthread_create(&listen_t, NULL, threadedListen, (void *) gserver_sockfd);
        pthread_create(&accept_t, NULL, threadedAccept, (void*)server_node);					
							
	}
	else if(strncmp(argv[2], "SAL", 3)){
	
		pthread_t connect_t, read_t, write_t;
		
		socklen_t len;
		struct sockaddr_in address;
		int result;
		
		gc_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(argv[1]);
		address.sin_port = htons(9001);
		len = sizeof(address);
		
		struct Node *client_node;
		client_node = malloc(sizeof(struct Node));
		client_node->sock = gc_sockfd;
		client_node->address = address;
		client_node->len = len;  
	
		
		pthread_create(&connect_t, NULL, threadedConnect, (void*)client_node);
		pthread_create(&read_t, NULL, threadedRead, (void*)gc_sockfd);
    	pthread_create(&write_t, NULL, threadedWrite, (void*)gc_sockfd);
							
	}        

	pthread_exit(NULL);
	

	atexit(closeSockets);	
	exit(1);
}
