 //包含实验室的通信库
 #include <windows.h>
 #include "..\\DLL_PtoPCommunicatio\\UDPComBase.h"
#include <iostream> 

 int main()
 {
 	// 新建udp类库
 	CUDPComBase* udpSocet= UDPComExports(NULL);
 
 	udpSocet->InitUDPCom();
 
 	SOCKET  m_skUDPSendSocket;       // UDP发送套接字
 
 	// 本地IP地址
 	BYTE abyLocalIp[IP_BYTE_LENGTH];
 	abyLocalIp[0] = 10;
 	abyLocalIp[0] = 113;
 	abyLocalIp[0] = 1;
 	abyLocalIp[0] = 100;
 
 	BYTE abyUdpDestIp[IP_BYTE_LENGTH];
 	abyUdpDestIp[0] = 115;
 	abyUdpDestIp[1] = 28;
 	abyUdpDestIp[2] = 14;
 	abyUdpDestIp[3] = 79;
 
 	UINT port = 80;
 
 	udpSocet->StartUDPSender(m_skUDPSendSocket,abyLocalIp,port);
 
 	//待发送的数据长度
 	char* pcSendMessage = new char[255];
 	memset(pcSendMessage, 0, 255 * sizeof(char));
 
 	char* sendMessage = "hello world!";
 	memcpy(pcSendMessage, sendMessage, sizeof(sendMessage));
 	
 	for (int i = 0;; i++)
 	{
		udpSocet->UDPSend(m_skUDPSendSocket, abyUdpDestIp, port, (byte*)pcSendMessage, 255, i); 
		std::cout << i << std::endl;
 	}
 	
 	udpSocet->EndUDPSender(m_skUDPSendSocket);
 	return 0;
 }


// #include <stdio.h>
// #include <winsock.h>
// #define CPORT 1520
// #define VERSIONRE MAKEWORD(2, 2)
//  int main()
//  {
// 	 int s;  //define two sockets
// 	 int nError;
// 	 WSADATA wsadata;
// 	 struct sockaddr_in sin, clientaddr;  //define an address
// 	 int retVal;
// 	 char buf[64];
// 	 int addrLen;
// 	 addrLen = sizeof(clientaddr);
// 	 memset(buf, 64, 0);
// 	 nError = WSAStartup(VERSIONRE, &wsadata);
// 	 if (nError != 0)
// 	 {
// 		 printf("None avaliable DLL!\n");
// 		 return -1;
// 	 }
// 
// 	 sin.sin_family = AF_INET;
// 	 sin.sin_port = htons((unsigned short)CPORT);
// 	 sin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
// 
// 	 s = socket(AF_INET, SOCK_DGRAM, 0);//make a socket
// 	 if (s == INVALID_SOCKET)
// 	 {
// 		 printf("socket failed!\n");
// 		 closesocket(s);
// 		 WSACleanup();
// 		 return -1;
// 	 }
// 
// 	 retVal = bind(s, (struct sockaddr*)(&sin), sizeof(sin));//bind socket to address
// 	 if (retVal == SOCKET_ERROR)
// 	 {
// 		 printf("bind failed!\n");
// 		 closesocket(s);
// 		 WSACleanup();
// 		 return -1;
// 	 }
// 	 retVal = recvfrom(s, buf, 64, 0, (struct sockaddr*)(&clientaddr), &addrLen);
// 	 if (retVal == SOCKET_ERROR)
// 	 {
// 		 printf("recv failed!\n");
// 		 closesocket(s);
// 		 WSACleanup();
// 		 return -1;
// 	 }
// 	 buf[retVal] = 0;
// 	 printf("%s\n", buf);
// 	 memset(buf, 64, 0);
// 	 strcpy(buf, "Hello!\n");
// 	 retVal = sendto(s, buf, 64, 0, (struct sockaddr*)(&clientaddr), sizeof(clientaddr));
// 	 if (retVal == SOCKET_ERROR)
// 	 {
// 		 printf("send failed!\n");
// 		 closesocket(s);
// 		 WSACleanup();
// 		 return -1;
// 	 }
// 
// 	 closesocket(s);//close socket
// 
// 	 WSACleanup();
// 	 return 0;
// }