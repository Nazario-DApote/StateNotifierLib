/**
 * @file   TcpClient.cpp
 * @Author S. Boschi
 *
 * Questo file contiene l'implementazione dei metodi della classe TcpClient.
 *
 */

#include "TcpClient.h"

#include "SocketLib_defs.h"

#include "CB2Enabler_cfg.h"
#include "LogMgr.h"


#define SOCK_MODE_READ		0x0000
#define SOCK_MODE_WRITE		0x0001
#define SOCK_MODE_WR		0x0002

#define OFFSET_MODE_READ	0
#define OFFSET_MODE_WRITE	1
#define OFFSET_MODE_WR		2


/*!
 * \fn  TcpClient
 *
 * Costruttore della classe TcpClient.
 *
 */
TcpClient::TcpClient()
{

}//constructor

/*!
 * \fn  ~TcpClient  
 *
 * Distruttore della classe TcpClient.
 *
 */
TcpClient::~TcpClient()
{
}//destructor



/*!
 * \fn  open
 *
 * Apre una connessione TCP.
 *
 * \returns  TRUE se l'operazione e' andata a buon fine, FALSE altrimenti.
 */
bool TcpClient::open(const char *sIpAddr, const unsigned int& uiPort)
{
    bool bRet = false;

    if((sIpAddr == nullptr) || (uiPort == INVALID_PORT_NUMBER))
    {
        LogMgr::Error("[TcpClient::open] Invalid IP adddress or TCP port!");
        bRet = false;
    }
    else
    {
        bRet = ( Open(sIpAddr, SOCK_MODE_WR, uiPort - OFFSET_MODE_WR, false, SOCKET_OPEN_TIMEOUT) == TRUE );
    }


    return bRet;
}//open


/*!
 * \fn  isConnect
 *
 * Controlla se e' la connessione con l'host e' aperta.
 *
 * \returns  TRUE se e' la connessione con l'host e' aperta, FALSE altrimenti.
 */
bool TcpClient::isConnect()
{
    return m_bIsCommunicationEstablished > 0;
}//isConnect


/*!
 * \fn  send
 *
 * Invia un buffer di byte all'host connesso.
 *
 * \returns  TRUE se l'operazione e' andata a buon fine, FALSE altrimenti.
 */
bool TcpClient::send(const char *pBuff, const unsigned int& uiBuffSize)
{
    bool bRet = false;
    int iRet;


    if(pBuff == nullptr)
    {
        LogMgr::Error("[TcpClient::send] Invalid message!");
        return false;
    }

    iRet = Write(pBuff, uiBuffSize);

    if(iRet > 0)
    {
        bRet = true;
    }
    else if(iRet == SOCKERR_NOTCONN)
    {
        LogMgr::Error("[TcpClient::send] Socket not connect!");
    }
    else if(iRet == SOCKERR_OTHER)
    {
        LogMgr::Error("[TcpClient::send] Internal socket error!");
    }
    else
    {
        LogMgr::Error("[TcpClient::send] Unknown socket error!");
    }



    return bRet;
}//send


/*!
 * \fn  recv
 *
 * Riceve un buffer di byte dall'host connesso.
 *
 * \returns  TRUE se l'operazione e' andata a buon fine, FALSE altrimenti.
 *
 */
bool TcpClient::recv(char *pBuff, const unsigned int& uiBuffSize, unsigned int& uiBytesRead)
{
    bool bRet = false;
    int iRet;

    uiBytesRead = 0;

    if(pBuff == nullptr)
    {
        LogMgr::Error("[TcpClient::send] Invalid message!");
        return false;
    }

    iRet = Read(pBuff, uiBuffSize, SOCKET_READ_TIMEOUT);


    if(iRet > 0)
    {
        uiBytesRead = iRet;
        bRet = (iRet <= (int)uiBuffSize);
    }
    else if(iRet == SOCKERR_NOTCONN)
    {
        LogMgr::Error("[TcpClient::recv] Socket not connect!");
    }
    else if(iRet == SOCKERR_OTHER)
    {
        LogMgr::Error("[TcpClient::recv] Internal socket error!");
    }
    else if(iRet == 0)//TIMEOUT
    {
        LogMgr::Error("[TcpClient::recv] Timeout expired!");
    }
    else
    {
        LogMgr::Error("[TcpClient::recv] Unknown socket error!");
    }

    return bRet;
}//recv







/*!
 * \fn  writeInt32LittleEndian
 *
 * Converte intero a 32 bit in un buffer di byte con rappresentazione little-endian.
 *
 */
void TcpClient::writeInt32LittleEndian(byte *pBuffer, int iValue)
{
    const int INT32_SIZE = 4;

    int i = 0;

    for (i = INT32_SIZE - 1; i >= 0; --i)
    {
        pBuffer[i] = (byte)(iValue & 0xFF);
        iValue >>= 8;
    }
}


/*!
 * \fn  extractInt32LittleEndian
 *
 * Converte un buffer di byte con rappresentazione little-endian in un intero a 32 bit.
 *
 * \returns  Un intero a 32 con rappresantazione little-endian.
 * 
 */
int TcpClient::extractInt32LittleEndian(byte *pBuffer)
{
    const int INT32_SIZE = 4;
    int iRet = 0;
    int i = 0;

    for (i = 0; i < INT32_SIZE; ++i)
    {
        iRet <<= 8;
        iRet += pBuffer[i];
    }

    return iRet;
}
