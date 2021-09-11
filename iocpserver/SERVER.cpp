#include "SERVER.h"
#include "So_context.h"
#include<Winsock2.h>
#include<io.h>
#pragma comment(lib,"Mswsock.lib")



SERVER::SERVER(HANDLE IOCP, MysqlConn mysql,TPool* threadpool) :m_lpfnAcceptEx(nullptr), m_lpfnGetAcceptExSockAddrs(nullptr), m_iocp(IOCP), s_mysql(mysql), s_threadp(threadpool)
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
	//接收参数
	param* para = (param*)parameter;
	SERVER* iocpmodel=(SERVER*)para->iocpm;
	std::cout << "thread" << para->threadid << std::endl;

	while (TRUE)
	{
		DWORD lpnumberofbytes = 0;
		So_context* pSocontext = nullptr;
		OVERLAPPED* lpoverlapped = nullptr;
		//等待事件发生，若无事件发生则线程阻塞
		const BOOL bRet = GetQueuedCompletionStatus(iocpmodel->m_iocp, &lpnumberofbytes, (PULONG_PTR)&pSocontext, &lpoverlapped, INFINITE);
		//判断线程结束信号
		if (NULL == (PULONG_PTR)pSocontext)
		{
			break;
		}
		Iocontex* piocontext = CONTAINING_RECORD(lpoverlapped, Iocontex, m_overlap);
		//iocpmodel->init(piocontext);//得到accpet，和acceptaddre函数的函数指针
		
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
								 
								 iocpmodel->_doaccept(pSocontext, piocontext, lpnumberofbytes, para->threadid);
		}
			break;
		case PostType::RECV:
		{				   
							   //1.读出相应Buffer中的值，并显示出来
							   //2.新建iocontext,投递send
							   std::cout << "recv" << std::endl;
							   iocpmodel->_dorecv(pSocontext, piocontext, lpnumberofbytes, para->threadid);
		}
			break;
		case PostType::SEND:
		{					   std::cout << "send" << pSocontext->m_socket<< std::endl;
							   iocpmodel->_dosend(pSocontext, piocontext, para->threadid);
		}
			break;
		case PostType::SENDHTML:
		{						std::cout << "sendhtml" << pSocontext->m_socket<<std::endl;
								iocpmodel->_dosendhtml(pSocontext, piocontext, para->threadid);
		}
			break;
		}
	}
	std::cout << "线程" <<para->threadid<<"结束"<< std::endl;
	RELEASE_POINTER(parameter);
	return 0;
	
}
void SERVER::_doaccept(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes, DWORD threadn)
{
	//1.收集侦听socket，收集client地址
	SOCKET listensocket = socontext->m_socket;
	SOCKADDR_IN* clientaddr = NULL;
	SOCKADDR_IN* LocalAddr = NULL;
	int remoteLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
	m_lpfnGetAcceptExSockAddrs(iocontext->m_wsaBuf.buf, 0, (sizeof(SOCKADDR_IN)+16), (sizeof(SOCKADDR_IN)+16), (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&clientaddr, &remoteLen);

	if (lpnumberofbytes == 0)
	{	
		std::cout << "未传值,已连接addr:" << threadn <<"port:" <<ntohs(clientaddr->sin_port)<<"socket:"<<iocontext->m_socket<<std::endl;
	}
	else
	{
		std::cout << "已连接addr:" << inet_ntoa(clientaddr->sin_addr) << ntohs(clientaddr->sin_port) << "data:" << iocontext->m_wsaBuf.buf << std::endl;
	}
	
	//2.
	
	//4.
	So_context* Socon_new = new So_context;
	Iocontex* iocon_new = new Iocontex;
	iocon_new->m_socket = iocontext->m_socket;
	iocon_new->m_PostType = PostType::RECV;
	Socon_new->m_socket = iocontext->m_socket;
	Socon_new->array_IoContext.push_back(iocon_new);
	memcpy(&(Socon_new->m_ClientAddr),clientaddr, remoteLen);
	std::cout << "cedian1" << std::endl;
	EnterCriticalSection(&Cri_list);
	solist.push_back(Socon_new);
	LeaveCriticalSection(&Cri_list);

	//remove(socontext, iocontext);
	//3.
	ZeroMemory(iocontext->m_buffer, sizeof(iocontext->m_buffer));
	iocontext->m_wsaBuf.buf = iocontext->m_buffer;
	iocontext->m_PostType = PostType::ACCEPT;
	iocontext->m_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	socontext->array_IoContext.push_back(iocontext);
	

    //5.
	CreateIoCompletionPort((HANDLE)iocon_new->m_socket, m_iocp, (ULONG_PTR)Socon_new, 0);
	DWORD dwFlags = 0, dwBytes = 0;
	// 初始化完成后，投递WSARecv请求
	const int nBytesRecv = WSARecv(iocon_new->m_socket,
		&iocon_new->m_wsaBuf, 1, &dwBytes, &dwFlags,
		&iocon_new->m_overlap, NULL);
	Socon_new->array_IoContext.push_back(iocon_new);
	//_postsend(Socon_new, iocon_new);

	//6.
	DWORD dwAddrLen = (sizeof(SOCKADDR_IN)+16);
	m_lpfnAcceptEx(listensocket, iocontext->m_socket, iocontext->m_wsaBuf.buf, 0, dwAddrLen, dwAddrLen, &dwBytes, &iocontext->m_overlap);


}

void SERVER::_dorecv(So_context* socontext, Iocontex* iocontext, DWORD lpnumberofbytes,DWORD threadn)
{
	char recv_result[1024];
	memcpy(recv_result, iocontext->m_wsaBuf.buf, 1024);
	if (lpnumberofbytes==0)
	{
		std::cout << "关闭" << socontext->m_socket << std::endl;
		RELEASE_SOCKET(socontext->m_socket);
	}
	recv_result[lpnumberofbytes] = '\0';
	std::cout << "receive:" << recv_result  << "thread:" << iocontext->m_socket << std::endl;
	//解析请求，投递对应的数据
	std::string req(recv_result);
	std::string method = req.substr(0, 3);
	std::string Head;
	if (method == "GET")
	{
		auto indend = req.find("HTTP");
		std::string dir=req.substr(4,indend-(4));
		std::cout << dir << std::endl;
		if (dir == "/ ")
		{
			Head = "HTTP/1.1 200 OK\r\nContent-Type:text/html;charset = UTF-8\r\n\r\n";
			dir = "C:\\Users\\ncslab\\Desktop\\C++ test\\多人聊天室\\多人聊天室\\server\\hellow.html";
			_postsend(socontext, iocontext, threadn, Head,dir);
		}
		else if (dir.find('/?') == 1)
		{
			auto split = dir.find('&');
			std::string username = dir.substr(11,split-11);
			std::string password = dir.substr(split + 10);
			std::cout << "用户名=" << username << "\n密码=" << password << std::endl;
			Head = "HTTP/1.1 200 OK\r\nContent-Type:text/html;charset = UTF-8\r\n\r\n";
			dir = "C:\\Users\\ncslab\\Desktop\\C++ test\\多人聊天室\\多人聊天室\\server\\acc.html";
			_postsend(socontext, iocontext, threadn, Head, dir);
           //调用线程池，建立线程任务，将usernme和password写入数据库
		   //由于本人不会前端，所以假设服务器需要分析数据内容，并决定存放的表和执行的query。实际上，这部分工作可由前端完成，服务器只调用前端传回的query即可。
			std::string query = "call namepassword.user_name_new('" + username + "', '" + password + "')";
			creatmysqltask(query);
		}
		else
		{
			Head = "HTTP/1.1 200 OK\r\nContent-Type:image\\jpeg;charset = UTF-8\r\n\r\n";
			analyze(dir);
			dir = "C:\\Users\\ncslab\\Desktop\\C++ test\\多人聊天室\\多人聊天室\\server"+dir;
			std::cout << dir << std::endl;
			_postsend(socontext, iocontext, threadn,Head, dir);
		}
	}

}

BOOL SERVER::_postsend(So_context* socontext, Iocontex* iocontext, DWORD threadn ,const std::string& OKHeaderFormat,std::string dir)
{
	//投递发送http协议头
	SOCKET recvsoc=iocontext->m_socket;
	remove(socontext, iocontext);
	std::cout << OKHeaderFormat << std::endl;
	Iocontex* iocon_new_head=new Iocontex;
	iocon_new_head->dir = dir;
	iocon_new_head->m_PostType = PostType::SEND;
	iocon_new_head->m_socket = recvsoc;
	memcpy(iocon_new_head->m_buffer, OKHeaderFormat.c_str(), OKHeaderFormat.length());
	iocon_new_head->m_wsaBuf.len = OKHeaderFormat.length();
	iocon_new_head->m_wsaBuf.buf = iocon_new_head->m_buffer;
	socontext->array_IoContext.push_back(iocon_new_head);
	const DWORD dwFlags = 0;
	DWORD dwSendNumBytes = 0;
	const int nRet = WSASend(iocon_new_head->m_socket,
			&iocon_new_head->m_wsaBuf, 1, &dwSendNumBytes, dwFlags,
			&iocon_new_head->m_overlap, NULL);
	std::cout << "post head" << recvsoc << std::endl;
	return 0;
}

BOOL SERVER::_postsendbody(So_context* socontext, Iocontex* iocontext, DWORD threadn)
{
	//投递发送http协议头
	SOCKET recvsoc = iocontext->m_socket;
	std::string dir = iocontext->dir;
	remove(socontext, iocontext);
	//打开文件，投递发送数据
	Iocontex* iocon_new_body = new Iocontex;
	iocon_new_body->m_PostType = PostType::SENDHTML;
	iocon_new_body->m_socket = recvsoc;
	std::string filename = dir;
	FILE* pFile = NULL;
	fopen_s(&pFile, filename.c_str(), "rb");
	if (pFile == NULL)
	{
		std::cout << "open file error" << recvsoc;
		return 0;
	}
	fseek(pFile, 0, SEEK_END);//move fseek to EOF
	int bufferlength = ftell(pFile); //This is the length of file
	iocon_new_body->m_buffer[bufferlength] = '\0';
	std::cout << "打开文件" << std::endl;
	bufferlength++; // place for'\0'
	fseek(pFile, 0, SEEK_SET); //move back
	std::cout << "bufferlength" << bufferlength << std::endl;
	fread(iocon_new_body->m_buffer, bufferlength, 1, pFile); //read file to buff
	fclose(pFile);

	iocon_new_body->m_wsaBuf.len = bufferlength;
	iocon_new_body->m_wsaBuf.buf = iocon_new_body->m_buffer;
	socontext->array_IoContext.push_back(iocon_new_body);
	const DWORD dwFlags_1 = 0;
	DWORD dwSendNumBytes_1 = 0;
	const int nRet_1 = WSASend(iocon_new_body->m_socket,
		&iocon_new_body->m_wsaBuf, 1, &dwSendNumBytes_1, dwFlags_1,
		&iocon_new_body->m_overlap, NULL);
	std::cout << "sb" << dwSendNumBytes_1 << std::endl;
	if (SOCKET_ERROR == nRet_1)
	{
		int nErr = WSAGetLastError();
		if (WSA_IO_PENDING != nErr)
		{
			std::cout << "投递WSASend失败！err=%d" << std::endl;
		}
	}
	return 1;
}

void SERVER::_dosend(So_context* socontext, Iocontex* iocontext, DWORD threadn)
{
	
	std::cout << "already send head " << std::endl;
	_postsendbody(socontext, iocontext, threadn);
	
}

void SERVER::_dosendhtml(So_context* socontext, Iocontex* iocontext, DWORD threadn)
{

	std::cout << "close" << iocontext->m_socket<< std::endl;
	closesocket(iocontext->m_socket);
	remove(socontext, iocontext);
	//移除socontext
	removeso(socontext);

	
}

bool SERVER::remove(So_context* socontext, Iocontex* iocontext)
{
		if (iocontext == nullptr)
		{
			return 0;
		}
		std::vector<Iocontex*>::iterator it;
		it = socontext->array_IoContext.begin();
		while (iocontext != *it )
		{
			it = it + 1;
			if (it == socontext->array_IoContext.end())
			{
				if (iocontext != *it)
					return 0;
				else
					break;
			}
				
		}
		socontext->array_IoContext.erase(it);
		delete iocontext;//.....................释放掉之前new的空间，很重要
		iocontext = nullptr;
		std::cout << "remove io " << std::endl;
		return 0;
}

void SERVER::init(SOCKET sock)
{
	DWORD dwBytes = 0;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	WSAIoctl(sock,
		SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs), &m_lpfnGetAcceptExSockAddrs,
		sizeof(m_lpfnGetAcceptExSockAddrs), &dwBytes, NULL, NULL);
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	WSAIoctl(sock,
		SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx,
		sizeof(GuidAcceptEx), &m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx), &dwBytes, NULL, NULL);
}

void SERVER::analyze(std::string& request)
{
	int ind = 0;
	while (ind != -1)
	{
		int indl = ind;
		ind=request.find('/',ind);
		if (ind != -1)
		{
			request.erase(ind, 1);
			request.insert(ind, "\\");    
			//std::cout << request << std::endl;
		}	
	}
}

bool SERVER::removeso(So_context* socontext)
{
	EnterCriticalSection(&Cri_list);
	std::vector<So_context*>::iterator it = solist.begin();
	if (socontext == nullptr)
	{
		LeaveCriticalSection(&Cri_list);
		return 0;
	}
	while (socontext != *it)
	{
		it = it + 1;
		if (it == solist.end())
		{
			if (socontext != *it)
			{
				LeaveCriticalSection(&Cri_list);
				return 0;
			}
			else
				break;
		}
	}
	std::cout << "removeso" << solist.size()<<std::endl;
	solist.erase(it);
	delete socontext;//.....................释放掉之前new的空间，很重要
	socontext = nullptr;
	LeaveCriticalSection(&Cri_list);
	return 0;
}

void SERVER::creatmysqltask(std::string query)
{
	s_threadp->task_ennqueue(MysqlConn::Query,&s_mysql.mysql, query);
}


