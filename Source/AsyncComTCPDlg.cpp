
// AsyncComToTCPDlg.cpp: файл реализации
//

#include "pch.h"
//#include "framework.h"
#include "AsyncComTCP.h"
#include "AsyncComTCPDlg.h"
#include "afxdialogex.h"
#include "CDialogEdit.h"
#include "CDialogEdit1.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ofstream fout;

// Диалоговое окно CAboutDlg используется для описания сведений о приложении

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

// Реализация
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Диалоговое окно CAsyncComTCPDlg



CAsyncComTCPDlg::CAsyncComTCPDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ASYNCCOMTOTCP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAsyncComTCPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ListControl);
}

BEGIN_MESSAGE_MAP(CAsyncComTCPDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CAsyncComTCPDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CAsyncComTCPDlg::OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, &CAsyncComTCPDlg::OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CAsyncComTCPDlg::OnBnClickedButtonDelete)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CAsyncComTCPDlg::OnBnClickedButtonStop)
	ON_WM_TIMER()
	ON_COMMAND(ID_FILE_SAVECONFIG, &CAsyncComTCPDlg::OnFileSaveconfig)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CAsyncComTCPDlg::OnNMDblclkList1)
END_MESSAGE_MAP()


// Обработчики сообщений CAsyncComTCPDlg

BOOL CAsyncComTCPDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Добавление пункта "О программе..." в системное меню.

	// IDM_ABOUTBOX должен быть в пределах системной команды.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию
	m_ListControl.SetExtendedStyle(m_ListControl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_SHOWSELALWAYS | LVIS_SELECTED);
	m_ListControl.InsertColumn(0, _T("COM Порт"), LVCFMT_LEFT, 80);
	m_ListControl.InsertColumn(1, _T("IP адрес"), LVCFMT_LEFT, 100);
	m_ListControl.InsertColumn(2, _T("TCP Порт"), LVCFMT_LEFT, 80);
	m_ListControl.InsertColumn(3, _T("Статус"), LVCFMT_LEFT, 80);

	//cout to file
	fout.open("Log.log");
	cout.rdbuf(fout.rdbuf());

	//Open config
	readCfg();

#ifdef TCP_CLIENT
	this->SetWindowTextA("Client");
#endif
#ifdef TCP_SERVER
	this->SetWindowTextA("Server");
#endif

	fillSettings();

	SetTimer(TIMER_UPDATE_ID, TIMER_UPDATE_TIMEOUT, NULL);//Update CListCtrl timer

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

void CAsyncComTCPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CAsyncComTCPDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CAsyncComTCPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAsyncComTCPDlg::OnBnClickedButtonAdd()
{
	// TODO: добавьте свой код обработчика уведомлений
	POSITION pos = m_ListControl.GetFirstSelectedItemPosition();
	int curSel = m_ListControl.GetNextSelectedItem(pos);
	CDialogEdit* editDlg;
	if (curSel != -1)
	{
		editDlg = new CDialogEdit(portSettings[curSel]);
	}
	else
	{ 
		editDlg = new CDialogEdit();
	}
	if (editDlg->DoModal() == IDOK)
	{
		portSettings.push_back(editDlg->getPortSetting());
	}
	fillSettings();
}

void CAsyncComTCPDlg::OnBnClickedButtonDelete()
{
	POSITION pos = m_ListControl.GetFirstSelectedItemPosition();
	int curSel = m_ListControl.GetNextSelectedItem(pos);
	if (curSel != -1)
	{
		portSettings.erase(portSettings.begin() + curSel);
		fillSettings();
	}
}

void CAsyncComTCPDlg::OnBnClickedButtonEdit()
{
	POSITION pos = m_ListControl.GetFirstSelectedItemPosition();
	int curSel = m_ListControl.GetNextSelectedItem(pos);
	if (curSel != -1)
	{
		CDialogEdit editDlg(portSettings[curSel], curSel);
		if (editDlg.DoModal() == IDOK)
		{
			portSettings[curSel] = editDlg.getPortSetting();
		}
		fillSettings();
	}
}

void CAsyncComTCPDlg::fillSettings()
{
	m_ListControl.DeleteAllItems();
	for (unsigned int i = 0; i < portSettings.size(); i++)
	{
		addItem(portSettings[i].getComPort(), 
			portSettings[i].getIP(), 
			portSettings[i].getTcpPort(), 
			portSettings[i].getStatusString());
	}
	//if (curSel != -1)
	//{
	//	//m_ListControl.SetSelectedColumn(curSel);
	//	m_ListControl.SetSelectionMark(curSel);
	//	m_ListControl.selec
	//}
}

