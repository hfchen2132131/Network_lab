#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    char filename[256];
    FILE *fp;
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket\n");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    //接收檔名
    read(sockfd, filename, 256);

    //建立檔案
    fp = fopen(filename,"wb");
    if(!fp) error("Cannot open the file\n");

    //printf("Please enter the message: ");
    bzero(buffer,256);
    //fgets(buffer,255,stdin);
    strcpy(buffer,"I got Filename");
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
            error("ERROR writing to socket");
    while(1){
        n = read(sockfd, buffer,256);
        if (n < 0) 
            error("ERROR recieving from server");
        if (n == 0) 
            break;
        printf("client got %d bytes\n", n);    
        n = fwrite(buffer, sizeof(char), n, fp);
		printf("fwrite %d bytes\n", n);
    }
    
    close(sockfd);
    return 0;
}
