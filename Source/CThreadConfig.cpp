#include "pch.h"
#include "common.h"

vector<CThreadConfig*> threadConfigs;

CThreadConfig::CThreadConfig(CPortSetting* portSetting)
{
	HRESULT hr = S_OK;

	m_portSetting = portSetting;
	m_TcpToComBuf.Initialize(DATA_BUFFER_SIZE);
	m_ComToTcpBuf.Initialize(DATA_BUFFER_SIZE);
	/*if (m_SockClientHandle != NULL)
	{
		Trace(TRACE_LEVEL_ERROR, "Previous thread exist. Can't create new thread");
		return E_ABORT;
	}*/

	hr = createEvent(&m_ReadEvent, TRUE, "m_ReadEvent");

	if (SUCCEEDED(hr))
	{
		hr = createEvent(&m_WriteEvent, FALSE, "m_WriteEvent");
	}

	if (SUCCEEDED(hr))
	{
		hr = createEvent(&m_CloseEvent, TRUE, "m_CloseEvent");
	}

	if (SUCCEEDED(hr))
	{
		hr = createEvent(&m_ConfigEvent, FALSE, "m_ConfigEvent");
	}

	//Reconnect Event
	if (SUCCEEDED(hr))
	{
		hr = createEvent(&m_ReconnectEvent, FALSE, "m_ReconnectEvent");
	}

	//Connect Event
	if (SUCCEEDED(hr))
	{
		hr = createEvent(&m_ConnectEvent, FALSE, "m_ConnectEvent");
	}
	m_NeedResend = false;

	//Socket thread
	if (SUCCEEDED(hr))
	{
		m_SockClientHandle = CreateThread(NULL, 0, &sockDataExchangeProc,
			this,
			0, NULL);
		if (m_SockClientHandle == NULL)
		{
			hr = E_UNEXPECTED;
		}
	}

	//COM exchange thread
	if (SUCCEEDED(hr))
	{
		m_ComPortHandle = CreateThread(NULL, 0, &comDataExchangeProc,
			this,
			0, NULL);
		if (m_ComPortHandle == NULL)
		{
			hr = E_UNEXPECTED;
		}
	}

	if (FAILED(hr))
	{
		cout << "Failed to start main TCP thread!";
		closeHandle(&m_ReadEvent, "&m_ReadEvent");
		closeHandle(&m_WriteEvent, "m_WriteEvent");
		closeHandle(&m_CloseEvent, "m_CloseEvent");
		closeHandle(&m_ConfigEvent, "m_ConfigEvent");
		closeHandle(&m_ReconnectEvent, "m_ReconnectEvent");
		closeHandle(&m_ConnectEvent, "m_ConnectEvent");
		terminateThread(&m_SockClientHandle, "m_SockClientHandle");
		terminateThread(&m_ComPortHandle, "m_ComPortHandle");
		throw new exception("Threads create fault");
	}
}

CThreadConfig::~CThreadConfig()
{
	//    DWORD wait;
	SetEvent(m_CloseEvent);
	waitToCloseThread(&m_SockClientHandle, "sockClientProc");
	waitToCloseThread(&m_ComPortHandle, "comDataExchangeProc");
	//    wait = WaitForSingleObject(m_SockClientHandle, 10000);
		//if (wait == WAIT_OBJECT_0)
		//{//Корректное завершение потока
		//    m_SockClientHandle = NULL;
		//    Trace(TRACE_LEVEL_INFORMATION, "Stop sockClientProc success!");
		//}
		//else if (wait == WAIT_TIMEOUT)
		//{//Принудительное завершение потока
		//    Trace(TRACE_LEVEL_INFORMATION, "Stop sockClientProc timeout! Try TerminateThread function");
		//    TerminateThread(m_SockClientHandle, 1);
		//    wait = WaitForSingleObject(m_SockClientHandle, 10000);
		//    if (wait == WAIT_OBJECT_0)
		//    {
		//        m_SockClientHandle = NULL;
		//        Trace(TRACE_LEVEL_INFORMATION, "Terminate sockClientProc success!");
		//    }
		//    else
		//    {
		//        Trace(TRACE_LEVEL_ERROR, "Terminate sockClientProc failed GetLastError: %ld", GetLastError());
		//    }
		//}
		//else
		//{
		//    Trace(TRACE_LEVEL_ERROR, "Stop sockClientProc failed GetLastError: %ld", GetLastError());
		//}
	closeHandle(&m_ReadEvent, "m_ReadEvent");
	closeHandle(&m_WriteEvent, "m_WriteEvent");
	closeHandle(&m_CloseEvent, "m_CloseEvent");
	closeHandle(&m_ConfigEvent, "m_ConfigEvent");
	closeHandle(&m_ReconnectEvent, "m_ReconnectEvent");
	closeHandle(&m_ConnectEvent, "m_ConnectEvent");
	//return S_OK;
}

HRESULT
CThreadConfig::createEvent(
	HANDLE* hEvent,
	_In_ BOOL manualReset,
	_In_ char* hNameToTrace
)
{
	HRESULT hr = S_OK;
	*hEvent = CreateEventA(NULL, manualReset, FALSE, NULL);
	cout << hNameToTrace << "=" << (int*)*hEvent;
	if (*hEvent == NULL)
	{
		hr = E_UNEXPECTED;
	}
	return hr;
}

void
CThreadConfig::waitToCloseThread(
	HANDLE* h,
	_In_ char* hNameToTrace)
{
	DWORD wait;
	//    SetEvent(m_CloseEvent);
	wait = WaitForSingleObject(*h, CLOSE_HANDLE_TIMEOUT);
	if (wait == WAIT_OBJECT_0)
	{//Корректное завершение потока
		*h = NULL;
		cout << "Stop " << hNameToTrace << " success!";
	}
	else if (wait == WAIT_TIMEOUT)
	{//Принудительное завершение потока
		cout << "Stop " << hNameToTrace << " timeout! Try TerminateThread function";
		TerminateThread(*h, 1);
		wait = WaitForSingleObject(*h, CLOSE_HANDLE_TIMEOUT);
		if (wait == WAIT_OBJECT_0)
		{
			*h = NULL;
			cout << "Terminate " << hNameToTrace <<" success!";
		}
		else
		{
			cout << "Terminate " << hNameToTrace << " failed GetLastError : " << GetLastError();
		}
	}
	else
	{
		cout << "Stop " << hNameToTrace << " failed GetLastError :" << GetLastError();
	}
}

void
CThreadConfig::closeHandle(
	HANDLE* h,
	_In_ char* hNameToTrace)
{
	if (*h == NULL) return;
	bool close = false;
	close = CloseHandle(*h);
	*h = NULL;
	cout << "CloseHandle " << hNameToTrace << " = " << close;
	if (close == false)
	{
		cout << hNameToTrace << " GetLastError: " << GetLastError();
	}
}

void
CThreadConfig::terminateThread(
	HANDLE* h,
	_In_ char* hNameToTrace)
{
	if (*h == NULL) return;
	bool close = false;
	close = TerminateThread(*h, 1);
	*h = NULL;
	cout << "TerminateThread" << hNameToTrace << " = " << close;
	if (close == false)
	{
		cout << hNameToTrace << " GetLastError: " << GetLastError();
	}
}