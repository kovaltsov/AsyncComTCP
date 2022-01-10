#include "pch.h"
#include "common.h"

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>  

//Processing socket network events
threadStatus socketDataEventsHandler(
    SOCKET ConnectSocket, 
    CThreadConfig* deviceParams, 
    SockedBuf* rcvBuf, 
    HANDLE hDataEvent)
{
    CRingBuffer* TcpToComBuf = &deviceParams->m_TcpToComBuf;
    int rez;
    WSANETWORKEVENTS wsaProcessEvent;
    //while (1)
    //{

    ::WSAEnumNetworkEvents(ConnectSocket, hDataEvent, &wsaProcessEvent);
    if (wsaProcessEvent.lNetworkEvents & FD_READ)
    {
        DBG("TCP Handle FD_READ");
        memset(rcvBuf->buf, 0, TCP_BUF_SIZE);
        rcvBuf->inBuf = recv(ConnectSocket, (char*)rcvBuf->buf, TCP_BUF_SIZE, 0);
        TcpToComBuf->Write((byte*)rcvBuf->buf, rcvBuf->inBuf);
        DBG("TCP len = " << (int)rcvBuf->inBuf << "rcv: " << (char*)rcvBuf->buf);
        SetEvent(deviceParams->m_TcpRcvEvent);
    }
    if (wsaProcessEvent.lNetworkEvents & FD_CONNECT)
    {
        DBG("TCP Handle FD_CONNECT");
        rez = wsaProcessEvent.iErrorCode[FD_CONNECT_BIT];
        if (rez == 0)
        {
            deviceParams->m_portSetting->setStatus(PortStatus::Connected);
            LOG("Connected to server.\n");
        }
        else
        {
            ERR("connect function failed with error: " << rez);
            return threadStatus::THREAD_RECONNECT;
        }
    }
    if (wsaProcessEvent.lNetworkEvents & FD_WRITE)
    {
        DBG("TCP Handle FD_WRITE\n");
        if (deviceParams->m_NeedResend)
        {
            deviceParams->m_NeedResend = false;
            SetEvent(deviceParams->m_ComRcvEvent);
        }
    }
    if (wsaProcessEvent.lNetworkEvents & FD_CLOSE)
    {
        DBG("TCP Handle FD_CLOSE\n");
        return threadStatus::THREAD_RECONNECT;
    }
    return threadStatus::THREAD_CONTINUE;
}

//Processing an event to write data from a COM port to TCP
threadStatus writeComToTcp(
    SOCKET ConnectSocket,
    CThreadConfig* deviceParams,
    SockedBuf* sendBuf
)
{
    int rez;
    CRingBuffer* ComToTcpBuf = &deviceParams->m_ComToTcpBuf;
    SIZE_T bytesSend;
    SIZE_T availableData = 0;
    //ResetEvent(deviceParams->m_ComRcvEvent);
    do
    {
        ComToTcpBuf->Read((byte*)sendBuf->buf, TCP_BUF_SIZE, &sendBuf->inBuf);
        bytesSend = 0;
        if (sendBuf->inBuf > 0)
        {
            while (bytesSend != sendBuf->inBuf)
            {
                rez = send(ConnectSocket, (char*)sendBuf->buf + bytesSend, (int)sendBuf->inBuf - (int)bytesSend, 0);
                if (rez > 0)
                {
                    bytesSend += rez;
                    DBG("Write to socket. len = " << (ULONG)sendBuf->inBuf << " rez = " << rez);
                }
                else if (rez == SOCKET_ERROR)
                {
                    rez = GetLastError();
                    if (rez == WSAEWOULDBLOCK)
                    {
                        ERR("Socket send error GetLastError: WSAEWOULDBLOCK. Wait FD_WRITE");
                        //SetEvent(deviceParams->m_ComRcvEvent);
                        ComToTcpBuf->CancelReadBytes(sendBuf->inBuf - bytesSend);
                        deviceParams->m_NeedResend = true;
                        return threadStatus::THREAD_CONTINUE;
                    }
                    else
                    {
                        ERR("Socket send error GetLastError: " << rez);
                        return threadStatus::THREAD_RECONNECT;
                    }
                }
            }
        }
        else
        {
            ERR("ComToTcpBuf read data error");
        }
        ComToTcpBuf->GetAvailableData(&availableData);
    } while (availableData > 0);//If there is still something left in the buffer, we repeat the sending
    return threadStatus::THREAD_CONTINUE;
}

