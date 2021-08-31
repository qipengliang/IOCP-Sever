#pragma once
#include <WinSock2.h>
#include <MSWSock.h>
#include <WinNt.h>
#define MAX_BUFFERLEN (1024*16)
enum class PostType
{
	UNKNOWN, // 用于初始化，无意义
	ACCEPT, // 标志投递的Accept操作
	SEND, // 标志投递的是发送操作
	RECV, // 标志投递的是接收操作
};

class Iocontex
{
public:
	OVERLAPPED m_overlap;
	SOCKET m_socket;
	PostType m_PostType;
	WSABUF  m_wsaBuf;

	char m_buffer[MAX_BUFFERLEN];
	Iocontex();
	~Iocontex();
};


