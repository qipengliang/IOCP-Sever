#pragma once
#pragma comment (lib,"ws2_32.lib")

#include<winsock2.h>
#include<iostream>
#include<vector>
#include"Iocontex.h"
class So_context
{
	public:
		SOCKET m_socket;
		SOCKADDR_IN  m_ClientAddr;
		std::vector<Iocontex*>  array_IoContext;
		So_context();
		~So_context();

};

