
int createSocket(int *n_socket)
{
    struct sockaddr_in servaddr;
    *n_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (*n_socket == -1)
    {
        perror("ERROR: El socket no pudo ser creado. \n");
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    //Asociar IP y puerto al socket
    if (bind(*n_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {    
        perror("ERROR: Error al asociar el socket al servidor.\n");
        return -1;
    }
    //Poner socket en modo escucha
    if (listen(*n_socket, MAXPLAYERS + 1) == -1)
    {
        perror("ERROR: Error al poner el socket en modo escucha.\n");
        return -1;
    }
    return 0;
}

void initPortComs(portCom *port[MAXPLAYERS])
{
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        port[i] = malloc(sizeof(portCom));
        port[i]->state = FREE;
        port[i]->p = NULL;
        memset(&port[i]->cli_addr, 0, sizeof(struct sockaddr));
        port[i]->clilen = sizeof(struct sockaddr);
    }

}

void processRequest(int i, portCom *ports[MAXPLAYERS], int n_jugadores)

{
    portCom *port = ports[i];
    char comand[5]; 
    strncpy(comand, port->buffer, 4); 
    comand[4] = '\0';
    char *msg;

    printf("[%d] RECIBIDO: %s\t|\tESTADO PREV: %c\t|\t", i, port->buffer, port->state);
    switch (port->state)
    {
            case FREE:
                //COnexion Inicial
                if(!strcmp(comand, "HOLA"))
                {
                    msg = "100";
                    port->state = NONREGISTER;
                }
                else
                    msg = "400";
            break;


            case NONREGISTER:
                //Registrar nuevo jugador
                if(!strcmp(comand, "REGI"))
                {
                    char *p_alias = strtok(port->buffer, DELIMITADOR);
                    p_alias = strtok(NULL, DELIMITADOR);
                    if(p_alias != NULL)
                    {
                        port->p = malloc(sizeof(player));
                        port->p->alias = malloc(strlen(p_alias) + 1);
                        strcpy(port->p->alias, p_alias);

                        port->p->id = id_count++;
                        port->p->points = 0;
                        port->p->wins = 0;
                        port->p->loses = 0;
                        port->p->draws = 0;
                        port->state = READY;
                        msg = "102";
                    }
                    else
                        msg = "402";
                }
                else
                    msg = "401";


                break;
            case READY:
                //INFO DEL JUGADOR
                if(!strcmp(comand, "INFO"))
                {
                    
                    msg = malloc(BUFFERSIZE);
                    sprintf(msg, "200|%d<%s<%d<%d<%d<%d", port->p->id, port->p->alias, port->p->points, port->p->wins, port->p->loses, port->p->draws);
                    port->state = WAITING;
                    
                }
                //MOSTRAR TABLA DE LOS JUGADORES
                else if(!strcmp(comand, "LIST"))
                {
                    
                    msg = malloc(BUFFERSIZE);
                    char *aux = malloc(60);
                    sprintf(msg, "201|");
                    for(int j = 0; j < MAXPLAYERS; j++)
                    {
                        if(ports[j]->state != FREE && ports[j]->state != NONREGISTER)
                        {
                            sprintf(aux, "%d<%s<%d<%d<%d<%d|", ports[j]->p->id, ports[j]->p->alias, ports[j]->p->points, ports[j]->p->wins, ports[j]->p->loses, ports[j]->p->draws);
                            strcat(msg, aux);
                        }
                        
                    }
                    free(aux);
                    port->state = WAITING;
                    
                }
                //SOLICITUD DE JUGADORES DISPONIBLES    
                else if(!strcmp(comand, "JUGA"))
                {
                    msg = malloc(BUFFERSIZE);
                    char *aux = malloc(60);
                    sprintf(msg, "202|");
                    for(int j = 0; j < MAXPLAYERS; j++)
                    {
                        if(ports[j]->state == READY && j != i)
                        {
                            sprintf(aux, "%d<%s|", ports[j]->p->id, ports[j]->p->alias);
                            strcat(msg, aux);
                        }
                        
                    }
                    free(aux);
                    port->state = WAITING;
                }
                //DESAFIAR A UN JUGADOR
                else if(!strcmp(comand, "DESA"))
                {
                    strtok(port->buffer, DELIMITADOR);
                    int id = atoi(strtok(NULL, DELIMITADOR));
                    int l_port = atoi(strtok(NULL, DELIMITADOR));

                    //Buscar el socket donde esta el id del jugador
                    int j;
                    bool correct = false;
                    for(j = 0; j < n_jugadores; j++)
                    {
                        if(ports[j]->state == READY && ports[j]->p->id == id && port->p->id != id)
                        {
                            correct = true;
                            break;
                        }
                    }
                   
                    if(correct)
                    {
                         //Preparar mensaje en el socket del destinatario
                        memset(ports[j]->buffer, 0, BUFFERSIZE);
                        char *clientAddress = malloc(sizeof(16));
                        inet_ntop(AF_INET, &((struct sockaddr_in*)&(ports[j]->cli_addr))->sin_addr, clientAddress, ports[j]->clilen);
                        sprintf(ports[j]->buffer, "203|%d|%s|%s|%d", port->p->id, port->p->alias, clientAddress,l_port);
                        msg = "104";
                        fds[j+1].events |= POLLOUT;
                        ports[j]->state=PLAYING;
                        port->state = PLAYING;
                    }
                    else
                        msg = "409";
                    
                }
                
                else
                    msg = "403";    



            break;
            case PLAYING:
                //SE RECHAZO EL DESAFIO
                if(!strcmp(comand, "PLNO"))
                {
                    port->state = READY;
                    msg = "105";
                }
                //SE ACEPTO EL DESAFIO
                else if(!strcmp(comand, "PLYE"))
                {
                    port->state = PLAYING;
                    msg = "104";
                }
                //REESPUESTA DEL JUGADOR AL DESAFIO
                else if(!strcmp(comand, "RESP"))
                {
                    
                    strtok(port->buffer, DELIMITADOR);
                    int id = atoi(strtok(NULL, DELIMITADOR));
                    char ans = (strtok(NULL, DELIMITADOR))[0];

                    int j;
                    for(j = 0; j < MAXPLAYERS; j++)
                    {
                        if(ports[j]->p->id == id)
                            break;
                    }

                    //Preparar mesaje en el puerto del destinatario
                    memset(ports[j]->buffer, 0, BUFFERSIZE);
                    sprintf(ports[j]->buffer, "204|%c", ans);
                  
                    //POner modo envio
                    fds[j+1].events |= POLLOUT;

                    switch (ans)
                    {
                        case 'Y':
                            port->state = PLAYING;
                            msg = "104";
                            break;
                        case 'N':
                            port->state= READY;
                            msg = "105";
                            break;
                        
                        default:
                            msg = "405";
                            break;
                    }
                    
                    
                }
                
                //JUEGO FINALIZÓ
                else if(!strcmp(comand, "FINN"))
                {
                    char ans = port->buffer[5];
                    port->state = READY;
                    msg = "103";
                    switch (ans)
                    {
                        case 'W':
                            (port->p->points) += 2;
                            (port->p->wins) += 1;
                            break;
                        case 'L':
                            (port->p->loses) += 1;
                            break;
                        case 'D':
                            (port->p->points) += 1;
                            (port->p->draws) += 1;
                            break;
                        
                        default:
                            port->state = PLAYING;
                            msg = "407";
                            break;
                    }
                }
                else
                    msg = "406";
            break;

            //ESPERAR CONFIRMACIÓN
            case WAITING:
                if(!strcmp(comand, "OKAY"))
                {
                    msg = "101";
                    port->state = READY;
                }
                else
                    msg = "404";
            break;
        
        default:
            msg = "420";
            break;
    }

        
        printf("ESTADO ACT: %c\t|\n", port->state);
        memset(port->buffer, 0, BUFFERSIZE);
        sprintf(port->buffer, "%s", msg);
        printf("[%d] ENVIADO: %s\n", i, port->buffer);
    

    
}