#pragma once
#include <WinSock2.h>
#include <MSWSock.h>
#include <WinNt.h>
#define MAX_BUFFERLEN (1024*16)
enum class PostType
{
	UNKNOWN, // ���ڳ�ʼ����������
	ACCEPT, // ��־Ͷ�ݵ�Accept����
	SEND, // ��־Ͷ�ݵ��Ƿ��Ͳ���
	RECV, // ��־Ͷ�ݵ��ǽ��ղ���
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


