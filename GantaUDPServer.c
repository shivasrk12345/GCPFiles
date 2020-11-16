//UDP server program

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

// intializing maximum users and socket
int maxusers_count = 0;
int number_of_users = 0;
int sockett = 0;

// intializing group name and buffer size
char* group_name = NULL;
#define BUF_SIZE 1024

//added code
char ackbuffer[10];
char ackmsg[4] = "ack";
char multicastmsg[10] = " ,multicasted to";
char unicastmsg[8] = "unicast";
char actualbroadcastmsg[1024];
char actualbroadcastmsg1[1024];


// creating socket address
struct sockaddr_in users[100] = {};
struct sockaddr_in udpserverr;
struct sockaddr_in client;


void *Clients_Acceptfunction(void *voidArg)
{

    char quit[] = "quit";
    char join_func[] = "join";
    char error[] = "error";
    char error2[] = "badgroup";
    char joined[] = "joined";
    char join[strlen(join_func) + strlen(group_name) + 1];
    strcpy( join, join_func );
    strcat( join, group_name );
    int valuee;
    int user_sizee = sizeof(client);
    char buffer[BUF_SIZE];
// infinite loop to listen client messages


    for(;;)
    {
        bzero(buffer, BUF_SIZE);
        bzero(&client, sizeof(client));
        valuee = recvfrom(sockett, buffer, sizeof(buffer), 0, (struct sockaddr *) &client, &user_sizee);       

        printf(" msg from client : %d", valuee);
        if (valuee >= 0) {
            if (buffer[strlen(buffer) - 1] == '\n') {
                buffer[strlen(buffer) - 1] = '\0';
            }
// display the message received from the client
	        printf("\n---------------------------------------------------------------------\n");
            fprintf(stderr, "Following Message Received from the Client: %s\n", buffer);
            printf("\n---------------------------------------------------------------------\n");
            if (strcmp(buffer, join) == 0)
            {
// joining request from the client
                if (number_of_users < maxusers_count)
                {
                    struct sockaddr_in add_client;
                   add_client.sin_family = client.sin_family;
                   add_client.sin_addr.s_addr = client.sin_addr.s_addr;
                   add_client.sin_port = client.sin_port;

                    for (int i = 0; i<maxusers_count; i++)
                    {
// adding new clients to the group
                        if (users[i].sin_family == 0)
                        {
                            fprintf(stderr, "Adding new client to the group %s\n",group_name);
                            users[i] =add_client;
                            number_of_users = number_of_users + 1;
                            sendto(sockett, joined, strlen(joined), 0, (struct sockaddr *) &client, sizeof(client));
                            break;
                        }
                    }
                }
                else
                {
                    sendto(sockett, error, strlen(error), 0, (struct sockaddr *) &client, sizeof(client));
                }
            }
// comparing the quit request
            else if (strcmp(buffer, quit) == 0)
            {
                for (int i = 0; i<maxusers_count; i++)
                {
                    if (memcmp(&users[i], &client, sizeof(client)) == 0)
                    {
                        char n_name[1024];
                        char t_temp[20];
                        char ip[25];
                        bzero(n_name, sizeof(n_name));
                        bzero(t_temp, sizeof(t_temp));
                        bzero(ip, sizeof(ip));
                        inet_ntop(AF_INET, &(users[i].sin_addr), ip, 25);
                        getnameinfo((struct sockaddr *) &client, sizeof(client), n_name, sizeof(n_name), t_temp, sizeof(t_temp), 0);
                        fprintf(stderr, "Removing %s %s from list.\n", n_name, ip);
                        struct sockaddr_in zeroedOut = { 0 };
                        users[i] = zeroedOut;
                        number_of_users = number_of_users - 1;
                        break;
                    }
                }
            }
            else if (strstr(buffer, join_func) != NULL) {
                sendto(sockett, error2, strlen(error2), 0, (struct sockaddr *) &client, sizeof(client));
            }
        }
// displaying the error message incase of error in receiving the message
        else
        {
            fprintf(stderr, "Error Occurred While Receving the Message from the Client\n");
        }
    }
}


// main function of udp server program
int main(int argc, char *argv[]) {
    printf("\n");
    printf("\n\t\t\t UDP SERVER PROGRAM");
    printf("\n");

// intializing port number and status
    int port_number = 0;
    int statuss = 0;

    if (2 != argc) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

// creating socket
    sockett = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockett == -1) {
// display error message incase of creating socket
        fprintf(stderr, "\nCould not create a Socket\n");
        exit(1);
    } else {
        fprintf(stderr, "\nSocket Created\n");
    }

// port number for listening
    port_number = atoi(argv[1]);
    bzero(&udpserverr, sizeof(udpserverr));
    udpserverr.sin_family = AF_INET;
    udpserverr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpserverr.sin_port = htons(port_number);
// binding socket
    statuss = bind(sockett,(struct sockaddr *) &udpserverr,sizeof(udpserverr));
    if (statuss == 0) {
        fprintf(stderr, "\nBind Completed\n");
    } else {
        fprintf(stderr, "\nCould Not Bind to the Address\n");
        close(sockett);
        exit(1);
    }

