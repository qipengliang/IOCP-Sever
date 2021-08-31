#include "SERVER.h"
#include "So_context.h"
#include<Winsock2.h>
#include<io.h>
#pragma comment(lib,"Mswsock.lib")



SERVER::SERVER(HANDLE IOCP) :m_lpfnAcceptEx(nullptr), m_lpfnGetAcceptExSockAddrs(nullptr), m_iocp(IOCP)
{
	
}


SERVER::~SERVER()
{
	
}
//工作线程要做的事:
//1.GetQueuedCompletionStatus()判断有无完成端口被激活，若没有，线程阻塞，若有，返回1，且参数中已被赋上相应完成多口的值
//2.读入lpcompeltionKey和lpoverlap以判断哪个socket做了何种IO.
DWORD WINAPI SERVER::workthread(LPVOID parameter)
{
	param* para = (param*)parameter;
	SERVER* iocpmodel=(SERVER*)para->iocpm;
	std::cout << "thread" << para->threadid << std::endl;

	while (TRUE)
	{
		DWORD lpnumberofbytes = 0;
		So_context* pSocontext = nullptr;
		OVERLAPPED* lpoverlapped = nullptr;

		const BOOL bRet = GetQueuedCompletionStatus(iocpmodel->m_iocp, &lpnumberofbytes, (PULONG_PTR)&pSocontext, &lpoverlapped, INFINITE);
		
		if (NULL == (DWORD)pSocontext)
		{
			break;
		}
		Iocontex* piocontext = CONTAINING_RECORD(lpoverlapped, Iocontex, m_overlap);
		iocpmodel->init(piocontext);//得到accpet，和acceptaddre函数的函数指针
		
		switch (piocontext->m_PostType)
		{
			
		case PostType::ACCEPT:
		{   //1.收取第一个数据
								 //2.在pSocontext中的vector中删除piocontext
								 //3.准备新的iocontext，op=accept，将之加入到到vector中.
								 //4.准备新的socket_contex，socket为接收socket,准备新的iocontext，op=receive，到的将之加入到vector中，

								 //5.连接新的socket_contex到iocp，投递iocontext

								 //6.在侦听套接字上投递修改后的piocontext，pSocketcontext由于是指针，故已做更改
								 
								 
								 std::cout << "accept"<< std::endl;
								 iocpmodel->_doaccept(pSocontext, piocontext, lpnumberofbytes);
		}
			break;
		case PostType::RECV:
		{
							   
							   //1.读出相应Buffer中的值，并显示出来
							   //2.重置iocontext,继续投递receive
							   //3.新建iocontext,投递send
							   std::cout << "recv" << std::endl;
							   iocpmodel->_dorecv(pSocontext, piocontext, lpnumberofbytes);
		}
			break;
		case PostType::SEND:
		{
							   iocpmodel->_dosend(pSocontext, piocontext);
		}
			break;
		}
	}
	std::cout << "线程" <<para->threadid<<"结束"<< std::endl;
	RELEASE_POINTER(parameter);
	return 0;
	
}
void SERVER::_doaccept(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes)
{
	//1.收集侦听socket，收集client地址
	SOCKET listensocket = socontext->m_socket;
	SOCKADDR_IN* clientaddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
	m_lpfnGetAcceptExSockAddrs(iocontext->m_wsaBuf.buf, 0, (sizeof(SOCKADDR_IN)+16), (sizeof(SOCKADDR_IN)+16), (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&clientaddr, &remoteLen);

	if (lpnumberofbytes == 0)
	{
		
		std::cout << "未传值,已连接addr:" << inet_ntoa(clientaddr->sin_addr) <<"port:" <<ntohs(clientaddr->sin_port)<<iocontext->m_socket<<std::endl;
	}
	else
	{
		std::cout << "已连接addr:" << inet_ntoa(clientaddr->sin_addr) << ntohs(clientaddr->sin_port) << "data:" << iocontext->m_wsaBuf.buf << std::endl;
	}
	clientadd.push_back(clientaddr);
	//2.
	
	
	//4.
	So_context* Socon_new = new So_context;
	Iocontex* iocon_new = new Iocontex;
	iocon_new->m_socket = iocontext->m_socket;
	iocon_new->m_PostType = PostType::RECV;
	Socon_new->m_socket = iocontext->m_socket;
	Socon_new->array_IoContext.push_back(iocon_new);
	memcpy(&(Socon_new->m_ClientAddr),
		clientaddr, sizeof(SOCKADDR_IN));

	remove(socontext, iocontext);
	//3.
	iocontext = new Iocontex;
	iocontext->m_PostType = PostType::ACCEPT;
	iocontext->m_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	socontext->array_IoContext.push_back(iocontext);

    //5.
	CreateIoCompletionPort((HANDLE)iocon_new->m_socket, m_iocp, (DWORD)Socon_new, 0);
	DWORD dwFlags = 0, dwBytes = 0;
	// 初始化完成后，投递WSARecv请求
	const int nBytesRecv = WSARecv(iocon_new->m_socket,
		&iocon_new->m_wsaBuf, 1, &dwBytes, &dwFlags,
		&iocon_new->m_overlap, NULL);
	_postsend(Socon_new, iocon_new);

	//6.
	DWORD dwAddrLen = (sizeof(SOCKADDR_IN)+16);
	m_lpfnAcceptEx(listensocket, iocontext->m_socket, iocontext->m_wsaBuf.buf, 0, dwAddrLen, dwAddrLen, &dwBytes, &iocontext->m_overlap);
}

void SERVER::_dorecv(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes)
{
	char recv_result[MAX_BUFFERLEN];
	memcpy(recv_result, iocontext->m_wsaBuf.buf, MAX_BUFFERLEN);
	if (lpnumberofbytes==0)
	{
		std::cout << "关闭" << std::endl;		
		closesocket(socontext->m_socket);
		RELEASE_SOCKET(socontext->m_socket);
	}
	recv_result[lpnumberofbytes] = '/0';
	std::cout << "receive:" << recv_result << std::endl;
	
	ZeroMemory(iocontext->m_buffer, sizeof(iocontext->m_buffer));
	iocontext->m_wsaBuf.len = MAX_BUFFERLEN;
	iocontext->m_wsaBuf.buf = iocontext->m_buffer;
	DWORD dwFlags = 0, dwBytes = 0;
	const int nBytesRecv = WSARecv(iocontext->m_socket,
		&iocontext->m_wsaBuf, 1, &dwBytes, &dwFlags,
		&iocontext->m_overlap, NULL);
	if (recv_result[lpnumberofbytes] == '/0')
		_postsend(socontext, iocontext);

}

void SERVER::_postsend(So_context* socontext, Iocontex* iocontext)
{
	
	char OKHeaderFormat[] =
		"HTTP/1.1 200 OK\n"
		"Accept-Ranges: bytes\n"
		"Connection:Keep-Alive\n"
		"Content-Type: text/html\n"
		"charset = ISO-8859-1\n"
		"Content-Length: %d\n"
		"\r\n";
	Iocontex* iocon_new_head=new Iocontex;
	/*std::string report = "HTTP/1.1 200 ok\r\nconnection: close\r\n\r\n";
	memcpy(iocon_new->m_buffer, report.c_str(), sizeof(report));
	iocon_new->m_wsaBuf.len = sizeof(report);
	iocon_new->m_wsaBuf.buf = iocon_new->m_buffer;*/
	iocon_new_head->m_PostType = PostType::SEND;
	iocon_new_head->m_socket = iocontext->m_socket;
	memcpy(iocon_new_head->m_buffer, OKHeaderFormat, sizeof(OKHeaderFormat));
	iocon_new_head->m_wsaBuf.len = sizeof(OKHeaderFormat);
	iocon_new_head->m_wsaBuf.buf = iocon_new_head->m_buffer;

	socontext->array_IoContext.push_back(iocon_new_head);
	const DWORD dwFlags = 0;
	DWORD dwSendNumBytes = 0;
	const int nRet = WSASend(iocon_new_head->m_socket,
		&iocon_new_head->m_wsaBuf, 1, &dwSendNumBytes, dwFlags,
		&iocon_new_head->m_overlap, NULL);

	Iocontex* iocon_new_body = new Iocontex;
	/*std::string report = "HTTP/1.1 200 ok\r\nconnection: close\r\n\r\n";
	memcpy(iocon_new->m_buffer, report.c_str(), sizeof(report));
	iocon_new->m_wsaBuf.len = sizeof(report);
	iocon_new->m_wsaBuf.buf = iocon_new->m_buffer;*/
	iocon_new_body->m_PostType = PostType::SEND;
	iocon_new_body->m_socket = iocontext->m_socket;
	std::string filename = "C:\\Users\\ncslab\\Desktop\\C++ test\\多人聊天室\\多人聊天室\\server\\hellow.html";
	FILE* pFile = NULL;
	fopen_s(&pFile, filename.c_str(), "r");
	if (pFile == NULL)
	{
		std::cout << "open file error";
	}
	fseek(pFile, 0, SEEK_END);//move fseek to EOF
	int bufferlength = ftell(pFile); //This is the length of file
	iocon_new_body->m_buffer[bufferlength] = '\0';
	bufferlength++; // place for'\0'
	fseek(pFile, 0, SEEK_SET); //move back

	fread(iocon_new_body->m_buffer, bufferlength, 1, pFile); //read file to buff
	fclose(pFile);
	
	//memcpy(iocon_new_body->m_buffer, OKHeaderFormat, sizeof(OKHeaderFormat));
	iocon_new_body->m_wsaBuf.len = bufferlength;
	iocon_new_body->m_wsaBuf.buf = iocon_new_body->m_buffer;
	socontext->array_IoContext.push_back(iocon_new_body);
	const DWORD dwFlags_1 = 0;
	DWORD dwSendNumBytes_1 = 0;
	const int nRet_1 = WSASend(iocon_new_body->m_socket,
		&iocon_new_body->m_wsaBuf, 1, &dwSendNumBytes_1, dwFlags_1,
		&iocon_new_body->m_overlap, NULL);
}

void SERVER::_dosend(So_context* socontext, Iocontex* iocontext)
{
	std::cout << "已回复收到" << std::endl;
	remove(socontext, iocontext);
}


void SERVER::remove(So_context* socontext, Iocontex* iocontext)
{
	std::vector<Iocontex*>::iterator it = socontext->array_IoContext.begin();
	while (iocontext != *it)
	{
		it=it+1;		
	}
	socontext->array_IoContext.erase(it);
	delete iocontext;//.....................释放掉之前new的空间，很重要
	iocontext = nullptr;
}

void SERVER::init(Iocontex* iocontext)
{
	DWORD dwBytes = 0;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	WSAIoctl(iocontext->m_socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), &m_lpfnGetAcceptExSockAddrs,
		sizeof(m_lpfnGetAcceptExSockAddrs), &dwBytes, NULL, NULL);
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	WSAIoctl(iocontext->m_socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx,
		sizeof(GuidAcceptEx), &m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx), &dwBytes, NULL, NULL);
}

