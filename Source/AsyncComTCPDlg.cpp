
// AsyncComToTCPDlg.cpp: файл реализации
//

#include "pch.h"
#include "framework.h"
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
	ON_BN_CLICKED(IDOK, &CAsyncComTCPDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CAsyncComTCPDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CAsyncComTCPDlg::OnBnClickedButtonApply)
	ON_WM_CLOSE()
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

	//cout to file
	fout.open("Log.txt");
	cout.rdbuf(fout.rdbuf());

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

void CAsyncComTCPDlg::OnBnClickedOk()
{
	// TODO: добавьте свой код обработчика уведомлений
	CDialogEx::OnOK();
}


void CAsyncComTCPDlg::OnBnClickedButtonAdd()
{
	// TODO: добавьте свой код обработчика уведомлений
	CPortSetting s("COM4", "192.168.17.35", 8035);
	CDialogEdit editDlg(s);
	if (editDlg.DoModal() == IDOK)
	{
		portSettings.push_back(editDlg.getPortSetting());
	}
	fillSettings();
}

void CAsyncComTCPDlg::fillSettings()
{
	m_ListControl.DeleteAllItems();
	for (unsigned int i = 0; i < portSettings.size(); i++)
	{
		addItem(portSettings[i].getComPort(), portSettings[i].getIP(), portSettings[i].getTcpPort());
	}
}

void CAsyncComTCPDlg::addItem(const string& ComPort, const string& IP, int TcpPort)
{
    char tcpStr[TCP_PORT_LENGTH] = { '\0' };
    if (_itoa_s(TcpPort, tcpStr, 10) == 0)
    {
        int lastItem = m_ListControl.GetItemCount();
        m_ListControl.InsertItem(lastItem, ComPort.c_str());
        m_ListControl.SetItemText(lastItem, 1, IP.c_str());
        m_ListControl.SetItemText(lastItem, 2, tcpStr);
    }
}


void CAsyncComTCPDlg::OnBnClickedButtonApply()
{
   // m_ListControl.DeleteAllItems();
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
    for (unsigned int i = 0; i < threadConfigs.size(); i++)
    {
        delete threadConfigs[i];
    }
	fout.close();
	CDialogEx::OnClose();
}