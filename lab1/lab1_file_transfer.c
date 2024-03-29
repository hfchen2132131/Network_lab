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
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define SEND_MODE 0
#define RECV_MODE 1
#define TCP_PROT 0
#define UDP_PROT 1
#define BUFF_SIZE 65500


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int mode; //TCP or UDP
    int prot; //SEND or RECV

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char *buffer;
    buffer = (char*)malloc(BUFF_SIZE);
    struct sockaddr_in serv_addr, cli_addr;
    struct hostent *server;
    int n;
    time_t rawtime;//for time stamp
    struct timeval start;//for trans time
    struct timeval end;

    
    
    char filename[256];
    FILE *fp;
    struct stat filestat;//for file size
    
     
    
    if (argc < 5) {
         fprintf(stderr,"ERROR, too few argument\n");
         exit(1);
    }

    if(!strcmp("tcp", argv[1])) prot = TCP_PROT;
    else if(!strcmp("udp", argv[1])) prot = UDP_PROT;
    else error("WRONG PROTOCOL");

    if(!strcmp("send", argv[2])) mode = SEND_MODE;
    else if(!strcmp("recv", argv[2])) mode = RECV_MODE;
    else error("WRONG MODE");   

    if(mode == SEND_MODE){
        if (argc < 6) {
            fprintf(stderr,"ERROR, too few argument\n");
            exit(1);
        } 
    }    

    if(prot == TCP_PROT){
        if(mode == SEND_MODE){
            //TCP and SEND

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
            //獲取檔案大小
            if(filestat.st_size > 1048576)
                printf("file size : %lf MB\n", (double)filestat.st_size / 1000000.0);
            else if(filestat.st_size > 1024)
                printf("file size : %lf KB\n", (double)filestat.st_size / 1000.0);
            else
                printf("file size : %ld B\n", filestat.st_size);
            
            //傳送檔名
            printf("%s\n", filename);
            n = write(newsockfd,filename,256);
            if (n < 0) 
                    error("ERROR writing to socket");
            //傳送檔案大小
            n = write(newsockfd,&filestat,sizeof(filestat));
            if (n < 0) 
                    error("ERROR writing to socket");
            
            //確認client收到
            n = read(newsockfd,buffer,BUFF_SIZE);
            if (n < 0) error("Client Doesn't recieve filename");
            printf("Here is the message: %s\n",buffer);

            //傳送檔案
            long int sendsize = 0;//已傳送的大小
            int stage = 0;//分辨25% 50% 75%
            char curtime[80];//for time stamp
            gettimeofday(&start, NULL);//開始計時
            while(!feof(fp)){
                //讀檔
                n = fread(buffer, sizeof(char), BUFF_SIZE, fp);
                if (n < 0) error("ERROR reading from file");
                //傳送
                n = write(newsockfd,buffer,n);
                if (n < 0) error("ERROR writing to socket");

                sendsize += n;//累積傳送大小

                //顯示已經傳了25% 50% 75%
                if((double)sendsize / (double)filestat.st_size > 0.25 && stage == 0){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("25%% %s", curtime);
                    ++stage;
                }
                if((double)sendsize / (double)filestat.st_size > 0.5 && stage == 1){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("50%% %s", curtime);
                    ++stage;
                }
                if((double)sendsize / (double)filestat.st_size > 0.75 && stage == 2){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("75%% %s", curtime);
                    ++stage;
                }
                
            }
            gettimeofday(&end, NULL);//停止計時

            rawtime = time(NULL);
            strftime (curtime,80,"%r\n",localtime (&rawtime));
            printf("100%% %s", curtime);

            //計算trans time
            double ttime = (end.tv_sec-start.tv_sec) + ((double) end.tv_usec - (double) start.tv_usec)/ 1000000.0;
            printf("Total trans time : %lf ms\n", ttime *1000);
            fclose(fp);
            close(newsockfd);
        }
        else if(mode == RECV_MODE)
        {
            //TCP RECV

            portno = atoi(argv[4]);//get port
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
            serv_addr.sin_addr.s_addr = inet_addr(argv[3]);
            serv_addr.sin_port = htons(portno);
            if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
                error("ERROR connecting");

            //接收檔名
            read(sockfd, filename, 256);
            char * pch, tp[10];
            pch = (char*) memchr(filename, '.', 256);

            //把檔名改成xxx_recv.xxx
            strcpy(tp, pch);
            strcpy(pch,"_recv");
            strcat(filename, tp);
            printf("%s\n",filename);

            //接收檔案大小
            n = read(sockfd, &filestat, sizeof(filestat));
            if(filestat.st_size > 1048576)
                printf("file size : %lf MB\n", (double)filestat.st_size / 1000000.0);
            else if(filestat.st_size > 1024)
                printf("file size : %lf KB\n", (double)filestat.st_size / 1000.0);
            else
                printf("file size : %ld B\n", filestat.st_size);

            //建立檔案
            fp = fopen(filename,"wb");
            if(!fp) error("Cannot open the file\n");

            //回覆已收到
            bzero(buffer,BUFF_SIZE);
            strcpy(buffer,"I got Filename");
            n = write(sockfd,buffer,strlen(buffer));
            if (n < 0) 
                    error("ERROR writing to socket");

            //接收檔案
            long int sendsize = 0;//已傳送的大小
            int stage = 0;//分辨25% 50% 75%
            char curtime[80];//for time stamp
            gettimeofday(&start, NULL);//開始計時
            while(1){
                //接收
                n = read(sockfd, buffer,BUFF_SIZE);
                if (n < 0) 
                    error("ERROR recieving from server");
                if (n == 0) 
                    break;
                //寫入檔案   
                n = fwrite(buffer, sizeof(char), n, fp);

                sendsize += n;//累積傳送大小

                //顯示已經傳了25% 50% 75%
                if((double)sendsize / (double)filestat.st_size > 0.25 && stage == 0){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("25%% %s", curtime);
                    ++stage;
                }
                if((double)sendsize / (double)filestat.st_size > 0.5 && stage == 1){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("50%% %s", curtime);
                    ++stage;
                }
                if((double)sendsize / (double)filestat.st_size > 0.75 && stage == 2){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("75%% %s", curtime);
                    ++stage;
                }
            }
            gettimeofday(&end, NULL);//停止計時

            rawtime = time(NULL);
            strftime (curtime,80,"%r\n",localtime (&rawtime));
            printf("100%% %s", curtime);

            //計算trans time
            double ttime = (end.tv_sec-start.tv_sec) + ((double) end.tv_usec - (double) start.tv_usec)/ 1000000.0;
            printf("Total trans time : %lf ms\n", ttime*1000);

            fflush(fp);
            fclose(fp);
        }
    }
    else if(prot == UDP_PROT){
        if(mode == SEND_MODE){
            //UDP SEND

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
            
            if(filestat.st_size > 1048576)
                printf("file size : %lf MB\n", (double)filestat.st_size / 1000000.0);
            else if(filestat.st_size > 1024)
                printf("file size : %lf KB\n", (double)filestat.st_size / 1000.0);
            else
                printf("file size : %ld B\n", filestat.st_size);
            
            char feedback[256];
            clilen = sizeof(cli_addr);
            while (1){
                //等待request
                printf("wait...\n");
                n = recvfrom(sockfd, feedback, 256, 0,
                            (struct sockaddr *)&cli_addr, &clilen);
                if(n == -1){
                    if (errno == EINTR)
                        continue;
                    error("reciving fail");
                }
                if(n >-1){
                    if(!strcmp(feedback, "Request")){
                        printf("client:%s\n", feedback);
                        break;
                    }
                    else{
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
            while (1){
                n = recvfrom(sockfd, feedback, 256, 0,
                            (struct sockaddr *)&cli_addr, &clilen);
                if(n == -1){
                    if (errno == EINTR)
                        continue;
                    error("reciving fail");
                }
                if(n >-1){
                    if(!strcmp(feedback, "Got filename")){
                        printf("client:%s\n", feedback);
                        break;
                    }
                    else{
                        error("client don't get filename");
                    }                    
                }
            }

            //傳送檔案資訊
            printf("%s\n", filename);
            n = sendto(sockfd, &filestat, sizeof(filestat), 0,
                        (struct sockaddr *)&cli_addr, clilen);
            if(n == -1) error("error sending filename");

            //確認收到
            while (1){
                n = recvfrom(sockfd, feedback, 256, 0,
                            (struct sockaddr *)&cli_addr, &clilen);
                if(n == -1){
                    if (errno == EINTR)
                        continue;
                    error("reciving fail");
                }
                if(n >-1){
                    if(!strcmp(feedback, "Got filestat")){
                        printf("client:%s\n", feedback);
                        break;
                    }
                    else{
                        error("client don't get filestat");
                    }                    
                }
            }
            long int sendsize = 0;//已傳送的大小
            int stage = 0;//分辨25% 50% 75%
            char curtime[80];//for time stamp

            //傳送檔案
            gettimeofday(&start, NULL);//開始計時
            while (!feof(fp))
            {
                //讀檔
                n = fread(buffer, sizeof(char), BUFF_SIZE, fp);
                if (n < 0) error("ERROR reading from file");

                //傳送
                n = sendto(sockfd, buffer, n, 0,
                        (struct sockaddr *)&cli_addr, clilen);
                if(n == -1) error("error sending file");

                //累積傳送大小
                sendsize += n;
                
                //確認client收到檔案
                n = recvfrom(sockfd, feedback, BUFF_SIZE, 0,
                            (struct sockaddr *)&cli_addr, &clilen);
                if (n == -1)
                {
                    if (errno == EINTR)
                        continue;

                    error("recvfrom error");
                }
                else if(n > -1)
                {
                    if(!strcmp(feedback, "Got file")){}
                    else{
                        error("client don't get file");
                    }    
                }

                
                //顯示已經傳了25% 50% 75%
                if((double)sendsize / (double)filestat.st_size > 0.25 && stage == 0){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("25%% %s", curtime);
                    ++stage;
                }
                if((double)sendsize / (double)filestat.st_size > 0.5 && stage == 1){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("50%% %s", curtime);
                    ++stage;
                }
                if((double)sendsize / (double)filestat.st_size > 0.75 && stage == 2){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("75%% %s", curtime);
                    ++stage;
                }
            }
            gettimeofday(&end, NULL);//停止計時

            //傳送結束傳送訊號
            n = sendto(sockfd, buffer, 0, 0,
                        (struct sockaddr *)&cli_addr, clilen);
            
            rawtime = time(NULL);
            strftime (curtime,80,"%r\n",localtime (&rawtime));
            printf("100%% %s", curtime);
            double ttime = (end.tv_sec-start.tv_sec) + ((double) end.tv_usec - (double) start.tv_usec)/ 1000000.0;
            printf("Total trans time : %lf ms\n", ttime *1000);
            fclose(fp);
        }
        else if(mode == RECV_MODE){
            //UDP RECV

            portno = atoi(argv[4]);
            server = gethostbyname(argv[3]);
            sockfd = socket(PF_INET, SOCK_DGRAM, 0);
            if (sockfd < 0)
                error("ERROR opening socket");

            memset(&serv_addr, 0, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(portno);
            serv_addr.sin_addr.s_addr = inet_addr(argv[3]);

            char feedback[256];
            strcpy(feedback,"Request");

            //傳送request
            n = sendto(sockfd, feedback, 256, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(n == -1) error("error sending filename");

            //接收filename
            while (1){
                n = recvfrom(sockfd, filename, 256, 0,
                            NULL, NULL);
                if(n == -1){
                    if (errno == EINTR)
                        continue;
                    error("reciving fail");
                }                
                if(n >-1){
                    if(n > 0){
                        break;
                    }
                    else{
                        error("client don't get filename");
                    }                    
                }
            }

            //傳送已收到檔案名稱
            strcpy(feedback,"Got filename");
            n = sendto(sockfd, feedback, 256, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(n == -1) error("error sending \"Got filename\"");

            //接收檔案資訊
            while (1){
                n = recvfrom(sockfd, &filestat, sizeof(filestat), 0,
                            NULL, NULL);
                if(n == -1){
                    if (errno == EINTR)
                        continue;
                    error("reciving fail");
                }                
                if(n >-1){
                    if(n > 0){
                        break;
                    }
                    else{
                        error("client don't get filename");
                    }                    
                }
            }
            //傳送已收到檔案資訊
            strcpy(feedback,"Got filestat");
            n = sendto(sockfd, feedback, 256, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(n == -1) error("error sending \"Got filestat\"");

            //建立檔案
            char * pch, tp[10];
            pch = (char*) memchr(filename, '.', 256);

            //把檔名改成xxx_recv.xxx
            strcpy(tp, pch);
            strcpy(pch,"_recv");
            strcat(filename, tp);
            printf("%s\n",filename);

            //建立檔案
            fp = fopen(filename,"wb");
            if(!fp) error("Cannot open the file");

            //印出檔案大小
            if(filestat.st_size > 1048576)
                printf("file size : %lf MB\n", (double)filestat.st_size / 1000000.0);
            else if(filestat.st_size > 1024)
                printf("file size : %lf KB\n", (double)filestat.st_size / 1000.0);
            else
                printf("file size : %ld B\n", filestat.st_size);

            long int sendsize = 0;//已傳送的大小
            int stage = 0;//分辨25% 50% 75%
            char curtime[80];//for time stamp
            gettimeofday(&start, NULL);//開始計時
            while(1){
                //接收
                n = recvfrom(sockfd, buffer, BUFF_SIZE, 0,
                            NULL, NULL);
                if (n == -1){
                    if (errno == EINTR)
                        continue;
                    error("recvfrom error");
                }
                else if(n > -1){
                    if(n > 0){
                    }
                    else{
                        break;
                    }    
                }

                //寫入檔案
                n = fwrite(buffer, sizeof(char), n, fp);

                sendsize += n;//累積傳送大小

                //回覆已收到
                strcpy(feedback,"Got file");
                n = sendto(sockfd, feedback, 256, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                if(n == -1) error("error sending \"Got filename\"");
                
                //顯示已經傳了25% 50% 75%
                if((double)sendsize / (double)filestat.st_size > 0.25 && stage == 0){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("25%% %s", curtime);
                    ++stage;
                }
                if((double)sendsize / (double)filestat.st_size > 0.5 && stage == 1){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("50%% %s", curtime);
                    ++stage;
                }
                if((double)sendsize / (double)filestat.st_size > 0.75 && stage == 2){
                    rawtime = time(NULL);
                    strftime (curtime,80,"%r\n",localtime (&rawtime));
                    printf("75%% %s", curtime);
                    ++stage;
                }                
            }
            fflush(fp);

            gettimeofday(&end, NULL);//停止計時
            rawtime = time(NULL);
            strftime (curtime,80,"%r\n",localtime (&rawtime));
            printf("100%% %s", curtime);
            double ttime = (end.tv_sec-start.tv_sec) + ((double) end.tv_usec - (double) start.tv_usec)/ 1000000.0;
            printf("Total trans time : %lf ms\n", ttime*1000);
            printf("packet loss rate : %3.2f%%\n" , 100.0 - 100 * ( (double)sendsize / (double)filestat.st_size));
            fclose(fp);
        }
    }
    
    
    
    close(sockfd);
    return 0;
}

