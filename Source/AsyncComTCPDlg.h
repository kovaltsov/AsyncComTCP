
// AsyncComToTCPDlg.h: файл заголовка
//

#pragma once

#define TIMER_UPDATE_ID 1
#define TIMER_UPDATE_TIMEOUT 100
#define CONFIG_FILE "AsyncComTcp.cfg"

// Диалоговое окно CAsyncComTCPDlg
class CAsyncComTCPDlg : public CDialogEx
{
// Создание
public:
	CAsyncComTCPDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ASYNCCOMTOTCP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ListControl;
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonEdit();
	afx_msg void OnBnClickedButtonDelete();
private:
	void fillSettings();
	void addItem(const string& ComPort, const string& IP, int TcpPort, const string& status);

	void setItem(int num, const string& ComPort, const string& IP, int TcpPort, const string& status);
	void stopThreads();
	void readCfg();
	void saveCfg();
public:
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnClose();
	void updatePortSettings();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnFileSaveconfig();
	afx_msg void OnNMDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
};