// creating the group for the client
    size_t size;
    printf("\n---------------------------------------------------------------------\n");
    fprintf(stderr, "\nPlease Enter The Group Name     \n");
    printf("\n---------------------------------------------------------------------\n");
// getting group name from the client
    getline(&group_name, &size, stdin);
    //strip newline character if present
    if (group_name[strlen(group_name) - 1] == '\n') {
        group_name[strlen(group_name) - 1] = '\0';
    }
// specifying maximum number of clients for the group
    printf("\n---------------------------------------------------------------------\n");
    fprintf(stderr, "\nPlease Enter the Maximum members for %s Group\n",group_name);
    printf("\n---------------------------------------------------------------------\n");
    char* t_temp = NULL;
    getline(&t_temp, &size, stdin);
    maxusers_count = atoi(t_temp);
// displaying group name and maximum users
    printf("\n\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    fprintf(stdout, "\n %s Group created and Maximum  Group size = %d\n", group_name, maxusers_count);
    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");


    // specifying maximum number of messages 
    printf("\n---------------------------------------------------------------------\n");
    fprintf(stderr, "\nPlease Enter the Maximum number of messages\n");
    printf("\n---------------------------------------------------------------------\n");
    char* t_temp1 = NULL;
    getline(&t_temp1, &size, stdin);
    int msgg_cnt = atoi(t_temp1);
    int tempmsggcount = 0;

    //start thread to listen for clients wanting to join
    pthread_t user_threadd;
    pthread_create(&user_threadd, NULL, Clients_Acceptfunction, NULL);
    while(tempmsggcount < msgg_cnt)
    {
// broadcasting the message to the clients
        char* broadcastmsgg = NULL;
	printf("\n----------------------------------------------------------------------\n");
        printf("\n Please Enter CLEARALL message UNSUBSCRIBE the Group\n");
        fprintf(stderr, " Please Enter the broadcasting  Message    \n");
	printf("\n-----------------------------------------------------------------------\n");
        getline(&broadcastmsgg, &size, stdin);
        if (broadcastmsgg[strlen(broadcastmsgg) - 1] == '\n') {
            broadcastmsgg[strlen(broadcastmsgg) - 1] = '\0';
        }
//compare the entered message with CLEARALL
        if (strcmp(broadcastmsgg, "CLEARALL") == 0)
        {
            for (int i = 0; i<maxusers_count; i++)
            {
                if (users[i].sin_family == 0)
                {
                    continue;
                }
                char n_name[1024];
                char t_temp[20];
                char ip[25];
                bzero(n_name, sizeof(n_name));
                bzero(t_temp, sizeof(t_temp));
                bzero(ip, sizeof(ip));
                inet_ntop(AF_INET, &(users[i].sin_addr), ip, 25);
                getnameinfo((struct sockaddr *)&users[i], sizeof(users[i]), n_name, sizeof(n_name), t_temp, sizeof(t_temp), 0);
// unsubscribing the group incase of CLEARALL received
                fprintf(stderr, "\nUNSUBSCRIBING the group %s %s.\n", n_name, ip);
                struct sockaddr_in zeroedOut = { 0 };
                users[i] = zeroedOut;
            }
            number_of_users = 0;
        }
        else if (strcmp(broadcastmsgg, "CLOSESERVER") == 0)
        {
            //close the server if the user enters CLOSESERVER
            fprintf(stderr, "Terminating the server program.\n");
            msgg_cnt = 0;
            pthread_cancel(user_threadd);
        }

//displaying the message to be broadcasted on the server side
        else
        {
            printf("\n*************************************************************************\n");
            fprintf(stderr, "\nBroadcasting following Message to the Clients in %s Group: %s\n", group_name, broadcastmsgg);
            printf("\n*************************************************************************\n");

            for (int i = 0; i<maxusers_count; i++)
            {
                if (users[i].sin_family != 0)
                {
                    strcpy( actualbroadcastmsg, broadcastmsgg );
                    strcat( actualbroadcastmsg, multicastmsg );
                    sendto(sockett, actualbroadcastmsg, strlen(actualbroadcastmsg), 0, (struct sockaddr *) &users[i], sizeof(users[i]));
                    // receiving acknowledgement
                    /*recvfrom(sockett, ackbuffer, sizeof(ackbuffer), 0, (struct sockaddr *) &users[i], sizeof(users[i]));
                    if (strcmp(ackbuffer, ackmsg) == 0)
                    {
                        continue;
                    }
                    else
                    {
                        // code to send unicast msg
                        while(strcmp(ackbuffer, ackmsg) != 0){
                            bzero(ackbuffer, sizeof(ackbuffer));
                            strcpy( actualbroadcastmsg1, broadcastmsgg );
                            strcat( actualbroadcastmsg1, unicastmsg );
                            sendto(sockett, actualbroadcastmsg1, strlen(actualbroadcastmsg1), 0, (struct sockaddr *) &users[i], sizeof(users[i]));
                            recvfrom(sockett, ackbuffer, sizeof(ackbuffer), 0, (struct sockaddr *) &users[i], sizeof(users[i]));
                        }

                    }*/

                }
            }
        }
        tempmsggcount++;
    }
    
    close(sockett);
    return 0;
}

// function to listen messages from the client



