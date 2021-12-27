#include "pch.h"
#include "common.h"

// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>  



#ifdef TCP_CLIENT

int connectToServer(
    CThreadConfig* deviceParams,
    SOCKET ConnectSocket,
    USHORT tcpPort,
    BOOL noDelay
)
{
    CPortSetting* portSetting = deviceParams->m_portSetting;
    //int iResult;
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port of the server to be connected to.
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
        cout << "SOL_SOCKET, SO_REUSEADDR failed with error: " << WSAGetLastError() << endl;
    }
    if (noDelay)
    {
        if (setsockopt(ConnectSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(yes)) == SO_ERROR)
        {
            cout << "IPPROTO_TCP, TCP_NODELAY failed with error: " << WSAGetLastError() << endl;
        }
    }

    //Set keep alive
    /*const char yes = 1;
    setsockopt(ConnectSocket, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));*/
    //----------------------
    // Connect to server.
    return connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));//always return -1, NON-BLOCKING mode used
   /*   if (iResult == SOCKET_ERROR) {
        cout << "connect function failed with error: %ld\n", WSAGetLastError());
        iResult = closesocket(ConnectSocket);
        if (iResult == SOCKET_ERROR)
            cout << "closesocket function failed with error: %ld\n", WSAGetLastError());
        return false;
    }
    pDevice->SetModemStatusMask(SERIAL_MSR_DSR | SERIAL_MSR_DCD);

    cout << "Connected to server.\n");
    return true;*/
}

//Обработка сетевых событий сокета
threadStatus socketDataEventsHandler(SOCKET ConnectSocket, CThreadConfig* deviceParams, SockedBuf* rcvBuf, HANDLE hDataEvent)
{
    CRingBuffer* TcpToComBuf = &deviceParams->m_TcpToComBuf;
    int rez;
    WSANETWORKEVENTS wsaProcessEvent;
    //while (1)
    //{

    ::WSAEnumNetworkEvents(ConnectSocket, hDataEvent, &wsaProcessEvent);
    if (wsaProcessEvent.lNetworkEvents & FD_READ)
    {
        cout << "TCP Handle FD_READ" << endl;
        memset(rcvBuf->buf, 0, TCP_BUF_SIZE);
        rcvBuf->inBuf = recv(ConnectSocket, (char*)rcvBuf->buf, TCP_BUF_SIZE, 0);
        TcpToComBuf->Write((byte*)rcvBuf->buf, rcvBuf->inBuf);
        cout << "TCP len = " << (int)rcvBuf->inBuf << "rcv: " << (char*)rcvBuf->buf << endl;
        SetEvent(deviceParams->m_ReadEvent);
    }
    if (wsaProcessEvent.lNetworkEvents & FD_CONNECT)
    {
        cout << "TCP Handle FD_CONNECT\n";
        rez = wsaProcessEvent.iErrorCode[FD_CONNECT_BIT];
        if (rez == 0)
        {
            cout << "Connected to server.\n" << endl;
        }
        else
        {
            cout << "connect function failed with error: " << rez << endl;
            return threadStatus::THREAD_RECONNECT;
        }
    }
    if (wsaProcessEvent.lNetworkEvents & FD_WRITE)
    {
        cout << "TCP Handle FD_WRITE\n" << endl;
        if (deviceParams->m_NeedResend)
        {
            deviceParams->m_NeedResend = false;
            SetEvent(deviceParams->m_WriteEvent);
        }
    }
    if (wsaProcessEvent.lNetworkEvents & FD_CLOSE)
    {
        cout << "TCP Handle FD_CLOSE\n" << endl;
        return threadStatus::THREAD_RECONNECT;
    }
    return threadStatus::THREAD_CONTINUE;
}

