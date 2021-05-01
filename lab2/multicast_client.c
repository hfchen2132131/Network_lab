/* Receiver/client multicast Datagram example. */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFF_SIZE 65500
 
struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
int datalen;
char databuf[1024];
 
int main(int argc, char *argv[])
{
/* Create a datagram socket on which to receive. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	else
	printf("Opening datagram socket....OK.\n");
		 
	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
	
	int reuse = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("Setting SO_REUSEADDR error");
		close(sd);
		exit(1);
	}
	else
		printf("Setting SO_REUSEADDR...OK.\n");
	
	 
	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;
	localSock.sin_port = htons(4321);
	localSock.sin_addr.s_addr = INADDR_ANY;
	if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
	{
		perror("Binding datagram socket error");
		close(sd);
		exit(1);
	}
	else
		printf("Binding datagram socket...OK.\n");
	 
	/* Join the multicast group 226.1.1.1 on the local address*/
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
	group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
	/* your ip address */ 
	group.imr_interface.s_addr = inet_addr("192.168.43.251"); 
	/* IP_ADD_MEMBERSHIP:  Joins the multicast group specified */ 
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(sd);
		exit(1);
	}
	else
		printf("Adding multicast group...OK.\n");
	 

    char filename[256],fname[256];
    FILE *fp;
    struct stat filestat;//for file size
    char *buffer;
    buffer = (char*)malloc(BUFF_SIZE);
    int n = 0;

    //建立檔案
    strcpy(fname,"recv_tp");
    if(argv[1] != NULL)
        strcat(fname,argv[1]);
    fp = fopen(fname,"wb");
    if(!fp) perror("Cannot open the file");

	/* Read from the socket. */
	datalen = sizeof(databuf);
	if(recvfrom(sd, filename, 256, 0, NULL, NULL) < 0)
	{
		perror("Reading datagram filename error");
		close(sd);
		exit(1);
	}
	else
	{
		printf("Reading datagram filename...OK.\n");
		printf("The message from multicast server is: \"%s\"\n", filename);
	}


    long int sendsize = 0;//已傳送的大小
    while(1){
        //接收
        n = recvfrom(sd, buffer, BUFF_SIZE, 0,
                    NULL, NULL);
        if (n < 0){
            perror("Reading datagram file error");
            close(sd);
            exit(1);
        }
        //else printf("got!!\n");
        if(n == 0) break;

        //寫入檔案
        n = fwrite(buffer, sizeof(char), n, fp);

        sendsize += n;//累積傳送大小
        
        
    }
    //印出檔案大小
    if(sendsize > 1048576)
        printf("recieve file size : %lf MB\n", (double)sendsize / 1000000.0);
    else if(sendsize > 1024)
        printf("recieve file size : %lf KB\n", (double)sendsize / 1000.0);
    else
        printf("recieve file size : %ld B\n", sendsize);
    fflush(fp);
    fclose(fp);
    close(sd);

    char * pch, tp[10];
    pch = (char*) memchr(filename, '.', 256);
    //把檔名改成xxx_recv.xxx
    strcpy(tp, pch);
    strcpy(pch,"_recv");
    if(argv[1] != NULL)
        strcat(pch,argv[1]);
    strcat(filename, tp);
    printf("%s\n",filename);
    char cmd[500];
    strcpy(cmd,"mv ");
    strcat(cmd,fname);
    strcat(cmd," ");
    strcat(cmd,filename);
    system(cmd);

	return 0;
}
