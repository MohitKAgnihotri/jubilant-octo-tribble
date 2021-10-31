#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <stdbool.h>
#define MAX_COMMAND_PAYLOAD 1024u
#define MAX_COMMAND_SIZE 64u
#define MAX_USER_ID 64u
#define MAX_IP_LENGTH 64u
#define MAX_PHONE_LENGTH 64u
#define MAX_PASSWORD_LENGTH 64u
#define MAX_SEARCH_STRING 64u

#define MAX_ADDRESS_BOOK_ENTRIES 1024u


typedef struct server_address_book
{
    unsigned char first_name[MAX_USER_ID];
    unsigned char last_name[MAX_USER_ID];
    unsigned char phone_number[MAX_PHONE_LENGTH];
}_server_address_book;

typedef struct server_user_data
{
    bool isLoggedIn;
    unsigned char user_id[MAX_USER_ID];
    unsigned char password[MAX_PASSWORD_LENGTH];
    unsigned char last_known_ip[MAX_IP_LENGTH];
}_server_user_data_struct;


#define LOGIN_STRING "LOGIN"
#define LOGIN_STRING_SIZE sizeof(LOGIN_STRING)

#define LOGOUT_STRING "LOGOUT"
#define LOGOUT_STRING_SIZE sizeof(LOGOUT_STRING)

#define QUIT_STRING "QUIT"
#define QUIT_STRING_SIZE sizeof(QUIT_STRING)

#define WHO_STRING "WHO"
#define WHO_STRING_SIZE sizeof(WHO_STRING)

#define LOOK_STRING "LOOK"
#define LOOK_STRING_SIZE sizeof(LOOK_STRING)

#define SERVER_200_STRING "200 OK\n"
#define SERVER_200_STRING_SIZE sizeof(SERVER_200_STRING)

#define SERVER_300_STRING "300 INVALID COMMAND\n"
#define SERVER_300_STRING_SIZE sizeof(SERVER_300_STRING)

#define SERVER_301_STRING "301 MESSAGE FORMAT ERROR\n"
#define SERVER_301_STRING_SIZE sizeof(SERVER_301_STRING)

#define SERVER_302_STRING "302 USER NOT LOGGED IN\n"
#define SERVER_302_STRING_SIZE sizeof(SERVER_302_STRING)

#define SERVER_410_STRING "410 WRONG USERID OR PASSWORD\n"
#define SERVER_410_STRING_SIZE sizeof(SERVER_410_STRING)

#define SERVER_404_STRING "404 YOUR SEARCH DID NOT MATCH ANY RECORDS\n"
#define SERVER_404_STRING_SIZE sizeof(SERVER_404_STRING)

#define SERVER_402_STRING "402 USER NOT ALLOWED TO EXECUTE THIS COMMAND\n"
#define SERVER_402_STRING_SIZE sizeof(SERVER_402_STRING)

// QUIT LOGIN, LOGOUT, WHO, LOOK.
typedef enum supported_command
{
    ADD = 1,
    DELETE = 2,
    LIST = 3,
    SHUTDOWN = 4,
    QUIT = 5, // Implemented
    LOGIN = 6, // Implemented
    LOGOUT = 7, // Implemented
    WHO = 8, // Implemented
    LOOK = 9 // Implemented
}_supported_command;

typedef struct command_quit
{

}_command_quit;

typedef struct command_login
{
    unsigned char user_id[MAX_USER_ID];
    unsigned char password[MAX_PASSWORD_LENGTH];

}_command_login;

typedef struct command_logout
{

}_command_logout;

typedef struct command_who
{

}_command_who;

typedef struct command_look
{
    unsigned int search_type;
    unsigned char search_string[MAX_SEARCH_STRING];
}_command_look;

typedef struct command_payload
{
    unsigned char command_id[MAX_COMMAND_SIZE];
    unsigned char command_payload[MAX_COMMAND_PAYLOAD];
}_command_payload;

typedef struct response_payload
{
    unsigned char command_id[MAX_COMMAND_SIZE];
    bool is_sucessfully_executed;
    int number_of_response_in_flight;
    unsigned char response_payload[MAX_COMMAND_PAYLOAD];
}_response_payload;

typedef struct client_interface
{
    int client_socket;
    int client_index;
    char str[16];
}_client_interface;

void client_process_quit(int socket_fd);
void client_process_login(int socket_fd);
void client_process_logout(int socket_fd);
void client_process_who(int socket_fd);
void client_process_look(int socket_fd);

#endif //_COMMANDS_H_
