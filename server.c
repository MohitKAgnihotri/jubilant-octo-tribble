#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include "server.h"
#include "commands.h"

#define BACKLOG 10
#define PORT_NUM 1313

_server_address_book server_user_data[MAX_ADDRESS_BOOK_SIZE] = {{.record_index = 1001, .first_name = "David", .last_name = "Miller", .phone_number= "313-510-6001"},
                                                                {.record_index = 1004, .first_name = "John", .last_name = "Miller", .phone_number= "315-123-1345"}};

/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

void SetupSignalHandler();

int CreateServerSocket(int port);

int server_socket_fd;

_server_user_data_struct authenticated_user_list[4] = {{.isLoggedIn = false, .user_id = "root", .password = "root01",""},
                                                    {.isLoggedIn = false, .user_id = "john", .password = "john01",""},
                                                    {.isLoggedIn = false, .user_id = "david", .password = "david01",""},
                                                    {.isLoggedIn = false, .user_id = "mary", .password = "mary01",""}};

int main(int argc, char *argv[])
{
    int port, new_socket_fd;
    pthread_attr_t pthread_client_attr;
    pthread_t pthread;
    socklen_t client_address_len;
    struct sockaddr_in client_address;

    /* Get port from command line arguments or stdin.
     * For this server, this is fixed to 1113*/
    port = PORT_NUM;

    /*Create the server socket */
    server_socket_fd = CreateServerSocket(port);

    /*Setup the signal handler*/
    SetupSignalHandler();

    /* Initialise pthread attribute to create detached threads. */
    if (pthread_attr_init(&pthread_client_attr) != 0) {
        perror("pthread_attr_init");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_client_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(1);
    }

    while (1) {

        /* Accept connection to client. */
        client_address_len = sizeof (client_address);
        new_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_address_len);
        if (new_socket_fd == -1) {
            perror("accept");
            continue;
        }

        /*Create client specific data structure*/
        _client_interface *thread_arg = (_client_interface *) malloc(sizeof(_client_interface));
        thread_arg->client_socket = new_socket_fd;

        /*Get the clients IP address */
        struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_address;
        struct in_addr ipAddr = pV4Addr->sin_addr;
        inet_ntop( AF_INET, &ipAddr, thread_arg->str, INET_ADDRSTRLEN );

        printf("Client connected from %s\n", thread_arg->str);

        /* Create thread to serve connection to client. */
        if (pthread_create(&pthread, &pthread_client_attr, pthread_routine, (void *)thread_arg) != 0) {
            perror("pthread_create");
            continue;
        }
    }

    return 0;
}

/*
 * Function to create the server socket.
 * */

int CreateServerSocket(int port)
{
    struct sockaddr_in address;
    int socket_fd;

    /* Initialise IPv4 address. */
    memset(&address, 0, sizeof (address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    /* Bind address to socket. */
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof (address)) == -1) {
        perror("bind");
        exit(1);
    }

    /* Listen on socket. */
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Configure server socket
    int enable = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    return socket_fd;
}

