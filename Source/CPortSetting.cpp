#include "pch.h"
#include "common.h"

vector<CPortSetting> portSettings;

CPortSetting::CPortSetting(_In_ const string &ComPort,
	_In_ const string &IP,
	_In_ USHORT TcpPort)
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
USHORT CPortSetting::getTcpPort()
{
	return tcpPort;
}

void CPortSetting::setIP(_In_ const string& IP)
{
	ip = IP;
}
void CPortSetting::setComPort(_In_ const string& ComPort)
{
	if (ComPort.find("COM", 0) != 0) throw new exception("does not start with COM");
	comPort = ComPort;
}
void CPortSetting::setTcpPort(_In_ int TcpPort)
{
	if (TcpPort < MIN_USER_TCP_PORT || TcpPort > MAX_USER_TCP_PORT) throw new exception("Tcp port incorrect");
	tcpPort = (USHORT)TcpPort;
}