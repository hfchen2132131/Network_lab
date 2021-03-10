/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];

    //讀取檔名
    
     char filename[256];
     FILE *fp;
    struct stat filestat;
     memset(filename,0,256);
     strcpy(filename,"Chapter 3.pptx");//argv[5]);

     //開啟檔案
    
    if ( lstat(filename, &filestat) < 0){
		exit(1);
	}
    fp = fopen(filename,"rb");
    if(!fp) error("Cannot open the file");
    
    printf("file size : %ld\n", filestat.st_size);

     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 5566;//atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");
     bzero(buffer,256);

    //傳送檔名
     n = write(newsockfd,filename,256);

     n = read(newsockfd,buffer,255);
     if (n < 0) error("Client Doesn't recieve filename");
     printf("Here is the message: %s\n",buffer);

    long int sendsize = 0;
    while(!feof(fp)){
        n = fread(buffer, sizeof(char), sizeof(buffer), fp);
        if (n < 0) error("ERROR reading from file");
        printf("read %d bytes from file\n", n);
        n = write(newsockfd,buffer,n);
        if (n < 0) error("ERROR writing to socket");
        printf("send %d bytes to client\n", n);
        sendsize += n;
        printf("%ld / %ld\n", sendsize, filestat.st_size);
    }
     
     close(newsockfd);
     close(sockfd);
     return 0;
}
