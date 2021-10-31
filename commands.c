#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "commands.h"

/* Function to read from the client */
void read_with_error_check(int socket_fd, void *buffer, int buffer_size)
{
    int read_bytes = read(socket_fd, buffer, buffer_size);
    if (read_bytes == 0)
    {
        printf("Client closed the connection\n");
        pthread_exit(0);
    }
    else if (read_bytes < 0)
    {
        perror("socket");
        pthread_exit(-1);
    }
}

void write_with_error_check(int socket_fd, void * buffer, int buffer_size)
{
    int written_bytes = write(socket_fd, buffer, buffer_size);
    if (written_bytes == 0)
    {
        printf("Client closed the connection\n");
        pthread_exit(0);
    }
    else if (written_bytes < 0)
    {
        perror("socket");
        pthread_exit(-1);
    }
}

void client_process_quit(int socket_fd)
{
    _command_payload tx_command_payload = { 0 };
    memcpy(tx_command_payload.command_id, QUIT_STRING, QUIT_STRING_SIZE);
    write_with_error_check(socket_fd, &tx_command_payload, sizeof(tx_command_payload));

    _response_payload server_response = { 0 };
    do
    {
        memset(&server_response, 0x00, sizeof(server_response));
        read_with_error_check(socket_fd, &server_response, sizeof(server_response));
        printf("%s", server_response.response_payload);
    } while (server_response.number_of_response_in_flight > 0);

    pthread_exit(0);
}

void client_process_login(int socket_fd)
{
    _command_payload tx_command_payload = { 0 };
    _command_login tx_command_login = { 0 };

    printf("\n Enter the user-id \n ");
    scanf("%s", tx_command_login.user_id);

    printf("\n Enter the password \n ");
    scanf("%s", tx_command_login.password);

    memcpy(tx_command_payload.command_id, "LOGIN", sizeof("LOGIN"));
    memcpy(tx_command_payload.command_payload, &tx_command_login, sizeof(tx_command_login));
    write_with_error_check(socket_fd, &tx_command_payload, sizeof(tx_command_payload));

    _response_payload server_response = { 0 };
    do
    {
        read_with_error_check(socket_fd, &server_response, sizeof(server_response));
        printf("%s", server_response.response_payload);
    } while (server_response.number_of_response_in_flight > 0);
}

void client_process_logout(int socket_fd)
{
    _command_payload tx_command_payload = { 0 };
    memcpy(tx_command_payload.command_id, LOGOUT_STRING, LOGOUT_STRING_SIZE);
    write_with_error_check(socket_fd, &tx_command_payload, sizeof(tx_command_payload));

    _response_payload server_response = { 0 };
    do
    {
        read_with_error_check(socket_fd, &server_response, sizeof(server_response));
        printf("%s", server_response.response_payload);
    } while (server_response.number_of_response_in_flight > 0);
}

void client_process_who(int socket_fd)
{
    _command_payload tx_command_payload = { 0 };
    memcpy(tx_command_payload.command_id, WHO_STRING, WHO_STRING_SIZE);
    write_with_error_check(socket_fd, &tx_command_payload, sizeof(tx_command_payload));

    _response_payload server_response = { 0 };
    do
    {
        memset(&server_response, 0x00, sizeof(server_response));
        read_with_error_check(socket_fd, &server_response, sizeof(server_response));
        printf("%s", server_response.response_payload);
    } while (server_response.number_of_response_in_flight > 0);
}

void client_process_look(int socket_fd)
{
    _command_payload tx_command_payload = { 0 };
    _command_look tx_command_who = { 0 };
    memcpy(tx_command_payload.command_id, LOOK_STRING, LOOK_STRING_SIZE);

    printf("Enter the search parameter type \n");
    scanf("%d", &tx_command_who.search_type);

    printf("Enter the search string \n");
    scanf("%s", tx_command_who.search_string);

    memcpy(tx_command_payload.command_payload, &tx_command_who, sizeof(tx_command_who));
    write_with_error_check(socket_fd, &tx_command_payload, sizeof(tx_command_payload));

    _response_payload server_response = { 0 };
    do
    {
        read_with_error_check(socket_fd, &server_response, sizeof(server_response));
        printf("%s", server_response.response_payload);
    } while (server_response.number_of_response_in_flight > 0);
}

