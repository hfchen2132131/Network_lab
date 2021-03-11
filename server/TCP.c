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
#include <netdb.h> 
#include <errno.h>

#define SEND_MODE 0
#define RECV_MODE 1
#define TCP_PROT 0
#define UDP_PROT 1
#define BUFF_SIZE 2560000


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int mode;
    int prot;
    char ip[15];

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char *buffer;//[25600000];
    buffer = malloc(BUFF_SIZE);
    struct sockaddr_in serv_addr, cli_addr;
    struct hostent *server;
    int n;

    
    
    char filename[256];
    FILE *fp;
    struct stat filestat;
    
     
    
    if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
    }

    if(!strcmp("tcp", argv[1])) prot = TCP_PROT;
    else if(!strcmp("udp", argv[1])) prot = UDP_PROT;
    else error("WRONG PROTOCOL");

    if(!strcmp("send", argv[2])) mode = SEND_MODE;
    else if(!strcmp("recv", argv[2])) mode = RECV_MODE;
    else error("WRONG MODE");    

    if(prot == TCP_PROT){
        if(mode == SEND_MODE){
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
                error("ERROR opening socket");
            bzero((char *) &serv_addr, sizeof(serv_addr));
            portno = atoi(argv[4]);
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
            bzero(buffer,BUFF_SIZE);

            //讀取檔名
            memset(filename,0,256);
            strcpy(filename,argv[5]);
            
            //開啟檔案
            if(lstat(filename, &filestat) < 0){
                error("file wrong");
            }
            fp = fopen(filename,"rb");
            if(!fp) error("Cannot open the file");
            
            printf("file size : %ld\n", filestat.st_size);
            //傳送檔名
            printf("%s\n", filename);
            n = write(newsockfd,filename,256);
            

            n = read(newsockfd,buffer,BUFF_SIZE);
            if (n < 0) error("Client Doesn't recieve filename");
            printf("Here is the message: %s\n",buffer);

            long int sendsize = 0;
            int stage = 0;
            while(!feof(fp)){
                n = fread(buffer, sizeof(char), BUFF_SIZE, fp);
                if (n < 0) error("ERROR reading from file");
                //printf("read %d bytes from file\n", n);
                n = write(newsockfd,buffer,n);
                if (n < 0) error("ERROR writing to socket");
                //printf("send %d bytes to client\n", n);
                sendsize += n;
                //printf("%ld / %ld\n", sendsize, filestat.st_size);
                if((double)sendsize / (double)filestat.st_size > 0.25 && stage == 0){
                    printf("25%%\n");
                    ++stage;
                }
                else if((double)sendsize / (double)filestat.st_size > 0.5 && stage == 1){
                    printf("50%%\n");
                    ++stage;
                }
                else if((double)sendsize / (double)filestat.st_size > 0.75 && stage == 2){
                    printf("75%%\n");
                    ++stage;
                }
                
            }
            
            printf("100%%\n");
            
            close(newsockfd);
        }
        else if(mode == RECV_MODE)
        {
            portno = atoi(argv[4]);
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) 
                error("ERROR opening socket\n");
            server = gethostbyname(argv[3]);
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
            char * pch, tp[10];
            pch = (char*) memchr(filename, '.', 256);
            printf("%s\n",pch);
            strcpy(tp, pch);
            strcpy(pch,"_recv");
            strcat(filename, tp);
            printf("%s\n",filename);
            //建立檔案
            fp = fopen(filename,"wb");
            if(!fp) error("Cannot open the file\n");

            //printf("Please enter the message: ");
            bzero(buffer,BUFF_SIZE);
            //fgets(buffer,255,stdin);
            strcpy(buffer,"I got Filename");
            n = write(sockfd,buffer,strlen(buffer));
            if (n < 0) 
                    error("ERROR writing to socket");
            while(1){
                n = read(sockfd, buffer,BUFF_SIZE);
                if (n < 0) 
                    error("ERROR recieving from server");
                if (n == 0) 
                    break;
                printf("client got %d bytes\n", n);    
                n = fwrite(buffer, sizeof(char), n, fp);
                printf("fwrite %d bytes\n", n);
            }
        }
    }
    else if(prot == UDP_PROT){
        if(mode == SEND_MODE){
            sockfd = socket(PF_INET, SOCK_DGRAM, 0);
            if (sockfd < 0)
                error("ERROR opening socket");
            bzero((char *) &serv_addr, sizeof(serv_addr));
            portno = atoi(argv[4]);
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            serv_addr.sin_port = htons(portno);
            if (bind(sockfd, (struct sockaddr *) &serv_addr,
                    sizeof(serv_addr)) < 0)
                    error("ERROR on binding");

            bzero(buffer,BUFF_SIZE);

            //讀取檔名
            memset(filename,0,256);
            strcpy(filename,argv[5]);
            
            //開啟檔案
            if(lstat(filename, &filestat) < 0){
                error("file wrong");
            }
            fp = fopen(filename,"rb");
            if(!fp) error("Cannot open the file");
            
            printf("file size : %ld\n", filestat.st_size);

            //等待request
            char feedback[256];
            clilen = sizeof(cli_addr);
            while (1)
            {
                n = recvfrom(sockfd, feedback, 256, 0,
                            (struct sockaddr *)&cli_addr, &clilen);
                if(n == -1){
                    if (errno == EINTR)
                        continue;
                    error("reciving fail");
                }

                if(n >-1){
                    if(!strcmp(feedback, "Request")){
                        break;
                    }
                    else
                    {
                        error("wrong request");
                    }
                }
            }           

            //傳送檔名
            printf("%s\n", filename);
            n = sendto(sockfd, filename, 256, 0,
                        (struct sockaddr *)&cli_addr, clilen);
            if(n == -1) error("error sending filename");
            //確認收到
            while (1)
            {
                n = recvfrom(sockfd, feedback, 256, 0,
                            (struct sockaddr *)&cli_addr, &clilen);
                if(n == -1){
                    if (errno == EINTR)
                        continue;
                    error("reciving fail");
                }
                
                if(n >-1){
                    if(!strcmp(feedback, "Got filename")){
                        break;
                    }
                    else
                    {
                        error("client don't get filename");
                    }                    
                }
            }
            
            //傳送檔案
            while (!feof(fp))
            {
                
                n = fread(buffer, sizeof(char), BUFF_SIZE, fp);
                if (n < 0) error("ERROR reading from file");
                //printf("read %d bytes from file\n", n);
                n = sendto(sockfd, buffer, n, 0,
                        (struct sockaddr *)&cli_addr, clilen);
                if(n == -1) error("error sending file");
                
                n = recvfrom(sockfd, buffer, BUFF_SIZE, 0,
                            (struct sockaddr *)&cli_addr, &clilen);
                if (n == -1)
                {
                    if (errno == EINTR)
                        continue;

                    error("recvfrom error");
                }
                else if(n > 0)
                {
                    
                }
            }
        }
        else if(mode == RECV_MODE){

        }
    }
    
    
    
     close(sockfd);
     return 0;
}

