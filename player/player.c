#include "playerStructures.h"
#include "playerGameFunctions.h"
#include "playerConectionFunctions.h"
#include "playerInputFunctions.h"




int main(int argc, char* argv [])
{
    if(argc > 2) {
        SERVER_IP = argv[1];
        SERVER_PORT = atoi(argv[2]);
    }

    char state = FREE;
    int master_socket;

    oponent op;
    player me;

    me.name = malloc(50);
    conectSocketMaster(&master_socket);
    
    bool sendChallenge = false;
    int freePortServer;
    
    char *buffers[3];
    buffers[0] = malloc(5);
    buffers[1] = malloc(BUFFERSIZE);
    buffers[2] = malloc(BUFFERSIZE);
    

    //INICIAR CONEXION MASTER
    send(master_socket, "HOLA", 5, 0);
    recv(master_socket, buffers[1], BUFFERSIZE, 0);
    if(strcmp("100", buffers[1]) != 0)
        exit(-1);

    printf("Ingrese su nombre: ");
    scanf("%s", me.name);
    memset(buffers[1],0,BUFFERSIZE);
    sprintf(buffers[1], "REGI<%s", me.name);
    send(master_socket, buffers[1], BUFFERSIZE, 0);
    memset(buffers[1],0,BUFFERSIZE);
    recv(master_socket, buffers[1], BUFFERSIZE, 0);
    if(strcmp("102", buffers[1]) != 0)
        exit(-1);
    state = READY;

    //poll
    // 0 -> teclado
    // 1 -> master
    // 2 -> play listen
    // 3 -> play connect
    
    fds[0].fd = STDIN_FILENO;
    fds[1].fd = master_socket;


    fds[0].events = POLLIN;
    fds[1].events = POLLIN;
    
 
    int respoll;
    printMenu();

    //TABLERO
    char board [size_b][size_b];
    
    
    while(true)
    {
        respoll = poll(fds, 4, -1);
        if (respoll < 0)
        {
            perror("ERROR: Poll\n");
            break;
        }

        
        for(int i = 0; i < 4; i++)
        {
            
            if(fds[i].revents & POLLIN)
            {
                switch (i)
                {
                    //teclado
                    case 0:
                        if(read(fds[0].fd, buffers[0], 4) < 0)
                        {
                            perror("ERROR: NO SE PUDO LEER EL TECLADO");
                            break;
                        }
                        
                        processKeyboard(buffers, &state, &freePortServer, &sendChallenge, &op, &me, board);
                    break;

                    //Master
                    case 1:
                        if(recv(fds[1].fd, buffers[1], BUFFERSIZE, 0) < 0)
                        {
                            perror("ERROR: NO SE PUDO OBTENER MENSAJE DEL MASTER");
                            break;
                        }
                        processMaster(buffers,&state, &sendChallenge, &freePortServer, &op, &me);
                    break;
                    
                    //Listen Play
                    case 2:
                        op.play_socket = accept(op.listen_socket, NULL, NULL);
                        if (op.play_socket < 0) 
                        {
                            perror("ERROR: NO SE PUDO ACEPTAR AL CLIENTE\n");
                            //exit(-2);    
                        }
                        fds[3].fd = op.play_socket;
                        fds[3].events = POLLIN;

                        printf("JUGADOR CONECTADO\n");
                        

                    break;                    
                    
                    //Juego
                    case 3:
                        //printf("MENSAJE JUGADOR: %s\n", buffers[2]);
                        if(read(op.play_socket, buffers[2], BUFFERSIZE) < 0)
                        {
                            perror("ERROR: NO SE PUDO LEER EL MESAJE DEL JUGADOR");
                            break;
                        }
                        processPlayer(board, buffers, fds, &me, &sendChallenge, &op);
                    break;      

                    default:
                        break;
                }
                
            }
        }
        

       //ENVIAR INFO
        for(int i = 1; i <= 3; i++)
        {
            if(fds[i].revents & POLLOUT)
            {
                int j;
                if(i == 3)
                    j = 2;
                else
                    j = i;

                int bytes_sent = send(fds[i].fd, buffers[j], BUFFERSIZE, 0);
                if (bytes_sent < 0) 
                {
                    perror("ERROR: No se pudo enviar la respuesta ");
                    
                    close(fds[i].fd);
                    break;
                } 
                else 
                {
                    
                    fds[i].events &= ~POLLOUT;
                   
                }
            }
        }
        

        
        
    }
    close(master_socket);
    
    
}

