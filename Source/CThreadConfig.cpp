#include "pch.h"
#include "common.h"

vector<CThreadConfig*> threadConfigs;

CThreadConfig::CThreadConfig(_In_ CPortSetting* portSetting)
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

	hr = createEvent(&m_TcpRcvEvent, FALSE, "m_TcpRcvEvent");

	if (SUCCEEDED(hr))
	{
		hr = createEvent(&m_ComRcvEvent, FALSE, "m_ComRcvEvent");
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
//COM port handles
	//Overlapped read
	if (SUCCEEDED(hr))
	{
		hr = createOverlappedEvent(&m_overlappedRd, "m_overlappedRd");
	}

	//Overlapped write
	if (SUCCEEDED(hr))
	{
		hr = createOverlappedEvent(&m_overlappedWr, "m_overlappedWr");
	}

	//Open Com Port
	if (SUCCEEDED(hr))
	{
		hr = comPortOpen(m_hComPort, portSetting->getComPort());
	}

	//COM read thread
	if (SUCCEEDED(hr))
	{
		m_ComPortRdHandle = CreateThread(NULL, 0, &comDataReadProc,
			this,
			0, NULL);
		if (m_ComPortRdHandle == NULL)
		{
			hr = E_UNEXPECTED;
		}
	}

	//COM write thread
	if (SUCCEEDED(hr))
	{
		m_ComPortWrHandle = CreateThread(NULL, 0, &comDataWriteProc,
			this,
			0, NULL);
		if (m_ComPortWrHandle == NULL)
		{
			hr = E_UNEXPECTED;
		}
	}

	if (FAILED(hr))
	{
		ERR("Failed to start main TCP thread!");
		closeHandle(&m_TcpRcvEvent, "&m_TcpRcvEvent");
		closeHandle(&m_ComRcvEvent, "m_ComRcvEvent");
		closeHandle(&m_CloseEvent, "m_CloseEvent");
		closeHandle(&m_ConfigEvent, "m_ConfigEvent");
		closeHandle(&m_ReconnectEvent, "m_ReconnectEvent");
		closeHandle(&m_ConnectEvent, "m_ConnectEvent");
		closeHandle(&m_hComPort, "m_hComPort");
		closeHandle(&m_overlappedRd.hEvent, "m_overlappedRd");
		closeHandle(&m_overlappedWr.hEvent, "m_overlappedWr");
		terminateThread(&m_SockClientHandle, "m_SockClientHandle");
		terminateThread(&m_ComPortRdHandle, "m_ComPortRdHandle");
		terminateThread(&m_ComPortWrHandle, "m_ComPortWrHandle");
		throw new exception("Threads create fault");
	}
}

CThreadConfig::~CThreadConfig()
{
	//    DWORD wait;
	SetEvent(m_CloseEvent);
	waitToCloseThread(&m_SockClientHandle, "sockClientProc");
	waitToCloseThread(&m_ComPortRdHandle, "comDataReadProc");
	waitToCloseThread(&m_ComPortWrHandle, "comDataWriteProc");
	closeHandle(&m_TcpRcvEvent, "m_TcpRcvEvent");
	closeHandle(&m_ComRcvEvent, "m_ComRcvEvent");
	closeHandle(&m_CloseEvent, "m_CloseEvent");
	closeHandle(&m_ConfigEvent, "m_ConfigEvent");
	closeHandle(&m_ReconnectEvent, "m_ReconnectEvent");
	closeHandle(&m_ConnectEvent, "m_ConnectEvent");
	closeHandle(&m_hComPort, "m_hComPort");
	closeHandle(&m_overlappedRd.hEvent, "m_overlappedRd");
	closeHandle(&m_overlappedWr.hEvent, "m_overlappedWr");
}

