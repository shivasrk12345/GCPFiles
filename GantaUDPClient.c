// UDP Client program

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

// initializing the socket and buffersize
int sockett = 0;
#define BUF_SIZE 1024

// function to display the welcome message
void display();
char quit[] = "quit";
char join_func[] = "join";
char error[] = "error";
char error2[] = "badgroup";
char joined[] = "joined";

// function to listen messages 
void *listen_messages(void *voidArg);

// structure to create the socket
struct sockaddr_in udpserverr;


// function to send data from receiver to relay server
void serverInformationfun(char c[256])
{
    sendto(sockett, c, strlen(c), 0, (struct sockaddr *) &udpserverr, sizeof(udpserverr));
}

// main function of the program
int main(int argc, char *argv[]) {
    display();
    int statuss = 0;
    char buffer[BUF_SIZE];
// initializing the group name
    char* group_name = NULL;
    int simplePort = 0;
    if (3 != argc) {
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }
// creating the socket
    sockett = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockett == -1) {
        fprintf(stderr, "\nCould not create a socket\n");
        exit(1);
    } else {
        fprintf(stderr, "\nSocket Created\n");
    }
    simplePort = atoi(argv[2]);
    bzero(&udpserverr, sizeof(udpserverr));
    udpserverr.sin_family = AF_INET;
    inet_addr(argv[1], &udpserverr.sin_addr.s_addr);
    udpserverr.sin_port = htons(simplePort);
    size_t size;
    
// getting group name from the client to join
       printf("\n---------------------------------------------------------------------\n");
       fprintf(stderr, "Please Enter the Group Name you want to Join");
       printf("\n---------------------------------------------------------------------\n");

    getline(&group_name, &size, stdin);
    //strip newline character if present
    if (group_name[strlen(group_name) - 1] == '\n') {
        group_name[strlen(group_name) - 1] = '\0';
    }

    int join_group = 1;
    size_t len;
    char *input = NULL;
    int sizeoff_server = sizeof(udpserverr);
// infinite loop to ask clients do they need to join the group or not
    while (join_group) {
        int statuss = 0;
        input = NULL;
        fprintf(stderr, "Would you like to join the group?");
	fprintf(stderr, "\nPlease Enter join to join the group and not join to exit from the group \n");
        getline(&input, &len, stdin);
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }
        for (int i = 0; input[i]; i++) {
            input[i] = tolower(input[i]);
        }
// exit the program if the user species NOTJOIN or notjoin
        if (strcmp(input, "notjoin") == 0) {
            fprintf(stderr, "\nExiting From The Client Program\n");
            close(sockett);
            exit(0);
// join message to join the group
        } else if (strcmp(input, "join") != 0) {
            fprintf(stderr, "\nPlease Enter the Valid Input\n");
        } else {

            char join[strlen(join_func) + strlen(group_name) + 1];
            strcpy( join, join_func );
            strcat( join, group_name );
            statuss = sendto(sockett, join, strlen(join), 0, (struct sockaddr *) &udpserverr, sizeof(udpserverr));
// displaying error message incase of status lessthan or equal to zero
            if (statuss <= 0)
            {
                fprintf(stderr, "\nUnable To Join and Terminating From The Client Instance.\n");
                close(sockett);
                exit(-1);
            }
// to read the buffer message
            bzero(buffer, BUF_SIZE);
            statuss = recvfrom(sockett, buffer, sizeof(buffer), 0, (struct sockaddr *) &udpserverr, &sizeoff_server);
            if (buffer[strlen(buffer) - 1] == '\n') {
                buffer[strlen(buffer) - 1] = '\0';
            }
            if (strcmp(buffer, joined) == 0)
            {
// displaying the message to the client after joining the group
		printf("\n-----------------------------------------------------------------------------\n");
                fprintf(stdout, "\nJoining the Group %s Please Enter 'quit' or 'QUIT' to Quit From the Group\n", group_name);
		printf("\n-----------------------------------------------------------------------------\n");
		fprintf(stdout, "\nPlease Enter 'quit' or 'QUIT' To Quit From The Group\n");
                join_group = 0;
            }
            else if (strcmp(buffer, error) == 0)
            {
                fprintf(stderr, "\nGroup is Full Please Try Again\n");
            }
// displaying error message incase of group doesn't exist
            else if (strcmp(buffer, error2) == 0)
            {
                fprintf(stderr, "\nGroup doesn't Exist Please Enter the Appropriate Group Name\n");
            }
        }
    }

// thread to listen incoming messages
    pthread_t thread_message;
    pthread_create(&thread_message, NULL, listen_messages, NULL);
    int quitt = 1;
    while (quitt)
    {
        input = NULL;
        getline(&input, &len, stdin);
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }
        for (int i = 0; input[i]; i++) {
            input[i] = tolower(input[i]);
        }

// comparing the message entered is equal to quit or not
        if (strcmp(input, "quit") != 0)
        {
            fprintf(stderr, " Please Enter correct input or  'quit' \n");
        }
// removing the user incase of quit message received from the client
        else
        {
            quitt = 0;
            fprintf(stderr, "\nUser Removed From the Group and Exiting the Program\n");
            //send quit message
            statuss = sendto(sockett, quit, strlen(quit), 0, (struct sockaddr *) &udpserverr, sizeof(udpserverr));
            if (statuss <= 0)
            {
                fprintf(stderr, "\nUnable to quit and Terminating the client instance\n");
            }
            pthread_cancel(thread_message);
        }

    }
    close(sockett);
    exit(0);

}

// function to listen messages
void *listen_messages(void *voidArg)
{
    int status1;
    int sizeoff_server = sizeof(udpserverr);
    char buffer[BUF_SIZE];
// infinite loop to receive and display the message from the server
    while(1)
    {
        bzero(buffer, BUF_SIZE);
        status1 = recvfrom(sockett, buffer, sizeof(buffer), 0, (struct sockaddr *) &udpserverr, &sizeoff_server);
// displaying the received message from the server
        if (status1 > 0) {

	    printf("\n---------------------------------------------------------------------\n");
            fprintf(stderr, " Message received  from server is: %s\n", buffer);
	    printf("\n---------------------------------------------------------------------\n");
// sending acknowledgement after receiving the message
        char c[10] = "ack";
        //sendto(sockett, c, strlen(c), 0, (struct sockaddr *) &udpserverr, sizeof(udpserverr));

        }
// displaying error message incase of error in receiving messages from the server
        else
        {
	    printf("\n---------------------------------------------------------------------\n");
            fprintf(stderr, "Error occured at the time of Receving Message\n");
	    printf("\n---------------------------------------------------------------------\n");
        }
    }
}

void display()
    {
	  printf("\n---------------------------------------------------------------------\n");
	  printf("\n\t\t\t UDP CLIENT PROGRAM");
	  printf("\n---------------------------------------------------------------------\n");
     }