/* Function to configure the signal handlers. */
void SetupSignalHandler()
{
    /* Assign signal handlers to signals. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
}



void server_send_login_response(int socket_fd, bool is_login_success)
{
    _response_payload responsePayload = {0};
    responsePayload.is_sucessfully_executed = is_login_success;
    memcpy(responsePayload.command_id,LOGIN_STRING,LOGIN_STRING_SIZE);
    if (is_login_success)
        memcpy(responsePayload.response_payload,SERVER_200_STRING, SERVER_200_STRING_SIZE);
    else
        memcpy(responsePayload.response_payload,SERVER_410_STRING, SERVER_410_STRING_SIZE);
    responsePayload.number_of_response_in_flight = 0;

    write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));
}

void server_send_logout_response(int socket_fd, bool isLoggedIn)
{
    _response_payload responsePayload = {0};
    responsePayload.is_sucessfully_executed = isLoggedIn;
    memcpy(responsePayload.command_id,LOGOUT_STRING,LOGOUT_STRING_SIZE);
    if (isLoggedIn)
        memcpy(responsePayload.response_payload,SERVER_200_STRING, SERVER_200_STRING_SIZE);
    else
        memcpy(responsePayload.response_payload,SERVER_302_STRING, SERVER_302_STRING_SIZE);
    responsePayload.number_of_response_in_flight = 0;
    write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));
}

void server_send_quit_response(int socket_fd)
{
    _response_payload responsePayload = {0};
    responsePayload.is_sucessfully_executed = true;
    memcpy(responsePayload.command_id,LOGOUT_STRING,LOGOUT_STRING_SIZE);
    memcpy(responsePayload.response_payload,SERVER_200_STRING, SERVER_200_STRING_SIZE);
    responsePayload.number_of_response_in_flight = 0;
    write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));
}

void server_send_look_response(int socket_fd, _command_look *command_look)
{
    int num_of_match_found = 0;
    int valid_array_idex[MAX_ADDRESS_BOOK_SIZE];
    int current_index = 0;
    memset(valid_array_idex, -1, MAX_ADDRESS_BOOK_SIZE);

    for (int i = 0; i < sizeof(server_user_data)/sizeof(server_user_data[0]); i++)
    {
        if (command_look->search_type == search_first_name)
        {
            if (strcmp(server_user_data[i].first_name, command_look->search_string) == 0)
            {
                num_of_match_found++;
                valid_array_idex[current_index++] = i;
            }
        }
        else if (command_look->search_type == search_last_name)
        {
            if (strcmp(server_user_data[i].last_name, command_look->search_string) == 0)
            {
                num_of_match_found++;
                valid_array_idex[current_index++] = i;
            }
        }
        else if (command_look->search_type == search_phone_number)
        {
            if (strcmp(server_user_data[i].phone_number, command_look->search_string) == 0)
            {
                num_of_match_found++;
                valid_array_idex[current_index++] = i;
            }
        }
    }

    _response_payload responsePayload = { 0 };
    responsePayload.is_sucessfully_executed = true;
    memcpy(responsePayload.command_id, LOOK_STRING, LOOK_STRING_SIZE);
    if (num_of_match_found == 0)
    {
        sprintf(responsePayload.response_payload, "%s \n", SERVER_404_STRING);
        responsePayload.number_of_response_in_flight = 0;
    }
    else
    {
        sprintf(responsePayload.response_payload, "%s \n %s %d %s \n", SERVER_200_STRING, "Found", num_of_match_found, "match");
        responsePayload.number_of_response_in_flight = 1;
    }
    write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));

    for (int i = 0; i < current_index; i++)
    {
        _response_payload responsePayload = {0};
        responsePayload.is_sucessfully_executed = true;
        memcpy(responsePayload.command_id,LOOK_STRING,LOOK_STRING_SIZE);
        sprintf(responsePayload.response_payload, "%d \t %s \t %s \t %s \n", server_user_data[valid_array_idex[i]].record_index,
            server_user_data[valid_array_idex[i]].first_name,
            server_user_data[valid_array_idex[i]].last_name,
            server_user_data[valid_array_idex[i]].phone_number);
        responsePayload.number_of_response_in_flight = 1;
        write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));
    }

    responsePayload.is_sucessfully_executed = true;
    memcpy(responsePayload.command_id,LOOK_STRING,LOOK_STRING_SIZE);
    memset(responsePayload.response_payload,0x00, sizeof(responsePayload.response_payload));
    responsePayload.number_of_response_in_flight = 0;
    write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));
}

void server_send_who_response(int socket_fd)
{
    _response_payload responsePayload = {0};
    bool least_one_user_online = false;
    responsePayload.is_sucessfully_executed = true;
    memcpy(responsePayload.command_id,WHO_STRING,WHO_STRING_SIZE);

    for (int i = 0; i < sizeof (authenticated_user_list) / sizeof authenticated_user_list[0]; i++)
    {
        if (authenticated_user_list[i].isLoggedIn)
        {
            least_one_user_online = true;
            break;
        }
    }

    if (least_one_user_online)
    {
        sprintf(responsePayload.response_payload, "%s \n %s \n", SERVER_200_STRING, "The list of the active users:");
        responsePayload.number_of_response_in_flight = 1;
    }
    else
    {
        sprintf(responsePayload.response_payload, "%s \n %s \n", SERVER_200_STRING, "NO active users at the moment");
        responsePayload.number_of_response_in_flight = 0;
    }

    write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));
    for (int i = 0; i < sizeof (authenticated_user_list) / sizeof authenticated_user_list[0]; i++)
    {
        if (authenticated_user_list[i].isLoggedIn)
        {
            _response_payload responsePayload = {0};
            responsePayload.is_sucessfully_executed = true;
            memcpy(responsePayload.command_id,WHO_STRING,WHO_STRING_SIZE);
            sprintf(responsePayload.response_payload, "%s \t %s \n", authenticated_user_list[i].user_id, authenticated_user_list[i].last_known_ip);
            responsePayload.number_of_response_in_flight = 1;
            write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));
        }
    }

    _response_payload final_responsePayload = {0};
    responsePayload.is_sucessfully_executed = true;
    memcpy(responsePayload.command_id,WHO_STRING,WHO_STRING_SIZE);
    sprintf(responsePayload.response_payload, "%s\n","");
    responsePayload.number_of_response_in_flight = 0;
    write_with_error_check(socket_fd, &responsePayload, sizeof(responsePayload));
}

void server_process_command(_client_interface *client_interface, _command_payload *received_command)
{
    if (strncmp(received_command->command_id,LOGIN_STRING, LOGIN_STRING_SIZE) == 0)
    {
        bool is_login_success = false;
        printf("Received Command = %s\n",received_command->command_id);
        _command_login *received_login_command = (_command_login *)&received_command->command_payload[0];
        printf("Received UserId  = %s\n", received_login_command->user_id);
        printf("Received Password = %s\n", received_login_command->password);

        for(int i = 0; i < (sizeof(authenticated_user_list)/sizeof(authenticated_user_list[0]));i++)
        {
            if (strncmp(authenticated_user_list[i].user_id,received_login_command->user_id,strlen(authenticated_user_list[i].user_id)) == 0
            && strncmp(authenticated_user_list[i].password,received_login_command->password,strlen(authenticated_user_list[i].password)) == 0 )
            {
                authenticated_user_list[i].isLoggedIn = true;
                strcpy(authenticated_user_list[i].last_known_ip, client_interface->str);
                client_interface->client_index = i;
                is_login_success = true;
                break;
            }
        }
        server_send_login_response(client_interface->client_socket, is_login_success);
    }
    else if (strncmp(received_command->command_id,LOGOUT_STRING, LOGOUT_STRING_SIZE) == 0)
    {
        printf("Received Command = %s\n",received_command->command_id);
        server_send_logout_response(client_interface->client_socket, authenticated_user_list[client_interface->client_index].isLoggedIn);
        authenticated_user_list[client_interface->client_index].isLoggedIn = false;
    }
    else if (strncmp(received_command->command_id,QUIT_STRING, QUIT_STRING_SIZE) == 0)
    {
        printf("Received Command = %s\n",received_command->command_id);
        server_send_quit_response(client_interface->client_socket);
        authenticated_user_list[client_interface->client_index].isLoggedIn = false;
    }
    else if (strncmp(received_command->command_id,WHO_STRING, WHO_STRING_SIZE) == 0)
    {
        printf("Received Command = %s\n",received_command->command_id);
        server_send_who_response(client_interface->client_socket);
    }
    else if (strncmp(received_command->command_id,LOOK_STRING, LOOK_STRING_SIZE) == 0)
    {
        printf("Received Command = %s\n",received_command->command_id);
        _command_look *received_look_command = (_command_look *)&received_command->command_payload[0];
        server_send_look_response(client_interface->client_socket, received_look_command);
    }
    else
    {

    }
}

void *pthread_routine(void *arg)
{
    _client_interface client;
    bool is_client_connected = true;
    client.client_socket = ((_client_interface*)arg)->client_socket;
    memcpy(client.str, ((_client_interface*)arg)->str, strlen(((_client_interface*)arg)->str));

    while(is_client_connected)
    {
        // Read command frame from the client
        _command_payload received_command = {0};
        read_with_error_check(client.client_socket, &received_command, sizeof(_command_payload));
        server_process_command(&client,&received_command);
    }

    free(arg);
    return NULL;
}

void signal_handler(int signal_number)
{
    close(server_socket_fd);
    exit(0);
}