HRESULT
CThreadConfig::createEvent(
	_Out_ HANDLE* hEvent,
	_In_ BOOL manualReset,
	_In_ char* hNameToTrace
)
{
	HRESULT hr = S_OK;
	*hEvent = CreateEventA(NULL, manualReset, FALSE, NULL);
	LOG(hNameToTrace << "=" << (int*)*hEvent);
	if (*hEvent == NULL)
	{
		hr = E_UNEXPECTED;
	}
	return hr;
}

HRESULT CThreadConfig::createOverlappedEvent(
	_Out_ OVERLAPPED* overlapped,
	_In_ char* hNameToTrace)
{
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->Offset = 0;
	overlapped->OffsetHigh = 0;
	return createEvent(&overlapped->hEvent, true, hNameToTrace);
}

void
CThreadConfig::waitToCloseThread(
	_Inout_ HANDLE* h,
	_In_ char* hNameToTrace)
{
	DWORD wait;
	//    SetEvent(m_CloseEvent);
	wait = WaitForSingleObject(*h, CLOSE_HANDLE_TIMEOUT);
	if (wait == WAIT_OBJECT_0)
	{//Корректное завершение потока
		*h = NULL;
		LOG("Stop " << hNameToTrace << " success!");
	}
	else if (wait == WAIT_TIMEOUT)
	{//Принудительное завершение потока
		GetExitCodeThread(*h, &wait);
		if (wait == STILL_ACTIVE)
		{
			ERR("Stop " << hNameToTrace << " timeout! Try TerminateThread function");
			TerminateThread(*h, 1);
			wait = WaitForSingleObject(*h, CLOSE_HANDLE_TIMEOUT);
			if (wait == WAIT_OBJECT_0)
			{
				*h = NULL;
				ERR("Terminate " << hNameToTrace << " success!");
			}
			else
			{
				ERR("Terminate " << hNameToTrace << " failed GetLastError : " << GetLastError());
			}
		}
	}
	else
	{
		ERR("Stop " << hNameToTrace << " failed GetLastError :" << GetLastError());
	}
}

void
CThreadConfig::closeHandle(
	_Inout_ HANDLE* h,
	_In_ char* hNameToTrace)
{
	if (*h == NULL) return;
	bool close = false;
	close = CloseHandle(*h);
	*h = NULL;
	LOG("CloseHandle " << hNameToTrace << " = " << close);
	if (close == false)
	{
		ERR(hNameToTrace << " GetLastError: " << GetLastError());
	}
}

void
CThreadConfig::terminateThread(
	_Inout_ HANDLE* h,
	_In_ char* hNameToTrace)
{
	if (*h == NULL) return;
	bool close = false;
	close = TerminateThread(*h, 1);
	*h = NULL;
	LOG("TerminateThread" << hNameToTrace << " = " << close);
	if (close == false)
	{
		ERR(hNameToTrace << " GetLastError: " << GetLastError());
	}
}

HRESULT CThreadConfig::comPortOpen(
	_Out_ HANDLE &hComPort, 
	_In_ const string& port)
{
	string addToPort = "\\\\.\\";
	HRESULT hr = S_OK;
	string p = port;
	p.insert(0, addToPort);
	hComPort = CreateFileA(p.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hComPort == INVALID_HANDLE_VALUE)
	{
		hr = E_UNEXPECTED;
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			ERR("serial port does not exist");
		}
		else
		{
			ERR("some other error occurred " << GetLastError());
		}
	}
	else
	{
		LOG("Port open successfully! " << port);
		DWORD fileType = GetFileType(hComPort);
		LOG("fileType = " << fileType << " lastError = " << GetLastError());
		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (!GetCommState(hComPort, &dcbSerialParams))
		{
			hr = E_UNEXPECTED;
			ERR("getting state error");
		}
		dcbSerialParams.BaudRate = CBR_256000;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = EVENPARITY;
		if (!SetCommState(hComPort, &dcbSerialParams))
		{
			hr = E_UNEXPECTED;
			ERR("error setting serial port state");
		}
	}
	return hr;
}
