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


int ENTRY_FLAG = 0;
int gserver_sockfd, gclient_sockfd;

struct Node{
	struct Node *next;
    int sock;
    char name[30];
    char pass[10];
	char addr[15];
};

struct List{
	struct Node *first;
	struct Node *last;
}list;

void init(struct List *list){
	list->first=0;
	list->last=0;
}

void add(struct List *list, char name[], char pass[], char address[], int sock)
{
	struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    strcpy(node->name, name);
    strcpy(node->pass, pass);
    strcpy(node->addr, address);
    node->sock = sock;
	node->next=NULL;
	if(!list->first){
		list->first=node;
		list->last = list->first;
	}
	else{
		list->last->next=node;
		list->last=list->last->next;
	}
}

int search(struct List *list, char* name, char** address, int** sock)
{
	struct Node *itr;
	for (itr = list->first; itr!=NULL; itr=itr->next)
	{
		if (strncmp(itr->name, name, strlen(name)) == 0)
		{
            *address = itr->addr;
            *sock = itr->sock;
			return 1;
		}
	}
	return 0;
}
/*
int del(struct List *list, int data)
{
	struct Node *itr;
	struct Node *secondLast;
	struct Node *temp;
	if(list->first==NULL)
		return 0;
	if (list->first->data==data){
		if(list->first==list->last)
		{
			free(list->first);
			list->first=list->last=0;
			return 1;
		}
		else
		{
			struct Node *temp = list->first->next;
			free(list->first);
			list->first=temp;
			return 1;
		}
	}
	
	for (itr=list->first; itr->next!=NULL;secondLast=itr,itr=itr->next)
	{
		if (itr->next->data==data)
		{
			if(itr->next==list->last)
				list->last=itr;
			temp =itr->next->next;
			free(itr->next);
			itr->next = temp;
			
			return 1;
		}
	}
	return 0;
}
*/

void print(struct List *list)
{
	struct Node *itr;
	for (itr=list->first; itr!=NULL;itr=itr->next)
	{
        printf("Name: %s\n", itr->name);
        printf("Pass: %s\n", itr->pass);
		printf("Address: %s\n", itr->addr);
	}
}

void* threadedListen(void *arg)
{
    int server_sockfd = (int) arg;
    printf("\n[Server]: Session Created\n");
    
    while(1) {
        listen(server_sockfd, 5);   
    }
    
    pthread_exit(NULL);
}

void sendList(int sockfd) // Sends list of clients.
{
    struct Node* itr = list.first;
    
    char n[30];
    
    if(itr) {
        while(itr) {
            strcpy(n, itr->name);
            write(sockfd, &n, sizeof(n));
            
            itr = itr->next;
            sleep(1);
        }
    }
}

void* threadedRead(void *arg)
{
    char m[30] = " ";
    memset(m, '\0', sizeof(m));
    
    struct Node* n = (struct Node*) arg;
    int client_sockfd = n->sock;
    char* client_address = n->addr;
    
    while(1) {
        read(client_sockfd, &m, sizeof(m));
        if(strncmp(m, "list", 4) == 0) {
            sendList(client_sockfd);
            memset(m, '\0', sizeof(m));
        }
        else if(strncmp(m, "xyzreg", 6) == 0) { // Message for Registration
            char* name;
            char* pass;
            char* pch = m;
            
            pch = strtok (m,",.");
            
            name = strtok (NULL, ",.");
            pass = strtok (NULL, ",.");
            
            int* search_sock;
            char* search_addr;
        
          	if(search(&list, name, &search_addr, &search_sock)) {
          		char* search_msg = "xyzuexist";
          		write(client_sockfd, search_msg, strlen(search_msg));
          	}
          	else {
	            add(&list, name, pass, client_address, client_sockfd);  
	        	char* search_msg = "xyzunew";
          		write(client_sockfd, search_msg, strlen(search_msg));    
	        }
            memset(m, '\0', sizeof(m));
        }
        else if(strncmp(m, "xyzser", 6) == 0) { // Message for Connection
            char* name;
            int* name_sock;
            char* name_addr;
            
            char* other_name;
            int* other_sock;
            char* other_addr;
            
            char* pch = m; 
            /* Seperating data from the recieved string */
            pch = strtok (m,",.-");
            name = strtok (NULL, ",.-");
            other_name = strtok (NULL, ",.-");            
            
            if(search(&list, name, &name_addr, &name_sock)) {
                if(search(&list, other_name, &other_addr, &other_sock)) {
                    
                    char name_msg[50], other_msg[50];
                    /* MAS for Master */
                    /* SAL for Slave */
                    
                    strcat(name_msg, "xyzinit,");
                    strcat(name_msg, name_addr);
                    strcat(name_msg, "-MAS");
                    write(other_sock, name_msg, strlen(name_msg));
                    
                    sleep(1);
                    
                    strcat(other_msg, "xyzinit,");
                    strcat(other_msg, other_addr);
                    strcat(other_msg, "-SAL");
                    write(name_sock, other_msg, strlen(other_msg));  
                }
            }
            memset(m, '\0', sizeof(m));
        }
    }
    pthread_exit(NULL);
}

void* threadedWrite(void *arg) 
{
    int client_sockfd = (int) arg;
    char msg[30];
    
    while(1) {
        write(client_sockfd, &msg, strlen(msg));
        memset(msg, '\0', sizeof(msg));
    }
    pthread_exit(NULL);
}

int main()
{
    
    init(&list);
    
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    
    unlink("server_socket");
    
    gserver_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);
        
    bind(gserver_sockfd, (struct sockaddr *)&server_address, server_len);
    
    pthread_t listen_t, read_t, write_t;
    
    pthread_create(&listen_t, NULL, threadedListen, (void *) gserver_sockfd);
    
    while(1) {
        client_len = sizeof(client_address);
        gclient_sockfd = accept(gserver_sockfd, (struct sockaddr*) &client_address, &client_len);
        
        struct Node *client_node;
        client_node = malloc(sizeof(struct Node));
        client_node->sock = gclient_sockfd;
        strcpy(client_node->addr, inet_ntoa(client_address.sin_addr));
        
        if(ENTRY_FLAG != 0)
        {
            printf("[Server]: Connected to %s\n", inet_ntoa(client_address.sin_addr));
        }
        
        ENTRY_FLAG = 1;
        pthread_create(&read_t, NULL, threadedRead, (void*) client_node);
        
        
        pthread_create(&write_t, NULL, threadedWrite, (void*) gclient_sockfd);           
    }
    pthread_exit(NULL);
    
    close(gserver_sockfd);
    close(gclient_sockfd);
}
