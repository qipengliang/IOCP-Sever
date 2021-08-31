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
//���̸߳�����
//1.���߳̽�����ɶ˿�
//2.����n���߳�
//3.��������socket
//4.���socket����ɶ˿ڵİ�
//5.��socketͶ��acceptex
//6.end
#define MAX_SOCKET 5;

void main()
{
	
	//1.
	HANDLE IOCP=CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	SERVER* ioserver=new SERVER(IOCP);
	//�õ�ϵͳӵ�е�cpu����
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int m_nProcessors = si.dwNumberOfProcessors;
	int n_maxthread = 2 * m_nProcessors;
	DWORD threadn=0;
	//����2*cpu���߳̾�����飬��������ͷ��߳�
	HANDLE* m_thread = new HANDLE[n_maxthread];
	//2.����winAPI�����̣߳�����Ϊ�̺߳�������������������߳�id�ĵ�ַ���������߳̾�������߳�id�����뺯����ʽ����ΪDWORD ������(LPVOID para)
	for (int i = 0; i < n_maxthread; i++)
	{
		param* parameter=new param;
		parameter->iocpm=ioserver;
		m_thread[i] =CreateThread(0, 0, SERVER::workthread, (void*)parameter, 0, &threadn);
		parameter->threadid = threadn;
	}
	//3.
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){            //����winsock2��
		printf("����ʧ��./n"); 
	}
	SOCKET listen_socket;
	struct sockaddr_in sever_add;
	ZeroMemory((char*)&sever_add, sizeof(sever_add));
	listen_socket = WSASocket(AF_INET, SOCK_STREAM, 0,NULL, 0, WSA_FLAG_OVERLAPPED);;//���������׽��֣�ipv4Э�飬TCPЭ��
	if (listen_socket == INVALID_SOCKET){
		printf("�����ӿ�ʧ��/n");
	}
	So_context* listen_socket_context = new So_context;
	listen_socket_context->m_socket = listen_socket;
	CreateIoCompletionPort((HANDLE)listen_socket, IOCP, (DWORD)listen_socket_context, 0);//4.socket_context��ָ��socket_context�ṹ���ָ��,Ϊio��Ϣ��,��socket��iocp
	sever_add.sin_family = AF_INET;
	sever_add.sin_addr.S_un.S_addr = INADDR_ANY; //INADDR_ANY:����IP��ַ
	sever_add.sin_port = htons(4000);
	::bind(listen_socket, (sockaddr*)&sever_add, sizeof(sever_add));
	::listen(listen_socket, SOMAXCONN);
	std::cout << "listensocket" << std::endl;
	//5.
	LPFN_ACCEPTEX  m_lpfnAcceptEx;         // AcceptEx����ָ��
	GUID GuidAcceptEx = WSAID_ACCEPTEX;    // GUID�������ʶ��AcceptEx���������
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(listen_socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &dwBytes, NULL, NULL))
	{
		std::cout<<"Ѱ��acceptexʧ��"<<std::endl;
	}
	int num_acceptsocket = MAX_SOCKET;
	for (int i = 0; i < num_acceptsocket; i++)
	{
		Iocontex* iocontext = new Iocontex;//�Ѿ�����Ĭ�Ϲ����ʼ����ϣ�m_socket=INVAIL_SOCKET
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
					printf("acceptʧ��");
				}
				else
				{
					std::cout << "accept" << i << "�ɹ�" << std::endl;
				}
		}
	}
	
	char quit='0';
	while (quit != 'q')
	{
		std::cout << "q o n" << std::endl;
		std::cin >> quit;
	}
	//�˳�
	for (int i = 0; i < n_maxthread; i++){
		PostQueuedCompletionStatus(IOCP, 0, (DWORD)NULL, NULL);
	}
	WaitForMultipleObjects(n_maxthread, m_thread,
		TRUE, INFINITE);
	//�ͷſռ�
	for (int i = 0; i < n_maxthread; i++)
	{
		RELEASE_HANDLE(m_thread[i]);
	}

	RELEASE_ARRAY(m_thread);
	// �ر�IOCP���
	RELEASE_HANDLE(IOCP);
	// �رռ���Socket
	RELEASE_POINTER(listen_socket_context);
	RELEASE_SOCKET(listen_socket);
	printf("�ͷ����");
}