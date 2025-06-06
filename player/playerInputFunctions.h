void processPlayer(char board[size_b][size_b], char *buffers[3], struct pollfd fds[4], player *pl, bool *send_desa, oponent *op)
{
    char comand[5]; 
    strncpy(comand, buffers[2], 4); 
    comand[4] = '\0';
    //char *msg;

    if(!strcmp(comand, "HOLA"))
    {
        initBoard(board);

        
        printf("JUEGO:\n\n%c:\t%s\n%c:\t%s\n", pl->simbolo, "YO", ((pl->simbolo==X)?O:X), "oponent");
        printf("\nTURNO: %c\n\n", O);       
        printExampleBoard(board);
        fprintf(stdout, "\n");
        fflush(stdout);

        if(pl->turno)
        {
            memset(buffers[2], 0, BUFFERSIZE);
            sprintf(buffers[2], "HOLA");
            fds[3].revents |= POLLOUT;
            fprintf(stdout,"\n--- TU TURNO ---\n");
            fprintf(stdout,"MOVIMIENTO: ");
            fflush(stdout);
            
        }
        else
        {   
            fprintf(stdout,"### ESPERANDO MOVIMIENTO oponent ###\n\n");
            fflush(stdout);
        }
    }
    else  if(!strcmp(comand, "POSX"))
    {
        fprintf(stdout,"### MOVIMIENTO REALIZADO ###\n\n");
        fflush(stdout);
        pl->turno = true;
        pl->movs++;
        char aux[3];
        sprintf(aux,"%c%c",buffers[2][5], buffers[2][6]);
        int fila;
        int columna;
        char op_simbol = ((pl->simbolo==X)?O:X);
        checkMovement(aux, board, op_simbol, &fila, &columna);
        makeMov(op_simbol, board, fila, columna);
        printBoard(board);
        char winner = checkWinner(board);
        if(winner == ' ')
        {
            if(pl->movs >= 9)
            {
                sprintf(buffers[2], "WINN");
                fds[3].revents |= POLLOUT;
                
            }
            else
            {
                printf("\nTURNO :%c\n\n", pl->simbolo);
                fprintf(stdout,"--- TU TURNO ---\n");
                fprintf(stdout,"MOVIMIENTO: ");
                fflush(stdout); 
            }
                
        }
        else
        {
            sprintf(buffers[2], "WINN");
            fds[3].revents |= POLLOUT;
        }
       

    }
    else  if(!strcmp(comand, "WINN"))
    {
        //sprintf(buffers[2], "WINN");
        char winner = checkWinner(board);
        if(winner == ' ')
        {
            sprintf(buffers[1], "FINN<D");
            fprintf(stdout, "\n#####\tEMPATE\t#####\n");
            fflush(stdout);
        }
        else
        {
            if(winner == pl->simbolo)
            {
                sprintf(buffers[1], "FINN<W");
                printf("\n#####\tGANADOR: %c\t#####\n", pl->simbolo);
                printf("\n#####\tGANASTE :)\t#####\n");
            }
            else
            {
                sprintf(buffers[1], "FINN<L");
                printf("\n#####\tGANADOR: %c\t#####\n", ((pl->simbolo==X)?O:X));
                printf("\n#####\tPERDISTE :(\t#####\n");
            }
        }
        fds[1].revents |= POLLOUT;
        
        
        close(op->play_socket);
        fds[3].fd = -1;
        fds[3].events = 0;
        if(*send_desa)
        {
            close(op->listen_socket);
            fds[2].fd = -1;
            fds[2].events = 0;
        }
        *send_desa = false;
    }
    else
    {
        printf("ERROR: Comando jugador desconocido\n");
    }

}

