#pragma once
#include "pch.h"
#include "CPortSetting.h"
#include "ringbuffer.h"

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
	HANDLE         m_ReadEvent;
	HANDLE         m_WriteEvent;
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
	CThreadConfig(CPortSetting* portSetting);
	~CThreadConfig();
private:
	HRESULT
		CThreadConfig::createEvent(
			HANDLE* hEvent,
			_In_ BOOL manualReset,
			_In_ char* hNameToTrace
		);

	HRESULT
		CThreadConfig::createOverlappedEvent(
			OVERLAPPED* overlapped,
			_In_ char* hNameToTrace
		);

	void
		CThreadConfig::waitToCloseThread(
			HANDLE* h,
			_In_ char* hNameToTrace);


	void
		CThreadConfig::closeHandle(
			HANDLE* h,
			_In_ char* hNameToTrace);

	void
		CThreadConfig::terminateThread(
			HANDLE* h,
			_In_ char* hNameToTrace);

	HRESULT
		comPortOpen(
			HANDLE& hComPort,
			string* port
		);

};