DWORD WINAPI comDataReadProc(
    _In_ LPVOID lpParameter
)
{
    CThreadConfig* deviceParams = (CThreadConfig*)lpParameter;
    CRingBuffer* ComToTcpBuf = &deviceParams->m_ComToTcpBuf;

    HANDLE COMport = deviceParams->m_hComPort;

    SockedBuf readBuf;

    COMSTAT comstat; //structure of the current state of the port
    DWORD btr, temp, mask = 0;
    int err;
    OVERLAPPED* overlapped = &deviceParams->m_overlappedRd;
    BOOL rez;

    const int waitCount = 2;
    WSAEVENT hDataEvent[waitCount];// = WSA_INVALID_EVENT;
    hDataEvent[0] = overlapped->hEvent;
    hDataEvent[1] = deviceParams->m_CloseEvent;
    DWORD wait;

    while (1) //until the thread is interrupted, we run a loop
    {
        //set the mask to trigger on the event of receiving a byte into the port
        rez = SetCommMask(COMport, EV_RXCHAR/* | EV_RXFLAG | EV_CTS | EV_DSR | EV_RLSD | EV_BREAK | EV_ERR | EV_RING | EV_TXEMPTY*/);
        if (!rez)
        {
            ERR("SetCommMask error GetLastError: " << GetLastError());
            return 0;
        }
        mask = 0;
        overlapped->Internal = 0;
        overlapped->InternalHigh = 0;
        overlapped->Offset = 0;
        overlapped->OffsetHigh = 0;
        rez = WaitCommEvent(COMport, &mask, overlapped);

        wait = WaitForMultipleObjects(waitCount, hDataEvent, false, INFINITE);
        if (wait == WAIT_OBJECT_0)
        {
            if (GetOverlappedResult(COMport, overlapped, &temp, true)) //completed successfully?
            {
                // overlapping operation WaitCommEvent
                if ((mask & EV_RXCHAR) != 0)
                {
                    memset(readBuf.buf, 0, TCP_BUF_SIZE);
                    ClearCommError(COMport, &temp, &comstat); //fill COMSTAT
                    btr = comstat.cbInQue; //get number of bytes received
                    if (btr)
                    {
                        rez = ReadFile(COMport, readBuf.buf, btr, NULL, overlapped);
                        if (rez == false)
                        {
                            err = GetLastError();
                            if (err != ERROR_IO_PENDING)
                            {
                                ERR("ReadFile error GetLastError: " << err);
                                continue;
                            }
                        }
                        if (GetOverlappedResult(COMport, overlapped, &readBuf.inBuf, true))
                        {
                            DBG("COM rcv " << readBuf.inBuf << " bytes msg: " << readBuf.buf);
                            ComToTcpBuf->Write(readBuf.buf, readBuf.inBuf);
                            SetEvent(deviceParams->m_ComRcvEvent);
                        }
                        else
                        {
                            err = GetLastError();
                            ERR("m_overlappedRd GetOverlappedResult error GetLastError: " << err);
                            continue;
                        }
                    }
                }
            }
        }
        //Close event
        if (wait == WAIT_OBJECT_0 + 1)
        {
            return 1;
        }
    }
    return 1;
}

HRESULT writeTcpToCom(
    CThreadConfig* deviceParams, 
    SockedBuf* sendBuf)
{
    CRingBuffer* TcpToComBuf = &deviceParams->m_TcpToComBuf;
    SIZE_T availableData;
    SIZE_T bytesSend, lpNumberOfBytesWritten;
    BOOL rez;
    int err;

    OVERLAPPED* overlapped = &deviceParams->m_overlappedWr;
    do
    {
        TcpToComBuf->Read((byte*)sendBuf->buf, TCP_BUF_SIZE, &sendBuf->inBuf);
        bytesSend = 0;
        if (sendBuf->inBuf > 0)
        {
   //         while (bytesSend != sendBuf->inBuf)
            overlapped->Internal = 0;
            overlapped->InternalHigh = 0;
            overlapped->Offset = 0;
            overlapped->OffsetHigh = 0;

            rez = WriteFile(deviceParams->m_hComPort, sendBuf->buf, sendBuf->inBuf, NULL, overlapped);
            if (rez == false)
            {
                err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    ERR("WriteFile error GetLastError: " << err);
                    break;
                }
            }
            if (GetOverlappedResult(deviceParams->m_hComPort, overlapped, &lpNumberOfBytesWritten, true))
            {
                DBG("Write to Com " << lpNumberOfBytesWritten << " bytes");
                if (lpNumberOfBytesWritten != sendBuf->inBuf)
                {
                    ERR("WriteFile error. Try to send " << sendBuf->inBuf << " bytes " << "Send really " << lpNumberOfBytesWritten << " bytes");
                }
            }
            else
            {
                err = GetLastError();
                ERR("m_overlappedWr GetOverlappedResult error GetLastError: " << err);
                break;
            }

        }
        else
        {
            ERR("TcpToComBuf read data error");
        }
        TcpToComBuf->GetAvailableData(&availableData);
    } while (availableData > 0);//If there is still something left in the buffer, we repeat the sending
    return S_OK;
}

