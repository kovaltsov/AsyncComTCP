#include "string"
#include "vector"

#pragma once

using namespace std;

//#include "AsyncComTCP.h"
class CPortSetting
{
private:
	string comPort;
	string ip;
	int tcpPort;

public:
	CPortSetting();
	CPortSetting(const string &ComPort,
		const string &IP,
		int TcpPort);
public:
	string getIP();
	string getComPort();
	int getTcpPort();
	void setIP(const string& IP);
	void setComPort(const string& ComPort);
	void setTcpPort(int TcpPort);
};

