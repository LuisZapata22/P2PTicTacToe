#include <stdio.h>      
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdbool.h> 
#include <errno.h>
#include <poll.h>
#include <ctype.h>


char* SERVER_IP = "127.0.0.1";
int SERVER_PORT = 2024;

#define BUFFERSIZE 1024


//ESTADOS
#define FREE 'F'
#define READY 'R'
#define WAITING 'W'
#define CHALLENGE 'A'
#define PLAYING 'P'

#define O 'O'
#define X 'X'

#define size_b 3


typedef struct oponent
{
    struct sockaddr_in addr;
    int id;
    int listen_socket;
    char *name;
    int play_socket;
    

}oponent;

typedef struct player
{
    char *name;
    char simbolo;
    int movs;
    bool turno;
}player;

struct pollfd fds[4];