DWORD WINAPI comDataWriteProc(
    _In_ LPVOID lpParameter
)
{
    CThreadConfig* deviceParams = (CThreadConfig*)lpParameter;
    SockedBuf sendBuf;

    const int waitCount = 2;
    WSAEVENT hDataEvent[waitCount];// = WSA_INVALID_EVENT;
    hDataEvent[0] = deviceParams->m_TcpRcvEvent;
    hDataEvent[1] = deviceParams->m_CloseEvent;
    DWORD wait;

    while (1)
    {
        wait = WaitForMultipleObjects(waitCount, hDataEvent, false, INFINITE);
        if (wait == WAIT_OBJECT_0)
        {
            writeTcpToCom(deviceParams, &sendBuf);
        }
        //Close event
        if (wait == WAIT_OBJECT_0 + 1)
        {
            return 1;
        }
    }

    return 1;
}

#ifdef TCP_CLIENT

int connectToServer(
    CThreadConfig* deviceParams,
    SOCKET ConnectSocket,
    USHORT tcpPort,
    BOOL noDelay
)
{
    CPortSetting* portSetting = deviceParams->m_portSetting;
    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    clientService.sin_addr.s_addr = inet_addr(portSetting->getIP().c_str());//(SRV_HOST);
    clientService.sin_port = htons(tcpPort);
    // Set nonblocking socket
    u_long mode = 1;  // 1 to enable non-blocking socket
    ioctlsocket(ConnectSocket, FIONBIO, &mode);
    int yes = 1;
    if (setsockopt(ConnectSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) == SO_ERROR)
    {
        ERR("SOL_SOCKET, SO_REUSEADDR failed with error: " << WSAGetLastError());
    }
    if (noDelay)
    {
        if (setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(yes)) == SO_ERROR)
        {
            ERR("IPPROTO_TCP, TCP_NODELAY failed with error: " << WSAGetLastError());
        }
    }

    //Set keep alive
    /*const char yes = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));*/
    //----------------------
    // Connect to server.
    return connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));//always return -1, NON-BLOCKING mode used
}

threadStatus waitForDataEventsLoop(
    SOCKET ConnectSocket,
    CThreadConfig* deviceParams
)
{
    const int waitCount = 4;
    WSAEVENT hDataEvent[waitCount];// = WSA_INVALID_EVENT;
    hDataEvent[0] = WSACreateEvent();
    hDataEvent[1] = deviceParams->m_ComRcvEvent;
    hDataEvent[2] = deviceParams->m_CloseEvent;
    hDataEvent[3] = deviceParams->m_ReconnectEvent;
    ::WSAEventSelect(ConnectSocket, hDataEvent[0], FD_WRITE | FD_READ | FD_CLOSE | FD_CONNECT);
    SockedBuf rcvBuf, sendBuf;
    DWORD wait;
    threadStatus tStatus;
    for (;;)
    {
        wait = ::WSAWaitForMultipleEvents(waitCount, hDataEvent, FALSE, 100, FALSE);
        if (wait == WSA_WAIT_EVENT_0)
        {
            tStatus = socketDataEventsHandler(ConnectSocket, deviceParams, &rcvBuf, hDataEvent[0]);
            if (tStatus != threadStatus::THREAD_CONTINUE)
            {
                return tStatus;
            }
        }
        //Write to socket
        if (wait == WSA_WAIT_EVENT_0 + 1)
        {
            tStatus = writeComToTcp(ConnectSocket, deviceParams, &sendBuf);
            if (tStatus != threadStatus::THREAD_CONTINUE)
            {
                return tStatus;
            }
        }
        //Close thread
        if (wait == WSA_WAIT_EVENT_0 + 2)
        {
            LOG("CLOSE THREAD EVENT CATCH");
            return threadStatus::THREAD_FREE;// 0;
        }
        //Reconnect to server
        if (wait == WSA_WAIT_EVENT_0 + 3)
        {
            LOG("RECONNECT EVENT CATCH");
            //ResetEvent(deviceParams->m_ReconnectEvent);
            return threadStatus::THREAD_RECONNECT;
        }
        int error_code;
        int error_code_size = sizeof(error_code);
        getsockopt(ConnectSocket, SOL_SOCKET, SO_ERROR, (char*)&error_code, &error_code_size);
        if (error_code != 0)
        {
            ERR("getsockopt err = " << error_code);
            return threadStatus::THREAD_RECONNECT;
        }
    }
}

