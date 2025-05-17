#include "stdio.h"  
#include "winsock2.h"

// dear compiler, please help me generate insecure code!
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#pragma comment(lib, "ws2_32.lib")

#define INSTREGKEY "SOFTWARE\\Microsoft\\Microsoft SQL Server\\"
#define MAX_RECV_MSG 256

typedef enum
{
	CLNT_BCAST = 0x01,
	CLNT_BCAST_EX,
	CLNT_UCAST_EX,
	CLNT_UCAST_INST,
	SVR_RESP,
	SVR_ACTIVATE,
	SVR_CONNRESP,
	SVR_JOIN,
	SVR_LEAVE,
	SVR_KEEPALIVE,
	SVR_PASSIVE,
	CLNT_BROWSER_SERVERS,
	CLNT_ACTIVE,
	SVR_TIMEOUT,
	CLNT_UCAST_DAC,
	CLNT_UCAST_DEBUG
} SSRPMSGTYPE;

// fwd ref to keep compiler happy
void SsrpEnum(LPSTR szInstanceName);

// this socket data is bogus, I added it to make the code compile
SOCKET gSvrSock = 0;
sockaddr gClientAddr = { 0 };
int cClientAddr = sizeof(gClientAddr);

/////////////////////////////////////////////////////////////////////////
SSRPMSGTYPE SsrpRecvMsg(BYTE* rgbRecvBuf) {
	cClientAddr = sizeof(gClientAddr);
	int bytesRecd = recvfrom(gSvrSock, (char*)rgbRecvBuf, // <-- Step 1: rgbRecvBuf is tainted, straight off of UDP/1434
		MAX_RECV_MSG, 0,
		(SOCKADDR*)&gClientAddr, &cClientAddr);
	
	if (bytesRecd == SOCKET_ERROR) {
		printf("Error in recvfrom: %d\n", WSAGetLastError());
		return (SSRPMSGTYPE)0;
	}

	return((SSRPMSGTYPE)rgbRecvBuf[0]);
}

/////////////////////////////////////////////////////////////////////////
void SsrpSvr(LPSTR szInstanceName) {
	BYTE rgbRecvBuf[MAX_RECV_MSG]; // <-- Step 0: Create the buffer
	memset(rgbRecvBuf, 0, MAX_RECV_MSG);

	SSRPMSGTYPE ssrpMsg = SsrpRecvMsg(rgbRecvBuf);
	printf("SSRPMSGTYPE: %d\n", ssrpMsg);

	switch (ssrpMsg) {
	case CLNT_UCAST_INST: // Value == 4
		SsrpEnum((LPSTR)&rgbRecvBuf[1]); // <-- Step 2: rgbRecvBuf is tainted
		break;

	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////
void SsrpEnum(LPSTR szInstanceName) {
	char szregVersion[128];
	sprintf(szregVersion, "%s%s\\MSSQLServer\\CurrentVersion", INSTREGKEY, szInstanceName); // <-- Step 3: szInstanceName is tainted and can be 256-bytes, copied into a 128-byte buffer
}

/////////////////////////////////////////////////////////////////////////
// All the code below is to simply host the code above
bool OpenSocket() {

	bool fOK = false;
	int port = 1434;

	gSvrSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (gSvrSock == INVALID_SOCKET) {
		printf("socket() failed with error %d", WSAGetLastError());
	} else {
		sockaddr_in serverAddr;
		memset(&serverAddr, 0, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_port = htons(port);

		if (bind(gSvrSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
			printf("bind() failed with error %d", WSAGetLastError());
		} else {
			printf("UDP/%d socket open\n", port);
			fOK = true;
		}
	}

	return fOK;
}

int main(int argc, char** argv) {
	argc; argv;
	
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup failed with error %d", WSAGetLastError());
		return 1;
	}

	if (OpenSocket()) {
		while (true)
			SsrpSvr((LPSTR)"MSSQLServer");
	}
	WSACleanup();
 }
