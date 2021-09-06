#include "TPool.h"
#include<thread>
#include<vector>
#include<queue>
#include<functional>
#include<iostream>
#include<mutex>
#include<condition_variable>
#include<future>
#include"SERVER.h"
#include"So_context.h"
#include <WinSock2.h>
#include <MSWSock.h>
#include <WinNt.h>
#include<memory>

#define MAX_SOCKET 5;


void main()
{
	
	//1.主线程建立完成端口
	HANDLE IOCP=CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	//2.建立监听socket
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){            //加载winsock2库
		printf("加载失败./n");
	}
	SOCKET listen_socket;
	struct sockaddr_in sever_add;
	ZeroMemory((char*)&sever_add, sizeof(sever_add));
	listen_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);;//建立监听套接字，ipv4协议，TCP协议
	if (listen_socket == INVALID_SOCKET){
		printf("建立接口失败/n");
	}
	So_context* listen_socket_context = new So_context;
	listen_socket_context->m_socket = listen_socket;
	//3.完成socket和完成端口的绑定
	CreateIoCompletionPort((HANDLE)listen_socket, IOCP, (DWORD)listen_socket_context, 0);//4.socket_context是指向socket_context结构体的指针,为io消息包,绑定socket和iocp
	sever_add.sin_family = AF_INET;
	sever_add.sin_addr.S_un.S_addr = INADDR_ANY; //INADDR_ANY:本机IP地址
	sever_add.sin_port = htons(4000);
	::bind(listen_socket, (sockaddr*)&sever_add, sizeof(sever_add));

	//4.创建n个线程,SERVER类负责管理线程函数以及创建的socontext
	SERVER* ioserver=new SERVER(IOCP);
	//得到accpetex，和GetAcceptExSockAddrs的函数指针
	ioserver->init(listen_socket);

	//得到系统拥有的cpu数量
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int m_nProcessors = si.dwNumberOfProcessors;
	int n_maxthread = 2 * m_nProcessors;
	DWORD threadn=0;
	//建立2*cpu个线程句柄数组，用于最后释放线程
	HANDLE* m_thread = new HANDLE[n_maxthread];
	//调用winAPI建立线程，传参为线程函数，函数参数，存放线程id的地址，将返回线程句柄，和线程id，传入函数格式必须为DWORD 函数名(LPVOID para)
	for (int i = 0; i < n_maxthread; i++)
	{
		param* parameter=new param;
		parameter->iocpm=ioserver;
		m_thread[i] =CreateThread(0, 0, SERVER::workthread, (void*)parameter, 0, &threadn);
		parameter->threadid = threadn;
	}

	//开始侦听
	::listen(listen_socket, SOMAXCONN);
	std::cout << "listensocket" << std::endl;



	//5.向socket投递acceptex
	LPFN_ACCEPTEX  m_lpfnAcceptEx;         // AcceptEx函数指针
	GUID GuidAcceptEx = WSAID_ACCEPTEX;    // GUID，这个是识别AcceptEx函数必须的
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &dwBytes, NULL, NULL))
	{
		std::cout<<"寻找acceptex失败"<<std::endl;
	}
	int num_acceptsocket = MAX_SOCKET;
	for (int i = 0; i < num_acceptsocket; i++)
	{
		Iocontex* iocontext = new Iocontex;//已经调用默认构造初始化完毕，m_socket=INVAIL_SOCKET
		iocontext->m_PostType = PostType::ACCEPT;
		iocontext->m_socket = WSASocket(AF_INET, SOCK_STREAM,
			IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		listen_socket_context->array_IoContext.push_back(iocontext);
		DWORD dwBytes = 0;
		DWORD dwAddrLen = (sizeof(SOCKADDR_IN)+16);
		WSABUF* pWSAbuf = &iocontext->m_wsaBuf;
		if (!m_lpfnAcceptEx(listen_socket, iocontext->m_socket, pWSAbuf->buf, 0, dwAddrLen, dwAddrLen, &dwBytes, &iocontext->m_overlap))
		{
				int nErr = WSAGetLastError();
				if (WSA_IO_PENDING != nErr)
				{
					printf("accept失败");
				}
				else
				{
					std::cout << "accept" << i << "成功" << std::endl;
				}
		}
	}
	//初始化同步用的临界区
	InitializeCriticalSection(&ioserver->Cri_list);
	EnterCriticalSection(&ioserver->Cri_list);
	ioserver->solist.push_back(listen_socket_context);
	LeaveCriticalSection(&ioserver->Cri_list);
	//等待服务器退出信号q
	char quit='0';
	while (quit != 'q')
	{
		std::cout << "q o n" << std::endl;
		std::cin >> quit;
	}

	//向各线程post socontext为NULL的io完成端口，当线程得到socontext为空时，结束线程
	for (int i = 0; i < n_maxthread; i++){
		PostQueuedCompletionStatus(IOCP, 0, (DWORD)NULL, NULL);
	}
	//等待
	WaitForMultipleObjects(n_maxthread, m_thread,
		TRUE, INFINITE);
	//释放资源
	for (int i = 0; i < n_maxthread; i++)
	{
		RELEASE_HANDLE(m_thread[i]);
	}

	RELEASE_ARRAY(m_thread);
	// 关闭IOCP句柄
	RELEASE_HANDLE(IOCP);
	//统一释放线程资源
	ioserver->solist.clear();
	// 关闭监听Socket
	RELEASE_POINTER(listen_socket_context);
	RELEASE_SOCKET(listen_socket);
	printf("释放完成");
}