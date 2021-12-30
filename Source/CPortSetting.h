#pragma once
#include "pch.h"

#define MIN_USER_TCP_PORT 1024
#define MAX_USER_TCP_PORT 49151

class CPortSetting
{
private:
	string comPort;
	string ip;
	USHORT tcpPort;

public:
	CPortSetting();
	CPortSetting(_In_ const string &ComPort,
		_In_ const string &IP,
		_In_ USHORT TcpPort);
public:
	string getIP();
	string getComPort();
	USHORT getTcpPort();
	void setIP(_In_ const string& IP);
	void setComPort(_In_ const string& ComPort);
	void setTcpPort(_In_ int TcpPort);
};
