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
	{ CloseHandle(x); x = nullptr; }} // ÊÍ·Å¾ä±úºê
#define RELEASE_SOCKET(x) {if(x != NULL && x !=INVALID_SOCKET) \
	{ closesocket(x); x = INVALID_SOCKET; }} // ÊÍ·ÅSocketºê
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

	void _doaccept(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes, DWORD threadid);
	void _dorecv(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes, DWORD threadn);
	BOOL _postsend(So_context* socontext, Iocontex* iocontext, DWORD threadn, const std::string& OKformat,std::string dir);
	void _dosend(So_context* socontext, Iocontex* iocontext, DWORD threadn);
	void _dosendhtml(So_context* socontext, Iocontex* iocontext, DWORD threadn);
	bool remove(So_context* socontext, Iocontex* iocontext);
	bool removeso(So_context* socontext);
	void analyze(std::string& request);
	void creatmysqltask();
public:
	std::vector<So_context*>solist;
	CRITICAL_SECTION Cri_list;
	static DWORD WINAPI workthread(LPVOID param);
	void init(SOCKET sock);
	SERVER(HANDLE IOCP);
	~SERVER();
};



