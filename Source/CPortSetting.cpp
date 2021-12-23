#include "pch.h"
#include "common.h"

vector<CPortSetting> portSettings;

CPortSetting::CPortSetting(const string &ComPort,
	const string &IP,
	int TcpPort)
{
	comPort = ComPort;
	ip = IP;
	tcpPort = TcpPort;
}

CPortSetting::CPortSetting()
{
	comPort = "";
	ip = "";
	tcpPort = 0;
}

string CPortSetting::getIP()
{
	return ip;
}
string CPortSetting::getComPort()
{
	return comPort;
}
int CPortSetting::getTcpPort()
{
	return tcpPort;
}

void CPortSetting::setIP(const string& IP)
{
	ip = IP;
}
void CPortSetting::setComPort(const string& ComPort)
{
	if (ComPort.find("COM", 0) != 0) throw new exception("does not start with COM");
	comPort = ComPort;
}
void CPortSetting::setTcpPort(int TcpPort)
{
	if (TcpPort > 65535) throw new exception("Tcp port incorrect");
	tcpPort = TcpPort;
}