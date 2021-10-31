
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "commands.h"

#define SERVER_NAME_LEN_MAX 255

void print_client_menu( void )
{
    printf("*****************************************************\n");
    printf("Please enter the command \n");
    printf("1 for ADD \n");
    printf("2 for DELETE \n");
    printf("3 for LIST \n");
    printf("4 for SHUTDOWN \n");
    printf("5 for QUIT \n");
    printf("6 for LOGIN \n");
    printf("7 for LOGOUT \n");
    printf("8 for WHO \n");
    printf("9 for LOOK \n");
    printf("*****************************************************\n");
}

void process_client_command(int socket_fd , unsigned int command_choice)
{
    switch(command_choice)
    {
    case QUIT:
        client_process_quit(socket_fd);
        break;
    case LOGIN:
        client_process_login(socket_fd);
        break;
    case LOGOUT:
        client_process_logout(socket_fd);
        break;
    case WHO:
        client_process_who(socket_fd);
        break;
    case LOOK:
        client_process_look(socket_fd);
        break;
    default:
        printf("The command is not yet implemented, Please use only QUIT LOGIN, LOGOUT, WHO, LOOK\n");
        break;
    }
}

int main(int argc, char *argv[])
{
    char server_name[SERVER_NAME_LEN_MAX + 1] = { 0 };
    int server_port, socket_fd;
    struct hostent *server_host;
    struct sockaddr_in server_address;
    char message_received[300];

    /* Get server name from command line arguments or stdin. */
    if (argc > 1)
    {
        strncpy(server_name, argv[1], SERVER_NAME_LEN_MAX);
    }
    else
    {
        printf("Enter Server Name: ");
        scanf("%s", server_name);
    }

    /* Get server port from command line arguments or stdin. */
    server_port = argc > 2 ? atoi(argv[2]) : 0;
    if (!server_port)
    {
        printf("Enter Port: ");
        scanf("%d", &server_port);
    }

    /* Get server host from server name. */
    server_host = gethostbyname(server_name);

    /* Initialise IPv4 server address with server host. */
    memset(&server_address, 0, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Connect to socket with server address. */
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
        perror("connect");
        exit(1);
    }

    bool is_client_active = true;
    while(is_client_active)
    {
        unsigned int choice;
        print_client_menu();
        scanf("%d",&choice);
        process_client_command(socket_fd, choice);
    }

    return 0;
}