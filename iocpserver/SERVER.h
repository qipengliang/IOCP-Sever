#pragma once
#pragma comment (lib,"ws2_32.lib")
#include<winsock2.h>
#include<mutex>
#include<string>
#include "TPool.h"
#include<iostream>
#include<string>
#include"So_context.h"
#define RELEASE_ARRAY(x) {if(x != nullptr ){delete[] x;x=nullptr;}} 
#define RELEASE_POINTER(x) {if(x != nullptr ){delete x;x=nullptr;}} 
#define RELEASE_HANDLE(x) {if(x != nullptr && x!=INVALID_HANDLE_VALUE)\
	{ CloseHandle(x); x = nullptr; }} // �ͷž����
#define RELEASE_SOCKET(x) {if(x != NULL && x !=INVALID_SOCKET) \
	{ closesocket(x); x = INVALID_SOCKET; }} // �ͷ�Socket��
class SERVER;
struct param
{
	SERVER* iocpm;
	int threadid;
};
class SERVER
{
private:
	LPFN_ACCEPTEX  m_lpfnAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockAddrs;
	//struct sockaddr_in sever_add;
	HANDLE m_iocp;
	std::vector<SOCKADDR_IN*>clientadd;

	void _doaccept(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes);
	void _dorecv(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes);
	void _postsend(So_context* socontext, Iocontex* iocontext);
	void _dosend(So_context* socontext, Iocontex* iocontext);
	void remove(So_context* socontext, Iocontex* iocontext);
	void init(Iocontex* iocontext);
public:
	static DWORD WINAPI workthread(LPVOID param);
	SERVER(HANDLE IOCP);
	~SERVER();
};