void processKeyboard(char* buffers[3], char *estado, int *freePortServer, bool *send_desa, oponent *op, player *pl, char board[size_b][size_b])
{
    
    switch (*estado)
    {
        case READY:
            *estado = WAITING;
            fds[1].events |= POLLOUT;
            memset(buffers[1], 0, BUFFERSIZE);
            
            switch (atoi(buffers[0])) 
            {
                case 1:
                    sprintf(buffers[1], "INFO");
                    break;
                case 2:
                    sprintf(buffers[1], "LIST");
                    break;
                case 3:
                    sprintf(buffers[1], "JUGA");
                    *estado = CHALLENGE;
                    *send_desa = true;
                    break;
                //Salir
                case 4:
                    exit(0);
                    break; 
                default:
                    *estado = READY;
                    fds[1].events &= ~POLLOUT;
                    printf("INVALIDO\n");
            }

        break;
        
        case CHALLENGE:
            //SELECCIONAR QUE JUGADOR A DESAFIAR
            if(*send_desa)
            {
                memset(buffers[1], 0, BUFFERSIZE);
               
                sprintf(buffers[1], "DESA<%d<%d", (atoi(buffers[0])), *freePortServer);
                fds[1].events |= POLLOUT;
                fprintf(stdout, "\n--- ESPERANDO RESPUESTA ---\n");
                fflush(stdout);
                
            }
            //RESPONDER DESAFIO
            else
            {
                memset(buffers[1], 0, BUFFERSIZE);
                sprintf(buffers[1], "RESP<%d<%c", (op->id), toupper(buffers[0][0]));
                fds[1].events |= POLLOUT;
            }
            
        break;

        case PLAYING:
            if(pl->turno)
            {
                int fila;
                int columna;
                if(!checkMovement(buffers[0], board, pl->simbolo, &fila, &columna))
                {
                    fprintf(stdout, "### MOVIMIENTO INVALIDO ###\nMOVIMIENTO: ");
                    fflush(stdout);
                }
                else
                {
                    makeMov(pl->simbolo, board, fila, columna);
                    memset(buffers[2], 0, BUFFERSIZE);
                    sprintf(buffers[2], "POSX|%s", buffers[0]);
                    fds[3].events |= POLLOUT;
                    pl->turno = false;
                    pl->movs++;
                    printBoard(board);
                    char winner = checkWinner(board);
                    if(winner == ' ')
                    {
                        fprintf(stdout,"\n### ESPERANDO MOVIMIENTO oponent ###\n\n");
                        fflush(stdout);
                    }
                }
                
                
            }
            else
            {
                fprintf(stdout, "ESPERA TU TURNO\n");
                fflush(stdout);
            }
        break;

        default:
            break;
    }
}

