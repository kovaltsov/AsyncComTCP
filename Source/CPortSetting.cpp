#include "pch.h"
#include "common.h"
#include "AsyncComTCPDlg.h"

vector<CPortSetting> portSettings;

CPortSetting::CPortSetting(_In_ const string& ComPort,
	_In_ const string& IP,
	_In_ USHORT TcpPort)
{
	comPort = ComPort;
	ip = IP;
	tcpPort = TcpPort;
	status = PortStatus::Stopped;
}

CPortSetting::~CPortSetting()
{
	;
}

bool CPortSetting::isStatusChange()
{
	bool ret = statusChange;
	statusChange = false;
	return ret;
}

CPortSetting::CPortSetting()
{
	comPort = "";
	ip = "0.0.0.0";
	tcpPort = 0;
	status = PortStatus::Stopped;
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

PortStatus CPortSetting::getStatus()
{
	return status;
}

string CPortSetting::getStatusString()
{
	switch (status)
	{
	case PortStatus::Stopped:
		return "Stopped";
	case PortStatus::Disconnected:
		return "Disconnected";
	case PortStatus::Connected:
		return "Connected";
	case PortStatus::Processing:
		return "Processing";
	}
	throw exception("Bad status");
}

void CPortSetting::setIP(_In_ const string& IP)
{
	ip = IP;
}
void CPortSetting::setComPort(_In_ const string& ComPort)
{
	if (ComPort.find("COM", 0) != 0) throw exception("does not start with COM");
	comPort = ComPort;
}
void CPortSetting::setTcpPort(_In_ int TcpPort)
{
	if (TcpPort < MIN_USER_TCP_PORT || TcpPort > MAX_USER_TCP_PORT) throw exception("Tcp port incorrect");
	tcpPort = (USHORT)TcpPort;
}

void CPortSetting::setStatus(_In_ PortStatus Status)
{
	if (status != Status)
	{
		statusChange = true;
	}
	status = Status;
}

bool operator==(CPortSetting const& s1, CPortSetting const& s2)
{
	if (s1.comPort == s2.comPort) return true;
	if (s1.ip == s2.ip && s1.tcpPort == s2.tcpPort) return true;
	return false;
}
