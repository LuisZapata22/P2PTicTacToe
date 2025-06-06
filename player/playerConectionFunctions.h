
int inicializarSocket(int *sockfd)
{
      
    *sockfd = socket(AF_INET,SOCK_STREAM,0);
    int n;
    if (*sockfd == -1)
    {
        perror("Error al crear el socket");
        n =  -1;
    }
    else
        n = 0;

    return n;
    
    
}

void conexionAddr(char *ip, int port, struct sockaddr_in *addr)
{
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (inet_pton(AF_INET, SERVER_IP, &(addr->sin_addr)) == -1)
    {
        perror("Error al configurar la dirección IP");
        exit(-1);  
    }


}

void conectarSocket(int *sockfd, struct sockaddr_in *addr)
{
    if (connect (*sockfd, (const struct sockaddr*)addr, sizeof(*addr)) < 0)
    {
            perror("Error al conectarse el socket");
    }
    else
    {
        fprintf(stdout, "CONECTADO!\n");
        fflush(stdout);
    }
}

void conectSocketMaster(int *sockfd)
{
    struct sockaddr_in servaddr;          
    inicializarSocket(sockfd);
    conexionAddr((char *)SERVER_IP, (int) SERVER_PORT, &servaddr);
    conectarSocket(sockfd, &servaddr);
}

int habilitarPuertoEscucha(int *sockfd, int *freePortServer)
{
    
    int n = 0;
    *freePortServer = 3023;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    do
    {
        addr.sin_port = htons(++(*freePortServer));
    } 
    while (bind(*sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1);
    if(listen(*sockfd, 2) == -1)
    {
        perror("LISTEN");
        n = -1;
    }
    
    return n;
}
