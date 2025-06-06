#include "masterStructures.h"
#include "masterFunctions.h"


int main() 
{
   
    int l_socket;
    if(createSocket(&l_socket) != 0)
        exit(-1);
    portCom *ports[MAXPLAYERS];
    initPortComs(ports);
    
    //POLL

    //Sockets poll
    
    fds[0].fd = l_socket;
    fds[0].events = POLLIN; 
    int nfds = 1; 

    
    int auxsockfd;
    int respoll;

    
    while (true) 
    {
        respoll = poll(fds, nfds, -1);
        if (respoll < 0)
            perror("ERROR: Poll\n");

        // New client tries to connect
        if (fds[0].revents & POLLIN && nfds <= MAXPLAYERS) 
        {
            ports[nfds-1]->clilen = sizeof(ports[nfds-1]->cli_addr);
            auxsockfd = accept(l_socket, &ports[nfds-1]->cli_addr, &ports[nfds-1]->clilen);
            if (auxsockfd < 0) 
            {
                perror("ERROR: No se puede aceptar al cliente\n");
                continue;    
            }
            printf("[%d]\tNueva conexion\n", nfds - 1);
            
            // Add new client to array
            fds[nfds].fd = auxsockfd;
            fds[nfds].events = POLLIN;
            nfds++;
        }

        // Handle existing clients
        for (int i = 1; i < nfds; i++) 
        {
            // Received data from client
            if (fds[i].revents & POLLIN) 
            {

                memset(ports[i-1]->buffer, 0, BUFFERSIZE);
                int valread = recv(fds[i].fd, ports[i-1]->buffer, BUFFERSIZE, 0);

                if (valread < 0)
                {
                    perror("ERROR: No se pudo leer el mensaje ");
                    close(fds[i].fd);
                    fds[i].fd = 0;
                    ports[i]->p = NULL;
                    ports[i]->state =FREE;
                }
                else if (valread == 0) 
                {
                    // Disconnected Client
                    close(fds[i].fd);
                    printf("[%d]\tCliente desconectado\n", i-1);
                    fds[i].fd = 0;
                    ports[i]->p = NULL;
                    ports[i]->state =FREE;

                } 
                else 
                {
                    //Process Message
                    processRequest(i-1, ports, nfds);
            
                    // Enable output for client
                    fds[i].events |= POLLOUT; 
                }
            }

            // Send data to client
            if (fds[i].revents & POLLOUT) 
            {
                
                int bytes_sent = send(fds[i].fd, ports[i-1]->buffer, BUFFERSIZE, 0);



                if (bytes_sent < 0) 
                {
                    perror("ERROR: No se pudo enviar la respuesta ");
                    
                    close(fds[i].fd);
                    fds[i].fd = 0;
                    ports[i]->p = NULL;
                    ports[i]->state =FREE;
                } 
                else 
                {
                    // Disable POLLOUT event after sending response
                    fds[i].events &= ~POLLOUT;
                }
            }
        }
    }
    close(l_socket);

    return 0;
}
