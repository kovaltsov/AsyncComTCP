#pragma once

#include "CPortSetting.h"
#include "ringbuffer.h"

// Set ring buffer size
#define DATA_BUFFER_SIZE 10240
// Set TCP read write buffer size
#define TCP_BUF_SIZE 1024 
//Timeout for reset m_Connected status, ms
#define ECHO_TIMEOUT 3000
#define CLOSE_HANDLE_TIMEOUT 10000
#define RECONNECT_TIMEOUT 1000
#define OPEN_HANDLE_TIMEOUT 3000

#define MAX_ECHO_SIZE 6//5 + 1 (65535 + \0)

enum class threadStatus
{
    THREAD_CONTINUE,
    THREAD_RECONNECT,
    THREAD_FREE
};

struct SockedBuf {
    byte buf[TCP_BUF_SIZE];
    SIZE_T inBuf;
    SockedBuf()
    {
        memset(buf, 0, TCP_BUF_SIZE);
        inBuf = 0;
    }
};

DWORD WINAPI sockDataExchangeProc(
    _In_ LPVOID lpParameter
);

DWORD WINAPI comDataExchangeProc(
    _In_ LPVOID lpParameter
);