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
//�����߳�Ҫ������:
//1.GetQueuedCompletionStatus()�ж�������ɶ˿ڱ������û�У��߳����������У�����1���Ҳ������ѱ�������Ӧ��ɶ�ڵ�ֵ
//2.����lpcompeltionKey��lpoverlap���ж��ĸ�socket���˺���IO.
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
		iocpmodel->init(piocontext);//�õ�accpet����acceptaddre�����ĺ���ָ��
		
		switch (piocontext->m_PostType)
		{
			
		case PostType::ACCEPT:
		{   //1.��ȡ��һ������
								 //2.��pSocontext�е�vector��ɾ��piocontext
								 //3.׼���µ�iocontext��op=accept����֮���뵽��vector��.
								 //4.׼���µ�socket_contex��socketΪ����socket,׼���µ�iocontext��op=receive�����Ľ�֮���뵽vector�У�

								 //5.�����µ�socket_contex��iocp��Ͷ��iocontext

								 //6.�������׽�����Ͷ���޸ĺ��piocontext��pSocketcontext������ָ�룬����������
								 
								 
								 std::cout << "accept"<< std::endl;
								 iocpmodel->_doaccept(pSocontext, piocontext, lpnumberofbytes);
		}
			break;
		case PostType::RECV:
		{
							   
							   //1.������ӦBuffer�е�ֵ������ʾ����
							   //2.����iocontext,����Ͷ��receive
							   //3.�½�iocontext,Ͷ��send
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
	std::cout << "�߳�" <<para->threadid<<"����"<< std::endl;
	RELEASE_POINTER(parameter);
	return 0;
	
}
void SERVER::_doaccept(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes)
{
	//1.�ռ�����socket���ռ�client��ַ
	SOCKET listensocket = socontext->m_socket;
	SOCKADDR_IN* clientaddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
	m_lpfnGetAcceptExSockAddrs(iocontext->m_wsaBuf.buf, 0, (sizeof(SOCKADDR_IN)+16), (sizeof(SOCKADDR_IN)+16), (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&clientaddr, &remoteLen);

	if (lpnumberofbytes == 0)
	{
		
		std::cout << "δ��ֵ,������addr:" << inet_ntoa(clientaddr->sin_addr) <<"port:" <<ntohs(clientaddr->sin_port)<<iocontext->m_socket<<std::endl;
	}
	else
	{
		std::cout << "������addr:" << inet_ntoa(clientaddr->sin_addr) << ntohs(clientaddr->sin_port) << "data:" << iocontext->m_wsaBuf.buf << std::endl;
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
	// ��ʼ����ɺ�Ͷ��WSARecv����
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
		std::cout << "�ر�" << std::endl;		
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
	std::string filename = "C:\\Users\\ncslab\\Desktop\\C++ test\\����������\\����������\\server\\hellow.html";
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
	std::cout << "�ѻظ��յ�" << std::endl;
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
	delete iocontext;//.....................�ͷŵ�֮ǰnew�Ŀռ䣬����Ҫ
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

