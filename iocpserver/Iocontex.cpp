#include "Iocontex.h"


Iocontex::Iocontex()
{
	dir = "";
	m_socket = INVALID_SOCKET;
	ZeroMemory(&m_overlap, sizeof(m_overlap));
	m_PostType = PostType::UNKNOWN;
	ZeroMemory(m_buffer, sizeof(m_buffer));
	m_wsaBuf.len = MAX_BUFFERLEN;
	m_wsaBuf.buf = m_buffer;
}


Iocontex::~Iocontex()
{
}