DWORD WINAPI sockDataExchangeProc(
    _In_ LPVOID lpParameter
)
{
    CThreadConfig* deviceParams = (CThreadConfig*)lpParameter;
    CPortSetting* portSetting = deviceParams->m_portSetting;
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        ERR("WSAStartup function failed with error : "<< iResult);
        return 1;
    }
    LOG("sockClientProc start");

    for (;;)
    {
        deviceParams->m_portSetting->setStatus(PortStatus::Processing);
        // Create a SOCKET for connecting to server
        SOCKET ConnectSocket;
        ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ConnectSocket == INVALID_SOCKET) {
            ERR("socket function failed with error: " << WSAGetLastError());
            break;
        }

        connectToServer(deviceParams, ConnectSocket, portSetting->getTcpPort(), true);
        threadStatus tResult = waitForDataEventsLoop(ConnectSocket, deviceParams);//blocking loop
        shutdown(ConnectSocket, SD_BOTH);
        iResult = closesocket(ConnectSocket);
        if (tResult == threadStatus::THREAD_FREE)
        {
            break;
        }
        deviceParams->m_portSetting->setStatus(PortStatus::Disconnected);
        if (iResult == SOCKET_ERROR) {
            ERR("closesocket function failed with error: " << WSAGetLastError());
            break;
        }
        Sleep(RECONNECT_TIMEOUT);
        LOG("Main thread not connected Sleep");
    }
    WSACleanup();
    LOG("Main thread closed success!");
    return 1;
}
#endif

#ifdef TCP_SERVER

int createServer(
    CThreadConfig* deviceParams,
    SOCKET ServerSocket,
    USHORT tcpPort,
    BOOL noDelay
)
{
    CPortSetting* portSetting = deviceParams->m_portSetting;
    //int iResult;
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be created.
    int iResult;
    sockaddr_in serverService;
    serverService.sin_family = AF_INET;
    serverService.sin_addr.s_addr = inet_addr(portSetting->getIP().c_str());//(SRV_HOST);
    serverService.sin_port = htons(tcpPort);
    // Set nonblocking socket
    u_long mode = 1;  // 1 to enable non-blocking socket
    ioctlsocket(ServerSocket, FIONBIO, &mode);
    int yes = 1;
    if (setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) == SO_ERROR)
    {
        ERR("SOL_SOCKET, SO_REUSEADDR failed with error: " << WSAGetLastError());
    }
    if (noDelay)
    {
        if (setsockopt(ServerSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(yes)) == SO_ERROR)
        {
            ERR("IPPROTO_TCP, TCP_NODELAY failed with error: " << WSAGetLastError());
        }
    }

    iResult = bind(ServerSocket, (SOCKADDR*)&serverService, sizeof(serverService));
    if (iResult == SOCKET_ERROR) {
        ERR("bind failed with error: " << WSAGetLastError());
        return SOCKET_ERROR;
    }

    iResult = listen(ServerSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        ERR("listen failed with error: " << WSAGetLastError());
        return SOCKET_ERROR;
    }

    return iResult;
}

