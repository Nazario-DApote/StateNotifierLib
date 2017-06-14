#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__


/*!
 * \file   TcpClient.h
 * \Author S. Boschi
 *
 * Questo file contiene la definizione della classe TcpClient.
 */

#include <boost/noncopyable.hpp>
#include <CCommunication.h>
#include <ClientSocket.h>

#define INVALID_PORT_NUMBER 0
#define SOCKET_OPEN_TIMEOUT		1000 //msec
#define SOCKET_READ_TIMEOUT		45000 //msec

/*!
 * \class   TcpClient
 *
 * Si occupa gestire le comunicazioni di rete.
 *
 */
class TcpClient: 
	public CClientSocket, 
	protected boost::noncopyable
{

	public:
		TcpClient();
		virtual ~TcpClient();

		bool open(const char *sIpAddr, const unsigned int& uiPort);
		bool isConnect();
		bool send(const char *pBuff, const unsigned int& uiBuffSize);
		bool recv(char *pBuff, const unsigned int& uiBuffSize, unsigned int& uiBytesRead);

		static void writeInt32LittleEndian(byte *pBuffer, int iValue);
		static int extractInt32LittleEndian(byte *pBuffer);

};

#endif /* __TCP_CLIENT_H__ */