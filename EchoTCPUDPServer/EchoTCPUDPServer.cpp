// EchoTCPUDPServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "SocketFrame.h"
#define ECHOPORT "7210"
#include<iostream>
#include <time.h>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	CSocketFrame frame;
	int iResult = 0;
	char    recvbuf[MAXLINE];
	SOCKET ListenSocket, ConnectSocket, ServerSocket;
	struct sockaddr_in	cliaddr;
	int addrlen =sizeof( sockaddr_in );

	time_t ticks;
	
	//Windows Sockets Dll��ʼ��
	frame.start_up();

	//�����������˵���ʽ�׽��ֲ���ָ���˿ں��ϼ���
	ListenSocket = frame.tcp_server( NULL, ECHOPORT );
	if ( ListenSocket == -1 )
		return -1;
	//���ö˿ڿ�����
	int on =true;
    iResult = setsockopt(ListenSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
    if ( iResult == SOCKET_ERROR)
    {
	   frame.quit( ListenSocket);
	   return -1;
    }

	//�����������˵����ݱ��׽��ֲ��󶨶˵��ַ
	ServerSocket = frame.udp_server( NULL, ECHOPORT );
	if ( ServerSocket == -1 )
		return -1;

    printf("������׼���û�����񡣡���\n");

	fd_set fdRead,fdSocket;
    FD_ZERO( &fdSocket );
	FD_SET( ServerSocket, &fdSocket);
	FD_SET( ListenSocket, &fdSocket);

	while( TRUE)
	{
		//ͨ��select�ȴ����ݵ����¼���������¼�������select�����Ƴ�fdRead������û��δ��I/O�������׽��־����Ȼ�󷵻�
	    fdRead = fdSocket;
		iResult = select( 0, &fdRead, NULL, NULL, NULL);
		if (iResult >0)
		{
			//�������¼�����
			//ȷ������Щ�׽�����δ����I/O������һ��������ЩI/O
			for (int i=0; i<(int)fdSocket.fd_count; i++)
			{
				if (FD_ISSET( fdSocket.fd_array[i] ,&fdRead))
				{
					if( fdSocket.fd_array[i] == ListenSocket)
					{
						if( fdSocket.fd_count < FD_SETSIZE)
						{
							//ͬʱ���õ��׽����������ܴ���FD_SETSIZE
							//���µ���������
							ConnectSocket = accept(ListenSocket,(sockaddr FAR*)&cliaddr,&addrlen);   
							if( ConnectSocket == INVALID_SOCKET)   
							{   
								printf("accept failed !\n");   
								closesocket( ListenSocket );   
								WSACleanup();   
								return 1;   
							}
							//�����µ������׽��ֽ��и��õȴ�
							FD_SET( ConnectSocket, &fdSocket);
							printf("���յ��µ����ӣ�%s\n", inet_ntoa( cliaddr.sin_addr));
						}
						else
						{
							printf("���Ӹ�������!\n"); 
							continue;
						}
					}
					else if( fdSocket.fd_array[i] == ServerSocket)
					{
						//��UDPЭ�����ݵ���
						memset( recvbuf, 0, MAXLINE );
						//��������
						iResult = recvfrom( fdSocket.fd_array[i], recvbuf, MAXLINE, 0, (SOCKADDR *)&cliaddr, &addrlen);
						if (iResult > 0)
						{
							printf("\n-----------------------------\n�ɹ�����UDP�ͻ���\nIP��ַ��%s �˿ڵ�ַ��%d \n\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
							printf("�������˽��յ�����%s\n", recvbuf);		


							//���䷢�����յ�������
							
							//��ȡ��ǰʱ��
							ticks = time(NULL);
							memset(recvbuf, 0, sizeof(recvbuf));
							sprintf(recvbuf, "%.24s\r\n", ctime(&ticks));
							printf("��ȡ��ǰϵͳʱ�䣺 %s\n", recvbuf);
							
							iResult = sendto( fdSocket.fd_array[i],recvbuf,24, 0, (SOCKADDR *)&cliaddr, addrlen );
							if(iResult == SOCKET_ERROR)
							{
								printf("sendto �������ô��󣬴����: %ld\n", WSAGetLastError());
								closesocket(fdSocket.fd_array[i]);
								FD_CLR(fdSocket.fd_array[i], &fdSocket);
							}
						}
						else if (iResult == 0) 
						{
							//���2�����ӹر�
							printf("����һ�������ѹر�...\n"); 
							closesocket(fdSocket.fd_array[i]);
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
						else
						{
							printf("recvfrom �������ô��󣬴����: %d\n", WSAGetLastError());
							closesocket(fdSocket.fd_array[i]); 
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
					}
					else 
					{
						//��TCPЭ�����ݵ���
						memset( recvbuf, 0, MAXLINE );
						//��������
						iResult = recv( fdSocket.fd_array[i], recvbuf, MAXLINE, 0 );
						if (iResult > 0)
						{
							printf("\n-----------------------------\n�ɹ�����TCP�ͻ���\nIP��ַ��%s �˿ڵ�ַ��%d \n\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
							printf("�������˽��յ�����%s\n", recvbuf);							
							

							//��ȡ��ǰʱ��
							ticks = time(NULL);
							memset(recvbuf, 0, sizeof(recvbuf));
							sprintf(recvbuf, "%.24s\r\n", ctime(&ticks));
							printf("��ȡ��ǰϵͳʱ�䣺 %s\n", recvbuf);

							iResult = send( fdSocket.fd_array[i],recvbuf,24, 0 );
							if(iResult == SOCKET_ERROR)
							{
								printf("send �������ô��󣬴����: %ld\n", WSAGetLastError());
								closesocket(fdSocket.fd_array[i]);
								FD_CLR(fdSocket.fd_array[i], &fdSocket);
							}
						}
						else if (iResult == 0) 
						{
							//���2�����ӹر�
							printf("��ǰ���ӹر�...\n"); 
							closesocket(fdSocket.fd_array[i]);
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
						else
						{
							printf("recv �������ô��󣬴����: %d\n", WSAGetLastError());
							closesocket(fdSocket.fd_array[i]); 
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
					}
				}
			}
		}
		else
		{
			printf("select �������ô��󣬴����: %d\n",WSAGetLastError() ); 
			break; 
		}
	}

    // cleanup 
    closesocket(ServerSocket); 
	frame.quit( ListenSocket );
    return 0; 
}

