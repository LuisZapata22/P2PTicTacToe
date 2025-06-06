
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


#define MAXPLAYERS 10
#define PORT 2024
#define BUFFERSIZE 1024

typedef struct player
{
    int id;
    char *alias;
    int points;
    int wins;
    int loses;
    int draws;
}
player;

typedef struct portCom
{
    char state;
    char buffer[BUFFERSIZE];
    player *p;
    struct sockaddr cli_addr;
    socklen_t clilen; 

}
portCom;

//  ESTADOS
#define FREE 'F'
#define NONREGISTER 'N'
#define READY 'R'
#define PLAYING 'P'
#define WAITING 'S'
#define ERROR 'E'


#define DELIMITADOR "<"

//ID DE JUGADORES
int id_count = 1;
struct pollfd fds[MAXPLAYERS + 1];

// void procesarSolicitud(int i, portCom *ports[MAXJUGADORES], int n_jugadores);
// int crearSocket(int *n_socket);
// void inicializarPortComs(portCom *port[MAXJUGADORES]);