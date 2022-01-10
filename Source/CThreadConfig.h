#pragma once
#include "pch.h"
#include "common.h"

class CThreadConfig
{
public:
	CPortSetting *m_portSetting;
	CRingBuffer m_TcpToComBuf;
	CRingBuffer m_ComToTcpBuf;
	//Threads
	HANDLE         m_SockClientHandle;
	HANDLE         m_ComPortRdHandle;
	HANDLE         m_ComPortWrHandle;
	//Events
	HANDLE         m_TcpRcvEvent;
	HANDLE         m_ComRcvEvent;
	HANDLE         m_CloseEvent;
	HANDLE         m_ConfigEvent;
	HANDLE         m_ReconnectEvent;
	HANDLE         m_ConnectEvent;
	BOOL           m_NeedResend;
	//Com port
	OVERLAPPED	   m_overlappedRd; //read thread
	OVERLAPPED     m_overlappedWr; //write thread
	HANDLE		   m_hComPort;
public:
	CThreadConfig(_In_ CPortSetting* portSetting);
	~CThreadConfig();
private:
	HRESULT
		createEvent(
			_Out_ HANDLE* hEvent,
			_In_ BOOL manualReset,
			_In_ char* hNameToTrace
		);

	HRESULT
		createOverlappedEvent(
			_Out_ OVERLAPPED* overlapped,
			_In_ char* hNameToTrace
		);

	void
		waitToCloseThread(
			_Inout_ HANDLE* h,
			_In_ char* hNameToTrace);


	void
		closeHandle(
			_Inout_ HANDLE* h,
			_In_ char* hNameToTrace);

	void
		terminateThread(
			_Inout_ HANDLE* h,
			_In_ char* hNameToTrace);

	HRESULT
		comPortOpen(
			_Out_ HANDLE& hComPort,
			_In_ const string& port);

};

