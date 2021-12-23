
// AsyncComToTCPDlg.h: файл заголовка
//

#pragma once
#include "common.h"

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
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonAdd();
private:
	void fillSettings();
	void addItem(const string& ComPort, const string& IP, int TcpPort);



};
