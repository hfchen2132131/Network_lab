/* Send Multicast Datagram code example. */
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

struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
char databuf[1024] = "Multicast test message lol!";
int datalen = sizeof(databuf);
 
int main (int argc, char *argv[ ])
{
/* Create a datagram socket on which to send. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  printf("Opening the datagram socket...OK.\n");
	 
	/* Initialize the group sockaddr structure with a */
	/* group address of 226.1.1.1 and port 4321. */
	memset((char *) &groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
	groupSock.sin_port = htons(4321);
	 
	/* Set local interface for outbound multicast datagrams. */
	/* The IP address specified must be associated with a local, */
	/* multicast capable interface. */
	localInterface.s_addr = inet_addr("192.168.43.251");
	
	/* IP_MULTICAST_IF:  Sets the interface over which outgoing multicast datagrams are sent. */
	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
	{
	  perror("Setting local interface error");
	  exit(1);
	}
	else
	  printf("Setting the local interface...OK\n");
	/* Send a message to the multicast group specified by the*/
	/* groupSock sockaddr structure. */
	/*int datalen = 1024;*/


	char filename[256];
    FILE *fp;
    struct stat filestat;//for file size
    char *buffer;
    buffer = (char*)malloc(BUFF_SIZE);

	//讀取檔名
	memset(filename,0,256);
	strcpy(filename,argv[1]);
	
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

	if(sendto(sd, filename, 256, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
	{
		perror("Sending datagram filename error");
	}
	else
	  printf("Sending datagram filename...OK\n");
    
    int n = 0;

    
    //傳送檔案
    long int sendsize = 0;//已傳送的大小
    int stage = 0;//分辨25% 50% 75%
    char curtime[80];//for time stamp
    while(!feof(fp)){
        //讀檔
        n = fread(buffer, sizeof(char), BUFF_SIZE, fp);
        if (n < 0) error("ERROR reading from file");
        //傳送
        n = sendto(sd, buffer,n, 0, (struct sockaddr*)&groupSock, sizeof(groupSock));
        if (n < 0) error("ERROR writing to socket");

        sendsize += n;//累積傳送大小   
        //printf("send %d\n",sendsize);     
    }
    n = sendto(sd, "",0, 0, (struct sockaddr*)&groupSock, sizeof(groupSock));
    if (n < 0) error("ERROR writing to socket");

    fclose(fp);
    close(sd);
	 
	/* Try the re-read from the socket if the loopback is not disable
	if(read(sd, databuf, datalen) < 0)
	{
	perror("Reading datagram message error\n");
	close(sd);
	exit(1);
	}
	else
	{
	printf("Reading datagram message from client...OK\n");
	printf("The message is: %s\n", databuf);
	}
	*/
	return 0;
}
