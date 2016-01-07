// EchoTCPUDPServer.cpp : 定义控制台应用程序的入口点。
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
	
	//Windows Sockets Dll初始化
	frame.start_up();

	//创建服务器端的流式套接字并在指定端口号上监听
	ListenSocket = frame.tcp_server( NULL, ECHOPORT );
	if ( ListenSocket == -1 )
		return -1;
	//设置端口可重用
	int on =true;
    iResult = setsockopt(ListenSocket,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
    if ( iResult == SOCKET_ERROR)
    {
	   frame.quit( ListenSocket);
	   return -1;
    }

	//创建服务器端的数据报套接字并绑定端点地址
	ServerSocket = frame.udp_server( NULL, ECHOPORT );
	if ( ServerSocket == -1 )
		return -1;

    printf("服务器准备好回射服务。。。\n");

	fd_set fdRead,fdSocket;
    FD_ZERO( &fdSocket );
	FD_SET( ServerSocket, &fdSocket);
	FD_SET( ListenSocket, &fdSocket);

	while( TRUE)
	{
		//通过select等待数据到达事件，如果有事件发生，select函数移除fdRead集合中没有未决I/O操作的套接字句柄，然后返回
	    fdRead = fdSocket;
		iResult = select( 0, &fdRead, NULL, NULL, NULL);
		if (iResult >0)
		{
			//有网络事件发生
			//确定有哪些套接字有未决的I/O，并进一步处理这些I/O
			for (int i=0; i<(int)fdSocket.fd_count; i++)
			{
				if (FD_ISSET( fdSocket.fd_array[i] ,&fdRead))
				{
					if( fdSocket.fd_array[i] == ListenSocket)
					{
						if( fdSocket.fd_count < FD_SETSIZE)
						{
							//同时复用的套接字数量不能大于FD_SETSIZE
							//有新的连接请求
							ConnectSocket = accept(ListenSocket,(sockaddr FAR*)&cliaddr,&addrlen);   
							if( ConnectSocket == INVALID_SOCKET)   
							{   
								printf("accept failed !\n");   
								closesocket( ListenSocket );   
								WSACleanup();   
								return 1;   
							}
							//增加新的连接套接字进行复用等待
							FD_SET( ConnectSocket, &fdSocket);
							printf("接收到新的连接：%s\n", inet_ntoa( cliaddr.sin_addr));
						}
						else
						{
							printf("连接个数超限!\n"); 
							continue;
						}
					}
					else if( fdSocket.fd_array[i] == ServerSocket)
					{
						//有UDP协议数据到达
						memset( recvbuf, 0, MAXLINE );
						//接收数据
						iResult = recvfrom( fdSocket.fd_array[i], recvbuf, MAXLINE, 0, (SOCKADDR *)&cliaddr, &addrlen);
						if (iResult > 0)
						{
							printf("\n-----------------------------\n成功连接UDP客户端\nIP地址：%s 端口地址：%d \n\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
							printf("服务器端接收到数据%s\n", recvbuf);		


							//回射发送已收到的数据
							
							//获取当前时间
							ticks = time(NULL);
							memset(recvbuf, 0, sizeof(recvbuf));
							sprintf(recvbuf, "%.24s\r\n", ctime(&ticks));
							printf("获取当前系统时间： %s\n", recvbuf);
							
							iResult = sendto( fdSocket.fd_array[i],recvbuf,24, 0, (SOCKADDR *)&cliaddr, addrlen );
							if(iResult == SOCKET_ERROR)
							{
								printf("sendto 函数调用错误，错误号: %ld\n", WSAGetLastError());
								closesocket(fdSocket.fd_array[i]);
								FD_CLR(fdSocket.fd_array[i], &fdSocket);
							}
						}
						else if (iResult == 0) 
						{
							//情况2：连接关闭
							printf("其中一个连接已关闭...\n"); 
							closesocket(fdSocket.fd_array[i]);
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
						else
						{
							printf("recvfrom 函数调用错误，错误号: %d\n", WSAGetLastError());
							closesocket(fdSocket.fd_array[i]); 
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
					}
					else 
					{
						//有TCP协议数据到达
						memset( recvbuf, 0, MAXLINE );
						//接收数据
						iResult = recv( fdSocket.fd_array[i], recvbuf, MAXLINE, 0 );
						if (iResult > 0)
						{
							printf("\n-----------------------------\n成功连接TCP客户端\nIP地址：%s 端口地址：%d \n\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
							printf("服务器端接收到数据%s\n", recvbuf);							
							

							//获取当前时间
							ticks = time(NULL);
							memset(recvbuf, 0, sizeof(recvbuf));
							sprintf(recvbuf, "%.24s\r\n", ctime(&ticks));
							printf("获取当前系统时间： %s\n", recvbuf);

							iResult = send( fdSocket.fd_array[i],recvbuf,24, 0 );
							if(iResult == SOCKET_ERROR)
							{
								printf("send 函数调用错误，错误号: %ld\n", WSAGetLastError());
								closesocket(fdSocket.fd_array[i]);
								FD_CLR(fdSocket.fd_array[i], &fdSocket);
							}
						}
						else if (iResult == 0) 
						{
							//情况2：连接关闭
							printf("当前连接关闭...\n"); 
							closesocket(fdSocket.fd_array[i]);
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
						else
						{
							printf("recv 函数调用错误，错误号: %d\n", WSAGetLastError());
							closesocket(fdSocket.fd_array[i]); 
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
					}
				}
			}
		}
		else
		{
			printf("select 函数调用错误，错误号: %d\n",WSAGetLastError() ); 
			break; 
		}
	}

    // cleanup 
    closesocket(ServerSocket); 
	frame.quit( ListenSocket );
    return 0; 
}

