#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

void* listeningThread(void *bcSock);

int main(int argc, char *argv[])
{
	int bcSock;
	struct sockaddr_in BroadcastAddr;
	char *BroadcastIP;
	unsigned short BroadcastPort;

	char *SendString;
	unsigned int SendStringLen;
	int BroadcastPermission;

	int Status;
	int i;
	pthread_t hilo;

	if(argc < 3)
	{
		fprintf(stderr, "Se necesita : %s <IP Address> <Port>\n", argv[0]);
		exit(1);
	} 

	BroadcastIP = argv[1]; 				/*First arg: Broadcast IP Address*/
	BroadcastPort = atoi(argv[2]);      /*Second arg: Broadcast Port*/
	SendString = "Mensaje del hostdiscover: HELLO";

		bcSock = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
	    if(bcSock == -1)
			{	
				fprintf(stderr, "No se puede crear el socket");
				return 1;
			}              

	BroadcastPermission = 1;
	Status = setsockopt(bcSock,SOL_SOCKET,SO_BROADCAST, (void *)&BroadcastPermission, sizeof(BroadcastPermission));
		if(Status == -1)
		  {
			fprintf(stderr,"No es posible mandar en broadcast");
			return 1;
		  }

		memset(&BroadcastAddr,0,sizeof(BroadcastAddr));
		BroadcastAddr.sin_family = AF_INET;
		inet_pton(AF_INET, BroadcastIP, &BroadcastAddr.sin_addr.s_addr);
		BroadcastAddr.sin_port = htons (BroadcastPort);

		Status = pthread_create(&(hilo), NULL, listeningThread,&bcSock);
		if(Status != 0)
		{
			printf("No se puede crear: [%s]\n", strerror(Status));
			return 1;
		}

		SendStringLen = strlen(SendString);
		for(i = 0; i < 100; i++)
		{
			Status = sendto(bcSock,SendString,SendStringLen,0,(struct sockaddr*)&BroadcastAddr, sizeof(BroadcastAddr));
			printf("Se enviaron %i bytes hacia la direccion de boradcast\n", Status);
			sleep(3);
		}

   return 0;	
}   

void* listeningThread(void *bcSock)
{
	char buffer[255];
	char ip[17];
	u_short clientPort;
	struct sockaddr_in ListenAddr;
	socklen_t Addrlen = sizeof(ListenAddr);
	int socket = *((int*)bcSock);

		while(1)
		{
			bzero(buffer,255);
			recvfrom(socket, buffer,255,0,(struct sockaddr*)&ListenAddr, &Addrlen);
			ListenAddr.sin_family = AF_INET;
			inet_ntop(AF_INET,&(ListenAddr.sin_addr),ip,INET_ADDRSTRLEN);
			clientPort = ntohs(ListenAddr.sin_port);
			printf("Se contesto desde: [%s:%i] %s\n",ip,clientPort,buffer);
		}

	return NULL;
}
