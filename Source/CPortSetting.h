#pragma once
#include "pch.h"
#include "AsyncComTCPDlg.h"

#define MIN_USER_TCP_PORT 1024
#define MAX_USER_TCP_PORT 49151

enum class PortStatus : BYTE
{
	Stopped,
	Disconnected,
	Processing,
	Connected,
};

class CPortSetting
{
private:
	string comPort;
	string ip;
	USHORT tcpPort;
	PortStatus status;
	bool statusChange = false;
public:
	CPortSetting();
	CPortSetting(_In_ const string &ComPort,
		_In_ const string &IP,
		_In_ USHORT TcpPort);
	~CPortSetting();
	bool isStatusChange();
public:
	string getIP();
	string getComPort();
	USHORT getTcpPort();
	PortStatus getStatus();
	string getStatusString();
	void setIP(_In_ const string& IP);
	void setComPort(_In_ const string& ComPort);
	void setTcpPort(_In_ int TcpPort);
	void setStatus(_In_ PortStatus Status);
};
