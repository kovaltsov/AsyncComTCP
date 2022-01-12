#pragma once
#include "CPortSetting.h"

// Диалоговое окно CDialogEdit

#define COM_OFFSET 3
#define COM_STR_SIZE 7 //COM255


class CDialogEdit : public CDialog
{
	DECLARE_DYNAMIC(CDialogEdit)
private:
	CPortSetting m_portSetting;
	int curVal = -1;
public:
	CDialogEdit(CWnd* pParent = nullptr);   // стандартный конструктор
	CDialogEdit(CPortSetting &s);
	CDialogEdit(CPortSetting& s, int CurVal);
	virtual ~CDialogEdit();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_EDIT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	DECLARE_MESSAGE_MAP()
private:
	void findPorts();
public:
	afx_msg void OnBnClickedOk();
	CIPAddressCtrl m_IP;
	CComboBox m_comPort;
	CPortSetting getPortSetting();
	CString m_tcpPort;
	DWORD m_IPvalue;
	CString m_comPortValue;
};