void processMaster(char *buffers[3], char *estado, bool *send_desa,  int *freePortServer, oponent *op, player *pl)
{
    char comand[4]; 
    strncpy(comand, buffers[1], 3);
    int cod = atoi(comand); 
    switch (*estado)
    {
        
        case READY:
            switch (cod)
            {
            case 101:
                printMenu();
            break;
            
            //Solicitud de juego de otro jugador
            case 203:
                *estado = CHALLENGE;
                strtok(buffers[1], "|");
                op->id = atoi(strtok(NULL, "|"));
                op->name = malloc(50);
                strcpy(op->name, strtok(NULL, "|")); 
                char jug_ip[20];
                strcpy(jug_ip, strtok(NULL, "|"));
                int jug_port = atoi(strtok(NULL, "|"));


                printf("\n\n------------------------\n");
                printf("ID: %d\n", op->id);
                printf("NOMBRE: %s\n", op->name);
                printf("PUERTO: %d\n", jug_port);
                printf("IP: %s\n", jug_ip);
                printf("-----------------------\n");

                conexionAddr(jug_ip, jug_port, &(op->addr));
                inicializarSocket(&(op->play_socket));
                printf("\n\n### JUGADOR: **%s** TE HA DESAFIADO ###\n", op->name);
                fprintf(stdout, "ACEPTAS EL RETO? (Y/N):");
                fflush(stdout);
            break;

            default:
                break;
            }
        break;
            
        case WAITING:
            

            switch (cod)
            {
                //INFO JUGADOR
                case 200:
                    printTableHead();
                    printPlayer(buffers[1]);
                    memset(buffers[1], 0, BUFFERSIZE);
                    sprintf(buffers[1], "OKAY");
                    fds[1].events |= POLLOUT;
                    *estado = READY;

                    
                break;

                //TABLA POSICIONES
                case 201:
                    printTableHead();
                    printPlayer(buffers[1]);
                    memset(buffers[1], 0, BUFFERSIZE);
                    sprintf(buffers[1], "OKAY");
                    fds[1].events |= POLLOUT;
                    *estado = READY;

                    
                break;

                
                
                default:
                printf("ERROR: Mensaje master erroneo\n");
                    break;
            }



        break;

        case CHALLENGE:

            switch (cod)
            {
                //Confirmación de jugadores disponibles
                case 101:
                    inicializarSocket(&op->listen_socket);
                    habilitarPuertoEscucha(&(op->listen_socket), freePortServer);
                    fds[2].fd = op->listen_socket;
                    fds[2].events = POLLIN;
                    fprintf(stdout, "INGRESE EL ID DEL JUGADOR: ");
                    fflush(stdout); 

                break;

                //Imprimir lista de jugadores disponibles para jugar   
                case 202:
                        printf("###  JUGADORES DISPONIBLES  ###\n");
                        printf("|ID\t|NOMBRE\t|\n");
                        printf("-------------------\n");
                        printPlayer(buffers[1]);
                        printf("\n\n");
                        memset(buffers[1], 0, BUFFERSIZE);
                        sprintf(buffers[1], "OKAY");
                        fds[1].events |= POLLOUT;
                        
                        
                break;

                //Respuesta del RETO
                case 204:
                   
                    if(buffers[1][4] == 'Y')
                    {
                        *estado = PLAYING;
                        fprintf(stdout, "RETO ACEPTADO!!!\n");
                        fflush(stdout);
                        memset(buffers[1], 0, BUFFERSIZE);
                        sprintf(buffers[1], "PLYE");
                      
                        pl->simbolo = O;
                        pl->movs = 0;
                        pl->turno = true;

                        fds[1].events |= POLLOUT;

                    }
                    else
                    {
                        *estado = READY;
                        fprintf(stdout, "RETO RECHAZADO!!!\n");
                        fflush(stdout);
                        close(op->listen_socket);
                        fds[2].fd = -1;
                        fds[2].events = 0;
                        memset(buffers[1], 0, BUFFERSIZE);
                        sprintf(buffers[1], "PLNO");
                        fds[1].events |= POLLOUT;
                        printMenu();
                        
                    }

                break;
                //
                case 104:
                    
                    if(!(*send_desa))
                    {
                        fprintf(stdout, "INICIA EL JUEGO!!!\n");
                        fflush(stdout);
                        
                        conectarSocket(&(op->play_socket), &(op->addr));
                        //sleep(1);
                        fds[3].fd = op->play_socket;
                        fds[3].events = POLLIN;
                        sprintf(buffers[2], "HOLA");
                        fds[3].events |= POLLOUT;
                        
                        *estado = PLAYING;
                        pl->simbolo = X;
                        pl->movs  = 0;
                        pl->turno = false;
                        
                    }
                        
                    
                break;

                case 105:
                    if(!(*send_desa))
                    {
                        fprintf(stdout, "JUEGO RECHAZADO!!!\n");
                        fflush(stdout);
                        *estado = READY;
                        printMenu();
                    }
                        
                    
                break;

                case 409:
                    *estado = READY;
                    fprintf(stdout, "ID ERRONEO!!!\n");
                    fflush(stdout);
                    printMenu();
                break;

                default:
                    break;
            }
            
        
        break;

        case PLAYING:
            switch (cod)
            {
            case 103:
                *estado = READY;
                printMenu();
            break;
            case 104:
            break;
            default:
                fprintf(stdout, "ERROR: Mensaje no esperado de Master en modo Jugar!\n");
                fflush(stdout);
                break;
            }
        break;

        default:
            printf("ERROR: MENSAJE RECIBIDO EN ESTADO NO ESPERADO\n");
        break;
    }
}