void CAsyncComTCPDlg::addItem(const string& ComPort, const string& IP, int TcpPort, const string& status)
{
    char tcpStr[TCP_PORT_LENGTH] = { '\0' };
    if (_itoa_s(TcpPort, tcpStr, 10) == 0)
    {
        int lastItem = m_ListControl.GetItemCount();
        m_ListControl.InsertItem(lastItem, ComPort.c_str());
        m_ListControl.SetItemText(lastItem, 1, IP.c_str());
        m_ListControl.SetItemText(lastItem, 2, tcpStr);
		m_ListControl.SetItemText(lastItem, 3, status.c_str());
    }
}

void CAsyncComTCPDlg::updatePortSettings()
{
	//m_ListControl.DeleteAllItems();
	for (unsigned int i = 0; i < portSettings.size(); i++)
	{
		if (portSettings[i].isStatusChange())
		{
		/*	setItem(i, portSettings[i].getComPort(),
				portSettings[i].getIP(),
				portSettings[i].getTcpPort(),
				portSettings[i].getStatusString());*/
			m_ListControl.SetItemText(i, 3, portSettings[i].getStatusString().c_str());
		}
	}
	//if (curSel != -1)
	//{
	//	//m_ListControl.SetSelectedColumn(curSel);
	//	m_ListControl.SetSelectionMark(curSel);
	//	m_ListControl.selec
	//}
}

void CAsyncComTCPDlg::setItem(int num, const string& ComPort, const string& IP, int TcpPort, const string& status)
{
	char tcpStr[TCP_PORT_LENGTH] = { '\0' };
	if (_itoa_s(TcpPort, tcpStr, 10) == 0)
	{
		m_ListControl.SetItemText(num, 0, ComPort.c_str());
		m_ListControl.SetItemText(num, 1, IP.c_str());
		m_ListControl.SetItemText(num, 2, tcpStr);
		m_ListControl.SetItemText(num, 3, status.c_str());
	}
}

void CAsyncComTCPDlg::stopThreads()
{
	for (unsigned int i = 0; i < threadConfigs.size(); i++)
	{
		delete threadConfigs[i];
	}
	threadConfigs.clear();
}

void CAsyncComTCPDlg::readCfg()
{
	portSettings.clear();
	ifstream fCfg;
	fCfg.open(CONFIG_FILE);
	if (!fCfg.bad())
	{
		CPortSetting s;
		try {
			while (!fCfg.eof())
			{
				string comPort, ip;
				int tcpPort;
				fCfg >> comPort >> ip >> tcpPort;
				s.setComPort(comPort);
				s.setIP(ip);
				s.setTcpPort(tcpPort);
				portSettings.push_back(s);
			}
		}
		catch (exception ex)
		{
			//MessageBox(ex.what(), "Error", MB_OK | MB_ICONERROR);
		}
	}
	fCfg.close();
}

void CAsyncComTCPDlg::saveCfg()
{
	ofstream fCfg;
	fCfg.open(CONFIG_FILE);
	if (!fCfg.bad())
	{
		fCfg.clear();
		for (unsigned int i = 0; i < portSettings.size(); i++)
		{
			/*addItem(portSettings[i].getComPort(),
				portSettings[i].getIP(),
				portSettings[i].getTcpPort(),
				portSettings[i].getStatusString());*/
			fCfg << portSettings[i].getComPort() << " " <<
				portSettings[i].getIP() << " " <<
				portSettings[i].getTcpPort() << endl;
		}
	}
	fCfg.close();
}


void CAsyncComTCPDlg::OnBnClickedButtonApply()
{
   // m_ListControl.DeleteAllItems();
	if (threadConfigs.size() != 0)
	{
		MessageBox("Threads in processing. Press stop button and retry", "Error", MB_OK | MB_ICONSTOP);
		return;
	}
    for (unsigned int i = 0; i < portSettings.size(); i++)
    {
        //addItem(portSettings[i].getComPort(), portSettings[i].getIP(), portSettings[i].getTcpPort());
        CThreadConfig* cfg = new CThreadConfig(&portSettings[i]);
        threadConfigs.push_back(cfg);
    }
	// TODO: добавьте свой код обработчика уведомлений
}


void CAsyncComTCPDlg::OnClose()
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного
	stopThreads();
	fout.close();
	CDialogEx::OnClose();
}

void CAsyncComTCPDlg::OnBnClickedButtonStop()
{
	// TODO: добавьте свой код обработчика уведомлений
	stopThreads();
}


void CAsyncComTCPDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного
	switch (nIDEvent)
	{
	case TIMER_UPDATE_ID:
		updatePortSettings();
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CAsyncComTCPDlg::OnFileSaveconfig()
{
	// TODO: добавьте свой код обработчика команд
	saveCfg();
}


void CAsyncComTCPDlg::OnNMDblclkList1(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	OnBnClickedButtonEdit();
}