//Обработка события на запись данных из COM порта в TCP
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
    //ResetEvent(deviceParams->m_WriteEvent);
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
                    cout << "Write to socket. len = " << (ULONG)sendBuf->inBuf <<" rez = " << rez << endl;
                }
                else if (rez == SOCKET_ERROR)
                {
                    rez = GetLastError();
                    if (rez == WSAEWOULDBLOCK)
                    {
                        cout << "Socket send error GetLastError: WSAEWOULDBLOCK. Wait FD_WRITE" << endl;
                        //SetEvent(deviceParams->m_WriteEvent);
                        ComToTcpBuf->CancelReadBytes(sendBuf->inBuf - bytesSend);
                        deviceParams->m_NeedResend = true;
                        return threadStatus::THREAD_CONTINUE;
                    }
                    else
                    {
                        cout << "Socket send error GetLastError: " << rez << endl;
                        return threadStatus::THREAD_RECONNECT;
                    }
                }
            }
        }
        else
        {
            cout << "ComToTcpBuf read data error" << endl;
        }
        ComToTcpBuf->GetAvailableData(&availableData);
    } while (availableData > 0);//Если в буфере еще что-то осталось повторяем отправку
    return threadStatus::THREAD_CONTINUE;
}

threadStatus waitForDataEventsLoop(
    SOCKET ConnectSocket,
    CThreadConfig* deviceParams
)
{
    const int waitCount = 4;
    WSAEVENT hDataEvent[waitCount];// = WSA_INVALID_EVENT;
    hDataEvent[0] = WSACreateEvent();
    hDataEvent[1] = deviceParams->m_WriteEvent;
    hDataEvent[2] = deviceParams->m_CloseEvent;
    hDataEvent[3] = deviceParams->m_ReconnectEvent;
    ::WSAEventSelect(ConnectSocket, hDataEvent[0], FD_WRITE | FD_READ | FD_CLOSE | FD_CONNECT);
    //char rcv[TCP_BUF_SIZE];//, req[TCP_BUF_SIZE];
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
   //         pDevice->ResetModemStatusMask(SERIAL_MSR_DSR | SERIAL_MSR_DCD);
            cout << "CLOSE THREAD EVENT CATCH" << endl;
            return threadStatus::THREAD_FREE;// 0;
        }
        //Reconnect to server
        if (wait == WSA_WAIT_EVENT_0 + 3)
        {
            cout << "RECONNECT EVENT CATCH" << endl;
            //ResetEvent(deviceParams->m_ReconnectEvent);
            return threadStatus::THREAD_RECONNECT;
        }
        int error_code;
        int error_code_size = sizeof(error_code);
        getsockopt(ConnectSocket, SOL_SOCKET, SO_ERROR, (char*)&error_code, &error_code_size);
        if (error_code != 0)
        {
            cout << "getsockopt err = " << error_code << endl;
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
        cout << "WSAStartup function failed with error : "<< iResult << endl;
        return 1;
    }
    cout<<"sockClientProc start"<<endl;

    for (;;)
    {
        // Create a SOCKET for connecting to server
        SOCKET ConnectSocket;
        ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ConnectSocket == INVALID_SOCKET) {
            cout << "socket function failed with error: " << WSAGetLastError() << endl;
            break;
        }

        connectToServer(deviceParams, ConnectSocket, portSetting->getTcpPort(), true);
        threadStatus tResult = waitForDataEventsLoop(ConnectSocket, deviceParams);//blocking loop
        shutdown(ConnectSocket, SD_BOTH);
        iResult = closesocket(ConnectSocket);
        if (iResult == SOCKET_ERROR) {
            cout << "closesocket function failed with error: " << WSAGetLastError() << endl;
            break;
        }
        if (tResult == threadStatus::THREAD_FREE)
        {
            break;
        }
        Sleep(RECONNECT_TIMEOUT);
        cout << "Main thread not connected Sleep" << endl;
    }
    WSACleanup();
    cout << "Main thread closed success!" << endl;
    return 1;
}

DWORD WINAPI comDataExchangeProc(
    _In_ LPVOID lpParameter
)
{
    return 0;
}

#endif