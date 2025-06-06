void printBoard(char board[size_b][size_b]) 
{
    for (int i = 0; i < size_b; i++) {
        for (int j = 0; j < size_b - 1; j++) {
            printf(" %c |", board[i][j]);
        }
        printf(" %c \n", board[i][size_b - 1]);
        if (i < size_b - 1)
        {
            printf("---|---|---\n");
        }
    }
}

void initBoard(char board[size_b][size_b]) {
    for (int i = 0; i < size_b; i++) {
        for (int j = 0; j < size_b; j++) {
            board[i][j] = ' ';
        }
    }
}

char checkWinner(char board[size_b][size_b])
{
    char winner = ' ';
    bool check_fila;
    bool check_columna;

    //Filas y columnas
    for(int i = 0; i < size_b; i++)
    {

        check_fila = true;
        check_columna = true;
        for(int j = 0; j < size_b; j++)
        {

            check_fila = check_fila && (board[i][i] == board[i][j]) && board[i][i] != ' ';
            check_columna = check_columna && (board[i][i] == board[j][i]) && board[i][i] != ' ';

        }
        if(check_fila || check_columna)
        {
            winner = board[i][i];
            break;
        }

    }

    if(board[0][0] == board[1][1] && board[0][0] == board[2][2] && (board[0][0] != ' '))
        winner = board[0][0];
    else if(board[0][2] == board[1][1] && board[0][2] == board[2][0]&& (board[1][1] != ' '))
        winner = board[0][0];



    //Diagonales
    return winner;
}

bool checkMovement(char *mov, char board[size_b][size_b], char simbolo, int *fila, int *columna)
{
    *fila = mov[0] -'A';
    *columna = mov[1] -'1';
    //printf("FILA %d\nCOLUMNA: %d\n", fila, columna);

    if(*fila >= 0 && *columna >= 0 && *fila <= size_b && *columna <= size_b && board[*fila][*columna] == ' ')
        return true;
    else
        return false;
    
}

void makeMov(char simbolo, char board [size_b][size_b], int fila, int columna)
{
     board[fila][columna] = simbolo;
}

void printExampleBoard()
{
    char fil = 'A';
    //int columna;
    for (int i = 0; i < size_b; i++) 
    {
        for (int j = 0; j < size_b - 1; j++) 
        {
            printf(" %c%d |", fil, j+1);
        }
        printf(" %c%d \n",fil, 3);
        if (i < size_b - 1)
        {
            printf("----|----|----\n");
        }
        fil++;
    }
}

void printMenu()
{
    fprintf(stdout, "\n--------- MENU ---------\n");
    fprintf(stdout, "1. Información del jugador\n");
    fprintf(stdout, "2. Mostrar tabla de juego\n");
    fprintf(stdout, "3. Jugar\n");
    fprintf(stdout, "4. Salir\n\n");
    fprintf(stdout, "OPCION: ");
    fflush(stdout); 
}

void printTableHead()
{
    
    printf("\n\n|ID\t|NOMBRE\t|PUNTOS\t|V\t|D\t|E\t|\n");
    printf("--------------------------------------------------\n");
}

void printPlayer(char *jug)
{
    //printf("%s", jug);
    printf("|");
    for(int i = 4; i < strlen(jug); i++)
    {
        
        if (jug[i] == '|') 
        {
            printf("\t|\n|");
            
        } 
        else if (jug[i] == '<') 
        {
            printf("\t|");
            
        } 
        else 
        {
            printf("%c", jug[i]);
        }
    }
    printf("\n\n\n");
}