threadStatus waitForDataEventsLoop(
    SOCKET serverSocket,
    CThreadConfig* deviceParams
)
{
    SOCKET clientSocket = INVALID_SOCKET;
    const int waitCount = 5;
    WSAEVENT hDataEvent[waitCount];// = WSA_INVALID_EVENT;
    hDataEvent[0] = WSACreateEvent();//Server socket events
    hDataEvent[1] = WSACreateEvent();//Connected socket events
    hDataEvent[2] = deviceParams->m_ComRcvEvent;
    hDataEvent[3] = deviceParams->m_CloseEvent;
    hDataEvent[4] = deviceParams->m_ReconnectEvent;
    //::WSAEventSelect(serverSocket, hDataEvent[0], FD_WRITE | FD_READ | FD_CLOSE | FD_CONNECT);
    ::WSAEventSelect(serverSocket, hDataEvent[0], FD_ACCEPT);
    //char rcv[TCP_BUF_SIZE];//, req[TCP_BUF_SIZE];
    SockedBuf rcvBuf, sendBuf;
    DWORD wait;
    threadStatus tStatus;
    for (;;)
    {
        wait = ::WSAWaitForMultipleEvents(waitCount, hDataEvent, FALSE, 100, FALSE);
        if (wait == WSA_WAIT_EVENT_0)
        {
            clientSocket = accept(serverSocket, NULL, NULL);
            ::WSAResetEvent(hDataEvent[0]);
            if (clientSocket == INVALID_SOCKET) {
                ERR("accept failed with error: " << WSAGetLastError());
                return threadStatus::THREAD_FREE;
            }
            deviceParams->m_portSetting->setStatus(PortStatus::Connected);
            //Activate client connection events
            ::WSAEventSelect(clientSocket, hDataEvent[1], FD_WRITE | FD_READ | FD_CLOSE | FD_CONNECT);
        }
        if (wait == WSA_WAIT_EVENT_0 + 1)
        {
            tStatus = socketDataEventsHandler(clientSocket, deviceParams, &rcvBuf, hDataEvent[1]);
            if (tStatus != threadStatus::THREAD_CONTINUE)
            {
                //Deactivate client connection events
                ::WSAEventSelect(clientSocket, hDataEvent[1], 0);
                return tStatus;
            }
        }
        //Write to socket
        if (wait == WSA_WAIT_EVENT_0 + 2)
        {
            tStatus = writeComToTcp(clientSocket, deviceParams, &sendBuf);
            if (tStatus != threadStatus::THREAD_CONTINUE)
            {
                return tStatus;
            }
        }
        //Close thread
        if (wait == WSA_WAIT_EVENT_0 + 3)
        {
            LOG("CLOSE THREAD EVENT CATCH");
            return threadStatus::THREAD_FREE;// 0;
        }
        //Reconnect to server
        if (wait == WSA_WAIT_EVENT_0 + 4)
        {
            LOG("RECONNECT EVENT CATCH");
            //ResetEvent(deviceParams->m_ReconnectEvent);
            return threadStatus::THREAD_RECONNECT;
        }
        int error_code;
        int error_code_size = sizeof(error_code);
        getsockopt(serverSocket, SOL_SOCKET, SO_ERROR, (char*)&error_code, &error_code_size);
        if (error_code != 0)
        {
            ERR("getsockopt err = " << error_code);
            return threadStatus::THREAD_RECONNECT;
        }
    }
}

DWORD WINAPI sockDataExchangeProc(
    _In_ LPVOID lpParameter
)
{
    CThreadConfig* deviceParams = (CThreadConfig*)lpParameter;
    CPortSetting* portSetting = deviceParams->m_portSetting;
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        ERR("WSAStartup function failed with error : " << iResult);
        return 1;
    }
    LOG("sockClientProc start");

    for (;;)
    {
        // Create a server SOCKET 
        deviceParams->m_portSetting->setStatus(PortStatus::Processing);
        SOCKET serverSocket;
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            ERR("socket function failed with error: " << WSAGetLastError());
            break;
        }

        iResult = createServer(deviceParams, serverSocket, portSetting->getTcpPort(), true);
        if (iResult == SOCKET_ERROR) {
            //ERR("closesocket function failed with error: " << WSAGetLastError());
            break;
        }
        threadStatus tResult = waitForDataEventsLoop(serverSocket, deviceParams);//blocking loop
        shutdown(serverSocket, SD_BOTH);
        iResult = closesocket(serverSocket);
        if (tResult == threadStatus::THREAD_FREE)
        {
            break;
        }
        deviceParams->m_portSetting->setStatus(PortStatus::Disconnected);
        if (iResult == SOCKET_ERROR) {
            ERR("closesocket function failed with error: " << WSAGetLastError());
            break;
        }
        Sleep(RECONNECT_TIMEOUT);
        LOG("Main thread not connected Sleep");
    }
    WSACleanup();
    LOG("Main thread closed success!");
    return 1;
}


#endif