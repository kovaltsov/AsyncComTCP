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

	hr = createEvent(&m_ReadEvent, FALSE, "m_ReadEvent");

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
		hr = comPortOpen(m_hComPort, &portSetting->getComPort());
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
		cout << "Failed to start main TCP thread!";
		closeHandle(&m_ReadEvent, "&m_ReadEvent");
		closeHandle(&m_WriteEvent, "m_WriteEvent");
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
	closeHandle(&m_hComPort, "m_hComPort");
	closeHandle(&m_overlappedRd.hEvent, "m_overlappedRd");
	closeHandle(&m_overlappedWr.hEvent, "m_overlappedWr");
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
	cout << hNameToTrace << "=" << (int*)*hEvent << endl;
	if (*hEvent == NULL)
	{
		hr = E_UNEXPECTED;
	}
	return hr;
}

HRESULT CThreadConfig::createOverlappedEvent(OVERLAPPED* overlapped, char* hNameToTrace)
{
	overlapped->Internal = 0;
	overlapped->InternalHigh = 0;
	overlapped->Offset = 0;
	overlapped->OffsetHigh = 0;
	return createEvent(&overlapped->hEvent, true, hNameToTrace);
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
		cout << "Stop " << hNameToTrace << " success!" << endl;
	}
	else if (wait == WAIT_TIMEOUT)
	{//Принудительное завершение потока
		cout << "Stop " << hNameToTrace << " timeout! Try TerminateThread function" << endl;
		TerminateThread(*h, 1);
		wait = WaitForSingleObject(*h, CLOSE_HANDLE_TIMEOUT);
		if (wait == WAIT_OBJECT_0)
		{
			*h = NULL;
			cout << "Terminate " << hNameToTrace <<" success!" << endl;
		}
		else
		{
			cout << "Terminate " << hNameToTrace << " failed GetLastError : " << GetLastError() << endl;
		}
	}
	else
	{
		cout << "Stop " << hNameToTrace << " failed GetLastError :" << GetLastError() << endl;
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
	cout << "CloseHandle " << hNameToTrace << " = " << close << endl;
	if (close == false)
	{
		cout << hNameToTrace << " GetLastError: " << GetLastError() << endl;
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
	cout << "TerminateThread" << hNameToTrace << " = " << close << endl;
	if (close == false)
	{
		cout << hNameToTrace << " GetLastError: " << GetLastError() << endl;
	}
}

HRESULT CThreadConfig::comPortOpen(HANDLE &hComPort, string* port)
{
	HRESULT hr = S_OK;
	hComPort = CreateFileA(port->c_str(), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hComPort == INVALID_HANDLE_VALUE)
	{
		hr = E_UNEXPECTED;
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			cout << "serial port does not exist.\n";
		}
		else
		{
			cout << "some other error occurred.\n";
			cout << GetLastError();
		}
	}
	else
	{
		cout << "Port open successfully! " << port << endl;
		DWORD fileType = GetFileType(hComPort);
		cout << "fileType = " << fileType << " lastError = " << GetLastError() << endl;
		DCB dcbSerialParams = { 0 };
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (!GetCommState(hComPort, &dcbSerialParams))
		{
			hr = E_UNEXPECTED;
			cout << "getting state error\n";
		}
		dcbSerialParams.BaudRate = CBR_56000;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = EVENPARITY;
		if (!SetCommState(hComPort, &dcbSerialParams))
		{
			hr = E_UNEXPECTED;
			cout << "error setting serial port state\n";
		}
	}
	return hr;
}
