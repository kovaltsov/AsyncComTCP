// CDialogEdit.cpp: файл реализации
//

#include "pch.h"
#include "CDialogEdit.h"
#include "afxdialogex.h"
#include "AsyncComTCP.h"
#include "WS2tcpip.h"

#pragma comment(lib, "OneCore.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Диалоговое окно CDialogEdit

IMPLEMENT_DYNAMIC(CDialogEdit, CDialog)

CDialogEdit::CDialogEdit(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_EDIT, pParent)
	, m_tcpPort(_T(""))
	, m_IPvalue(0)
	, m_IP()
	, m_comPortValue(_T(""))
{

}

CDialogEdit::CDialogEdit(CPortSetting &s)
	: CDialog(IDD_DIALOG_EDIT)
{
	
	m_portSetting = s;
	m_tcpPort.Format("%d", s.getTcpPort());

	IN_ADDR IPAddr;
	inet_pton(AF_INET, m_portSetting.getIP().c_str(), &IPAddr);
	//reset network byte order
	m_IPvalue = (ntohl(IPAddr.S_un.S_addr));

}

CDialogEdit::~CDialogEdit()
{
}

void CDialogEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS, m_IP);
	DDX_Control(pDX, IDC_COMBO_COM, m_comPort);
	DDX_Text(pDX, IDC_EDIT_TCPPORT, m_tcpPort);
	DDX_IPAddress(pDX, IDC_IPADDRESS, m_IPvalue);
	DDX_CBString(pDX, IDC_COMBO_COM, m_comPortValue);

	findPorts();
}


BEGIN_MESSAGE_MAP(CDialogEdit, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogEdit::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CDialogEdit::OnBnClickedButton1)
END_MESSAGE_MAP()


// Обработчики сообщений CDialogEdit

//find all available ports
void CDialogEdit::findPorts()
{
	ULONG size = 1;
	ULONG *ports = new ULONG[size];
	ULONG found;
	ULONG rez = GetCommPorts(ports, size, &found);
	char str[COM_STR_SIZE] = "COM";
	//m_comPort.Clear();
	m_comPort.ResetContent();
	switch (rez)
	{
	case ERROR_MORE_DATA:
		delete[] ports;
		size = found;
		ports = new ULONG[size];
		rez = GetCommPorts(ports, size, &found);
		break;
	}
	if (rez == ERROR_SUCCESS)
	{
		for (int i = 0; i < size; i++)
		{
			_itoa_s(ports[i], str + COM_OFFSET, COM_STR_SIZE - COM_OFFSET, 10);
			m_comPort.AddString(str);
		}
	}
	//set cur port
	int cur = m_comPort.FindString(0, m_portSetting.getComPort().c_str());
	m_comPort.SetCurSel(cur);
	delete[] ports;
}

void CDialogEdit::OnBnClickedOk()
{
	UpdateData(TRUE);
	// TODO: добавьте свой код обработчика уведомлений
	char buf[INET_ADDRSTRLEN];
	IN_ADDR IPAddr;
	//reset network byte order
	IPAddr.S_un.S_addr = ntohl(m_IPvalue);
	string strIP;
	inet_ntop(AF_INET, &IPAddr, buf, INET_ADDRSTRLEN);
	strIP = buf;

	int tcpPort;
	m_tcpPort.GetString();
	tcpPort = atoi(m_tcpPort.GetString());

	string strCom;
	strCom = m_comPortValue;
	
	try {
		m_portSetting.setIP(strIP);

		m_portSetting.setComPort(strCom);

		m_portSetting.setTcpPort(tcpPort);
	}
	catch (exception ex)
	{
		MessageBox(ex.what(), "Error", MB_OK | MB_ICONERROR);
	}
	CDialog::OnOK();
}

CPortSetting CDialogEdit::getPortSetting()
{
	return m_portSetting;
}





void CDialogEdit::OnBnClickedButton1()
{
	// TODO: добавьте свой код обработчика уведомлений

}
