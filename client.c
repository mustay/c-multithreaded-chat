#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#define MASTER 100
#define SLAVE 200

int gsockfd; // global sock fd.

int REG_FLAG = 0; // flag used for user registration validation.

char g_name[40]; // global name.
char g_addr[40]; // global address.

void closeSockets(void) // close socket function called atexit()
{
	close(gsockfd);
}

void* test(void* arg) ////////////////////////////////////// CHANGE NAME
{
	char* x = g_addr;
	char* pch = x;
    char* addr;
    char* stat;
    
    const char address[20];
    const char status[15];
    pch = strtok (x,",-");
    addr = strtok (NULL, ",-");
    stat = strtok (NULL, ",-");
    
    strcpy(address, addr);
    strcpy(status, stat);
    
    char buffer[BUFSIZ + 1];
    pid_t fork_result;
    memset(buffer, '\0', sizeof(buffer));

    fork_result = fork();
    
    if (fork_result == -1) {
        fprintf(stderr, "Fork failure");
    }
    if (fork_result == 0) {
        printf("Forking");
        execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", "./chat", address, status, (void*)NULL);
    }
    pthread_exit(NULL);
}

void* threadedRead(void *arg)
{
    int client_sockfd = (int) arg;
    char msg[40];
    pthread_t init_conn;
    int result;
    while(1) {
        sleep(2);
        
        result = read(client_sockfd, &msg, sizeof(msg));

        if(result == 0) { // Close socket and exit
        	atexit(closeSockets);
        	exit(-1);
        }
    
        if(strncmp(msg, "xyzinit", 7) == 0) { // MSG for client-client comm initiation.
        	strcpy(g_addr, msg);
            pthread_create(&init_conn, NULL, test, NULL);
        }
        else if(strncmp(msg, "xyzu", 4) == 0) { // MSG for new user registration.
        
            if(strncmp(msg, "xyzunew", 7) == 0) { // MSG from server notifying user has been registered.
                REG_FLAG = 2;
            }
            else if(strncmp(msg, "xyzuexist", 9) == 0) { // MSG from server notifying user name unavailable;
                REG_FLAG = 1;
                printf("[SERVER]: User already exists\n");
            }
        }
        else
            printf("[MSG]: %s\n", msg);
            
        fflush(stdout);
        memset(msg, '\0', sizeof(msg));
    }
    pthread_exit(NULL);
}

void* threadedWrite(void *arg) 
{
    int client_sockfd = (int) arg;
    char msg[30];
    
    while(1) {
        if(write(client_sockfd, &msg, strlen(msg)) != -1) { 
            memset(msg, '\0', sizeof(msg));
        }
    }
    pthread_exit(NULL);
}

int reg_user(int sockfd) // User Registration
{
	
    while(REG_FLAG != 2) { // Waits for FLAG Authorization from server.
        char my_name[30], r_pass[10], data[50] = "xyzreg,";
        printf("\nEnter Name: ");
        scanf("%s", my_name);

        printf("Enter Password: ");
        scanf("%s", r_pass);

        strcat(data, my_name);
        strcat(data, ".");
        strcat(data, r_pass);
        write(sockfd, &data, strlen(data));
        printf("\nPlease Wait. Attempting to Register");
        strcpy(g_name, my_name);
        sleep(5);
    }
    return 1;
}


int main()
{
    
    int len;
    struct sockaddr_in address;
    int result;
    
    gsockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("192.168.56.1");
    address.sin_port = htons(9734);
    len = sizeof(address);
    
    result = connect(gsockfd, (struct sockaddr *) &address, len);
    
    pthread_t read_t, write_t;
    
    if(result == -1) {
        perror("client error");
        exit(1);
    }
    
    pthread_create(&read_t, NULL, threadedRead, (void*)gsockfd);
    pthread_create(&write_t, NULL, threadedWrite, (void*)gsockfd);
    
    /* User Registration Validation */
    int reg_check = 0;
    while(reg_check != 1)
    {
        reg_check = reg_user(gsockfd);
        
        if(reg_check == 1) {
            printf("\nRegistered Successfully");
        }    
        sleep(1);
    }
    
    int opt = 5;
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\t\t\tWelcome to the chatroom\n");
    printf("\t\t\tConnected as: %s\n", g_name);
    printf("\t\n");
    printf("\t1. Connect\n");
    printf("\t2. List\n");
    printf("\t3. Quit\n");
    printf("\tEnter (1-3): \n");
    scanf("%d", &opt);    
    
    while(opt != 3) {
        if(opt == 1) {
            fflush(stdin);
            char other_name[30], con_data[50] = "xyzser,";
            printf("\nEnter User Name: ");
            scanf("%s", other_name);
            
            if(strcmp(g_name, other_name) != 0){ // Checking if user attempting to connect to SELF.
		        strcat(con_data, g_name);
		        strcat(con_data, "-");
		        strcat(con_data, other_name);
		    
		        write(gsockfd, &con_data, strlen(con_data));
            }           
        }
        else if(opt == 2) {
            write(gsockfd, "list", 4);
        }
        else if(opt == 3)
        {
            
        }
        
        printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        printf("\t\t\tWelcome to the chatroom\n");
        printf("\t\t\tConnected as: %s\n", g_name);
        printf("\t\n");
        printf("\t1. Connect\n");
        printf("\t2. List\n");
        printf("\t3. Quit\n");
        printf("\tEnter (1-3): \n");
        scanf("%d", &opt);
    } 
    pthread_exit(NULL);
}
